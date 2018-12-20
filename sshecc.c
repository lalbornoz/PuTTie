/*
 * Elliptic-curve crypto module for PuTTY
 * Implements the three required curves, no optional curves
 *
 * NOTE: Only curves on prime field are handled by the maths functions
 *       in Weierstrass form using Jacobian co-ordinates.
 *
 *       Montgomery form curves are supported for DH. (Curve25519)
 *
 *       Edwards form curves are supported for DSA. (Ed25519)
 */

/*
 * References:
 *
 * Elliptic curves in SSH are specified in RFC 5656:
 *   http://tools.ietf.org/html/rfc5656
 *
 * That specification delegates details of public key formatting and a
 * lot of underlying mechanism to SEC 1:
 *   http://www.secg.org/sec1-v2.pdf
 *
 * Montgomery maths from:
 * Handbook of elliptic and hyperelliptic curve cryptography, Chapter 13
 *   http://cs.ucsb.edu/~koc/ccs130h/2013/EllipticHyperelliptic-CohenFrey.pdf
 *
 * Curve25519 spec from libssh (with reference to other things in the
 * libssh code):
 *   https://git.libssh.org/users/aris/libssh.git/tree/doc/curve25519-sha256@libssh.org.txt
 *
 * Edwards DSA:
 *   http://ed25519.cr.yp.to/ed25519-20110926.pdf
 */

#include <stdlib.h>
#include <assert.h>

#include "ssh.h"

/* ----------------------------------------------------------------------
 * Elliptic curve definitions
 */

static void initialise_wcurve(struct ec_curve *curve, int bits,
                              const unsigned char *p,
                              const unsigned char *a, const unsigned char *b,
                              const unsigned char *n, const unsigned char *Gx,
                              const unsigned char *Gy)
{
    int length = bits / 8;
    if (bits % 8) ++length;

    curve->type = EC_WEIERSTRASS;

    curve->fieldBits = bits;
    curve->p = bignum_from_bytes(p, length);

    /* Curve co-efficients */
    curve->w.a = bignum_from_bytes(a, length);
    curve->w.b = bignum_from_bytes(b, length);

    /* Group order and generator */
    curve->w.n = bignum_from_bytes(n, length);
    curve->w.G.x = bignum_from_bytes(Gx, length);
    curve->w.G.y = bignum_from_bytes(Gy, length);
    curve->w.G.curve = curve;
    curve->w.G.infinity = false;
}

static void initialise_mcurve(struct ec_curve *curve, int bits,
                              const unsigned char *p,
                              const unsigned char *a, const unsigned char *b,
                              const unsigned char *Gx)
{
    int length = bits / 8;
    if (bits % 8) ++length;

    curve->type = EC_MONTGOMERY;

    curve->fieldBits = bits;
    curve->p = bignum_from_bytes(p, length);

    /* Curve co-efficients */
    curve->m.a = bignum_from_bytes(a, length);
    curve->m.b = bignum_from_bytes(b, length);

    /* Generator */
    curve->m.G.x = bignum_from_bytes(Gx, length);
    curve->m.G.y = NULL;
    curve->m.G.z = NULL;
    curve->m.G.curve = curve;
    curve->m.G.infinity = false;
}

static void initialise_ecurve(struct ec_curve *curve, int bits,
                              const unsigned char *p,
                              const unsigned char *l, const unsigned char *d,
                              const unsigned char *Bx, const unsigned char *By)
{
    int length = bits / 8;
    if (bits % 8) ++length;

    curve->type = EC_EDWARDS;

    curve->fieldBits = bits;
    curve->p = bignum_from_bytes(p, length);

    /* Curve co-efficients */
    curve->e.l = bignum_from_bytes(l, length);
    curve->e.d = bignum_from_bytes(d, length);

    /* Group order and generator */
    curve->e.B.x = bignum_from_bytes(Bx, length);
    curve->e.B.y = bignum_from_bytes(By, length);
    curve->e.B.curve = curve;
    curve->e.B.infinity = false;
}

static struct ec_curve *ec_p256(void)
{
    static struct ec_curve curve = { 0 };
    static bool initialised = false;

    if (!initialised)
    {
        static const unsigned char p[] = {
            0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        };
        static const unsigned char a[] = {
            0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc
        };
        static const unsigned char b[] = {
            0x5a, 0xc6, 0x35, 0xd8, 0xaa, 0x3a, 0x93, 0xe7,
            0xb3, 0xeb, 0xbd, 0x55, 0x76, 0x98, 0x86, 0xbc,
            0x65, 0x1d, 0x06, 0xb0, 0xcc, 0x53, 0xb0, 0xf6,
            0x3b, 0xce, 0x3c, 0x3e, 0x27, 0xd2, 0x60, 0x4b
        };
        static const unsigned char n[] = {
            0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xbc, 0xe6, 0xfa, 0xad, 0xa7, 0x17, 0x9e, 0x84,
            0xf3, 0xb9, 0xca, 0xc2, 0xfc, 0x63, 0x25, 0x51
        };
        static const unsigned char Gx[] = {
            0x6b, 0x17, 0xd1, 0xf2, 0xe1, 0x2c, 0x42, 0x47,
            0xf8, 0xbc, 0xe6, 0xe5, 0x63, 0xa4, 0x40, 0xf2,
            0x77, 0x03, 0x7d, 0x81, 0x2d, 0xeb, 0x33, 0xa0,
            0xf4, 0xa1, 0x39, 0x45, 0xd8, 0x98, 0xc2, 0x96
        };
        static const unsigned char Gy[] = {
            0x4f, 0xe3, 0x42, 0xe2, 0xfe, 0x1a, 0x7f, 0x9b,
            0x8e, 0xe7, 0xeb, 0x4a, 0x7c, 0x0f, 0x9e, 0x16,
            0x2b, 0xce, 0x33, 0x57, 0x6b, 0x31, 0x5e, 0xce,
            0xcb, 0xb6, 0x40, 0x68, 0x37, 0xbf, 0x51, 0xf5
        };

        initialise_wcurve(&curve, 256, p, a, b, n, Gx, Gy);
        curve.textname = curve.name = "nistp256";

        /* Now initialised, no need to do it again */
        initialised = true;
    }

    return &curve;
}

static struct ec_curve *ec_p384(void)
{
    static struct ec_curve curve = { 0 };
    static bool initialised = false;

    if (!initialised)
    {
        static const unsigned char p[] = {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
            0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
        };
        static const unsigned char a[] = {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
            0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xfc
        };
        static const unsigned char b[] = {
            0xb3, 0x31, 0x2f, 0xa7, 0xe2, 0x3e, 0xe7, 0xe4,
            0x98, 0x8e, 0x05, 0x6b, 0xe3, 0xf8, 0x2d, 0x19,
            0x18, 0x1d, 0x9c, 0x6e, 0xfe, 0x81, 0x41, 0x12,
            0x03, 0x14, 0x08, 0x8f, 0x50, 0x13, 0x87, 0x5a,
            0xc6, 0x56, 0x39, 0x8d, 0x8a, 0x2e, 0xd1, 0x9d,
            0x2a, 0x85, 0xc8, 0xed, 0xd3, 0xec, 0x2a, 0xef
        };
        static const unsigned char n[] = {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xc7, 0x63, 0x4d, 0x81, 0xf4, 0x37, 0x2d, 0xdf,
            0x58, 0x1a, 0x0d, 0xb2, 0x48, 0xb0, 0xa7, 0x7a,
            0xec, 0xec, 0x19, 0x6a, 0xcc, 0xc5, 0x29, 0x73
        };
        static const unsigned char Gx[] = {
            0xaa, 0x87, 0xca, 0x22, 0xbe, 0x8b, 0x05, 0x37,
            0x8e, 0xb1, 0xc7, 0x1e, 0xf3, 0x20, 0xad, 0x74,
            0x6e, 0x1d, 0x3b, 0x62, 0x8b, 0xa7, 0x9b, 0x98,
            0x59, 0xf7, 0x41, 0xe0, 0x82, 0x54, 0x2a, 0x38,
            0x55, 0x02, 0xf2, 0x5d, 0xbf, 0x55, 0x29, 0x6c,
            0x3a, 0x54, 0x5e, 0x38, 0x72, 0x76, 0x0a, 0xb7
        };
        static const unsigned char Gy[] = {
            0x36, 0x17, 0xde, 0x4a, 0x96, 0x26, 0x2c, 0x6f,
            0x5d, 0x9e, 0x98, 0xbf, 0x92, 0x92, 0xdc, 0x29,
            0xf8, 0xf4, 0x1d, 0xbd, 0x28, 0x9a, 0x14, 0x7c,
            0xe9, 0xda, 0x31, 0x13, 0xb5, 0xf0, 0xb8, 0xc0,
            0x0a, 0x60, 0xb1, 0xce, 0x1d, 0x7e, 0x81, 0x9d,
            0x7a, 0x43, 0x1d, 0x7c, 0x90, 0xea, 0x0e, 0x5f
        };

        initialise_wcurve(&curve, 384, p, a, b, n, Gx, Gy);
        curve.textname = curve.name = "nistp384";

        /* Now initialised, no need to do it again */
        initialised = true;
    }

    return &curve;
}

static struct ec_curve *ec_p521(void)
{
    static struct ec_curve curve = { 0 };
    static bool initialised = false;

    if (!initialised)
    {
        static const unsigned char p[] = {
            0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff
        };
        static const unsigned char a[] = {
            0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xfc
        };
        static const unsigned char b[] = {
            0x00, 0x51, 0x95, 0x3e, 0xb9, 0x61, 0x8e, 0x1c,
            0x9a, 0x1f, 0x92, 0x9a, 0x21, 0xa0, 0xb6, 0x85,
            0x40, 0xee, 0xa2, 0xda, 0x72, 0x5b, 0x99, 0xb3,
            0x15, 0xf3, 0xb8, 0xb4, 0x89, 0x91, 0x8e, 0xf1,
            0x09, 0xe1, 0x56, 0x19, 0x39, 0x51, 0xec, 0x7e,
            0x93, 0x7b, 0x16, 0x52, 0xc0, 0xbd, 0x3b, 0xb1,
            0xbf, 0x07, 0x35, 0x73, 0xdf, 0x88, 0x3d, 0x2c,
            0x34, 0xf1, 0xef, 0x45, 0x1f, 0xd4, 0x6b, 0x50,
            0x3f, 0x00
        };
        static const unsigned char n[] = {
            0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xfa, 0x51, 0x86, 0x87, 0x83, 0xbf, 0x2f,
            0x96, 0x6b, 0x7f, 0xcc, 0x01, 0x48, 0xf7, 0x09,
            0xa5, 0xd0, 0x3b, 0xb5, 0xc9, 0xb8, 0x89, 0x9c,
            0x47, 0xae, 0xbb, 0x6f, 0xb7, 0x1e, 0x91, 0x38,
            0x64, 0x09
        };
        static const unsigned char Gx[] = {
            0x00, 0xc6, 0x85, 0x8e, 0x06, 0xb7, 0x04, 0x04,
            0xe9, 0xcd, 0x9e, 0x3e, 0xcb, 0x66, 0x23, 0x95,
            0xb4, 0x42, 0x9c, 0x64, 0x81, 0x39, 0x05, 0x3f,
            0xb5, 0x21, 0xf8, 0x28, 0xaf, 0x60, 0x6b, 0x4d,
            0x3d, 0xba, 0xa1, 0x4b, 0x5e, 0x77, 0xef, 0xe7,
            0x59, 0x28, 0xfe, 0x1d, 0xc1, 0x27, 0xa2, 0xff,
            0xa8, 0xde, 0x33, 0x48, 0xb3, 0xc1, 0x85, 0x6a,
            0x42, 0x9b, 0xf9, 0x7e, 0x7e, 0x31, 0xc2, 0xe5,
            0xbd, 0x66
        };
        static const unsigned char Gy[] = {
            0x01, 0x18, 0x39, 0x29, 0x6a, 0x78, 0x9a, 0x3b,
            0xc0, 0x04, 0x5c, 0x8a, 0x5f, 0xb4, 0x2c, 0x7d,
            0x1b, 0xd9, 0x98, 0xf5, 0x44, 0x49, 0x57, 0x9b,
            0x44, 0x68, 0x17, 0xaf, 0xbd, 0x17, 0x27, 0x3e,
            0x66, 0x2c, 0x97, 0xee, 0x72, 0x99, 0x5e, 0xf4,
            0x26, 0x40, 0xc5, 0x50, 0xb9, 0x01, 0x3f, 0xad,
            0x07, 0x61, 0x35, 0x3c, 0x70, 0x86, 0xa2, 0x72,
            0xc2, 0x40, 0x88, 0xbe, 0x94, 0x76, 0x9f, 0xd1,
            0x66, 0x50
        };

        initialise_wcurve(&curve, 521, p, a, b, n, Gx, Gy);
        curve.textname = curve.name = "nistp521";

        /* Now initialised, no need to do it again */
        initialised = true;
    }

    return &curve;
}

static struct ec_curve *ec_curve25519(void)
{
    static struct ec_curve curve = { 0 };
    static bool initialised = false;

    if (!initialised)
    {
        static const unsigned char p[] = {
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xed
        };
        static const unsigned char a[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x6d, 0x06
        };
        static const unsigned char b[] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
        };
        static const unsigned char gx[32] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09
        };

        initialise_mcurve(&curve, 256, p, a, b, gx);
        /* This curve doesn't need a name, because it's never used in
         * any format that embeds the curve name */
        curve.name = NULL;
        curve.textname = "Curve25519";

        /* Now initialised, no need to do it again */
        initialised = true;
    }

    return &curve;
}

static struct ec_curve *ec_ed25519(void)
{
    static struct ec_curve curve = { 0 };
    static bool initialised = false;

    if (!initialised)
    {
        static const unsigned char q[] = {
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xed
        };
        static const unsigned char l[32] = {
            0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x14, 0xde, 0xf9, 0xde, 0xa2, 0xf7, 0x9c, 0xd6,
            0x58, 0x12, 0x63, 0x1a, 0x5c, 0xf5, 0xd3, 0xed
        };
        static const unsigned char d[32] = {
            0x52, 0x03, 0x6c, 0xee, 0x2b, 0x6f, 0xfe, 0x73,
            0x8c, 0xc7, 0x40, 0x79, 0x77, 0x79, 0xe8, 0x98,
            0x00, 0x70, 0x0a, 0x4d, 0x41, 0x41, 0xd8, 0xab,
            0x75, 0xeb, 0x4d, 0xca, 0x13, 0x59, 0x78, 0xa3
        };
        static const unsigned char Bx[32] = {
            0x21, 0x69, 0x36, 0xd3, 0xcd, 0x6e, 0x53, 0xfe,
            0xc0, 0xa4, 0xe2, 0x31, 0xfd, 0xd6, 0xdc, 0x5c,
            0x69, 0x2c, 0xc7, 0x60, 0x95, 0x25, 0xa7, 0xb2,
            0xc9, 0x56, 0x2d, 0x60, 0x8f, 0x25, 0xd5, 0x1a
        };
        static const unsigned char By[32] = {
            0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
            0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
            0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
            0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x58
        };

        /* This curve doesn't need a name, because it's never used in
         * any format that embeds the curve name */
        curve.name = NULL;

        initialise_ecurve(&curve, 256, q, l, d, Bx, By);
        curve.textname = "Ed25519";

        /* Now initialised, no need to do it again */
        initialised = true;
    }

    return &curve;
}

/* Return 1 if a is -3 % p, otherwise return 0
 * This is used because there are some maths optimisations */
static bool ec_aminus3(const struct ec_curve *curve)
{
    bool ret;
    Bignum _p;

    if (curve->type != EC_WEIERSTRASS) {
        return false;
    }

    _p = bignum_add_long(curve->w.a, 3);

    ret = !bignum_cmp(curve->p, _p);
    freebn(_p);
    return ret;
}

/* ----------------------------------------------------------------------
 * Elliptic curve field maths
 */

static Bignum ecf_add(const Bignum a, const Bignum b,
                      const struct ec_curve *curve)
{
    Bignum a1, b1, ab, ret;

    a1 = bigmod(a, curve->p);
    b1 = bigmod(b, curve->p);

    ab = bigadd(a1, b1);
    freebn(a1);
    freebn(b1);

    ret = bigmod(ab, curve->p);
    freebn(ab);

    return ret;
}

static Bignum ecf_square(const Bignum a, const struct ec_curve *curve)
{
    return modmul(a, a, curve->p);
}

static Bignum ecf_treble(const Bignum a, const struct ec_curve *curve)
{
    Bignum ret, tmp;

    /* Double */
    tmp = bignum_lshift(a, 1);

    /* Add itself (i.e. treble) */
    ret = bigadd(tmp, a);
    freebn(tmp);

    /* Normalise */
    while (bignum_cmp(ret, curve->p) >= 0)
    {
        tmp = bigsub(ret, curve->p);
        assert(tmp);
        freebn(ret);
        ret = tmp;
    }

    return ret;
}

static Bignum ecf_double(const Bignum a, const struct ec_curve *curve)
{
    Bignum ret = bignum_lshift(a, 1);
    if (bignum_cmp(ret, curve->p) >= 0)
    {
        Bignum tmp = bigsub(ret, curve->p);
        assert(tmp);
        freebn(ret);
        return tmp;
    }
    else
    {
        return ret;
    }
}

/* ----------------------------------------------------------------------
 * Memory functions
 */

void ec_point_free(struct ec_point *point)
{
    if (point == NULL) return;
    point->curve = 0;
    if (point->x) freebn(point->x);
    if (point->y) freebn(point->y);
    if (point->z) freebn(point->z);
    point->infinity = false;
    sfree(point);
}

static struct ec_point *ec_point_new(const struct ec_curve *curve,
                                     const Bignum x, const Bignum y, const Bignum z,
                                     bool infinity)
{
    struct ec_point *point = snewn(1, struct ec_point);
    point->curve = curve;
    point->x = x;
    point->y = y;
    point->z = z;
    point->infinity = infinity;
    return point;
}

static struct ec_point *ec_point_copy(const struct ec_point *a)
{
    if (a == NULL) return NULL;
    return ec_point_new(a->curve,
                        a->x ? copybn(a->x) : NULL,
                        a->y ? copybn(a->y) : NULL,
                        a->z ? copybn(a->z) : NULL,
                        a->infinity);
}

static bool ec_point_verify(const struct ec_point *a)
{
    if (a->infinity) {
        return true;
    } else if (a->curve->type == EC_EDWARDS) {
        /* Check y^2 - x^2 - 1 - d * x^2 * y^2 == 0 */
        Bignum y2, x2, tmp, tmp2, tmp3;
        bool ret;

        y2 = ecf_square(a->y, a->curve);
        x2 = ecf_square(a->x, a->curve);
        tmp = modmul(a->curve->e.d, x2, a->curve->p);
        tmp2 = modmul(tmp, y2, a->curve->p);
        freebn(tmp);
        tmp = modsub(y2, x2, a->curve->p);
        freebn(y2);
        freebn(x2);
        tmp3 = modsub(tmp, tmp2, a->curve->p);
        freebn(tmp);
        freebn(tmp2);
        ret = !bignum_cmp(tmp3, One);
        freebn(tmp3);
        return ret;
    } else if (a->curve->type == EC_WEIERSTRASS) {
        /* Verify y^2 = x^3 + ax + b */
        bool ret = false;

        Bignum lhs = NULL, x3 = NULL, ax = NULL, x3ax = NULL, x3axm = NULL, x3axb = NULL, rhs = NULL;

        Bignum Three = bignum_from_long(3);

        lhs = modmul(a->y, a->y, a->curve->p);

        /* This uses montgomery multiplication to optimise */
        x3 = modpow(a->x, Three, a->curve->p);
        freebn(Three);
        ax = modmul(a->curve->w.a, a->x, a->curve->p);
        x3ax = bigadd(x3, ax);
        freebn(x3); x3 = NULL;
        freebn(ax); ax = NULL;
        x3axm = bigmod(x3ax, a->curve->p);
        freebn(x3ax); x3ax = NULL;
        x3axb = bigadd(x3axm, a->curve->w.b);
        freebn(x3axm); x3axm = NULL;
        rhs = bigmod(x3axb, a->curve->p);
        freebn(x3axb);

        ret = !bignum_cmp(lhs, rhs);
        freebn(lhs);
        freebn(rhs);

        return ret;
    } else {
        return false;
    }
}

/* ----------------------------------------------------------------------
 * Elliptic curve point maths
 */

/* Returns true on success and false on memory error */
static bool ecp_normalise(struct ec_point *a)
{
    if (!a) {
        /* No point */
        return false;
    }

    if (a->infinity) {
        /* Point is at infinity - i.e. normalised */
        return true;
    }

    if (a->curve->type == EC_WEIERSTRASS) {
        /* In Jacobian Coordinates the triple (X, Y, Z) represents
           the affine point (X / Z^2, Y / Z^3) */

        Bignum Z2, Z2inv, Z3, Z3inv, tx, ty;

        if (!a->x || !a->y) {
            /* No point defined */
            return false;
        } else if (!a->z) {
            /* Already normalised */
            return true;
        }

        Z2 = ecf_square(a->z, a->curve);
        Z2inv = modinv(Z2, a->curve->p);
        if (!Z2inv) {
            freebn(Z2);
            return false;
        }
        tx = modmul(a->x, Z2inv, a->curve->p);
        freebn(Z2inv);

        Z3 = modmul(Z2, a->z, a->curve->p);
        freebn(Z2);
        Z3inv = modinv(Z3, a->curve->p);
        freebn(Z3);
        if (!Z3inv) {
            freebn(tx);
            return false;
        }
        ty = modmul(a->y, Z3inv, a->curve->p);
        freebn(Z3inv);

        freebn(a->x);
        a->x = tx;
        freebn(a->y);
        a->y = ty;
        freebn(a->z);
        a->z = NULL;
        return true;
    } else if (a->curve->type == EC_MONTGOMERY) {
        /* In Montgomery (X : Z) represents the x co-ord (X / Z, ?) */

        Bignum tmp, tmp2;

        if (!a->x) {
            /* No point defined */
            return false;
        } else if (!a->z) {
            /* Already normalised */
            return true;
        }

        tmp = modinv(a->z, a->curve->p);
        if (!tmp) {
            return false;
        }
        tmp2 = modmul(a->x, tmp, a->curve->p);
        freebn(tmp);

        freebn(a->z);
        a->z = NULL;
        freebn(a->x);
        a->x = tmp2;
        return true;
    } else if (a->curve->type == EC_EDWARDS) {
        /* Always normalised */
        return true;
    } else {
        return false;
    }
}

static struct ec_point *ecp_doublew(const struct ec_point *a, bool aminus3)
{
    Bignum S, M, outx, outy, outz;

    if (bignum_cmp(a->y, Zero) == 0)
    {
        /* Identity */
        return ec_point_new(a->curve, NULL, NULL, NULL, true);
    }

    /* S = 4*X*Y^2 */
    {
        Bignum Y2, XY2, _2XY2;

        Y2 = ecf_square(a->y, a->curve);
        XY2 = modmul(a->x, Y2, a->curve->p);
        freebn(Y2);

        _2XY2 = ecf_double(XY2, a->curve);
        freebn(XY2);
        S = ecf_double(_2XY2, a->curve);
        freebn(_2XY2);
    }

    /* Faster calculation if a = -3 */
    if (aminus3) {
        /* if a = -3, then M can also be calculated as M = 3*(X + Z^2)*(X - Z^2) */
        Bignum Z2, XpZ2, XmZ2, second;

        if (a->z == NULL) {
            Z2 = copybn(One);
        } else {
            Z2 = ecf_square(a->z, a->curve);
        }

        XpZ2 = ecf_add(a->x, Z2, a->curve);
        XmZ2 = modsub(a->x, Z2, a->curve->p);
        freebn(Z2);

        second = modmul(XpZ2, XmZ2, a->curve->p);
        freebn(XpZ2);
        freebn(XmZ2);

        M = ecf_treble(second, a->curve);
        freebn(second);
    } else {
        /* M = 3*X^2 + a*Z^4 */
        Bignum _3X2, X2, aZ4;

        if (a->z == NULL) {
            aZ4 = copybn(a->curve->w.a);
        } else {
            Bignum Z2, Z4;

            Z2 = ecf_square(a->z, a->curve);
            Z4 = ecf_square(Z2, a->curve);
            freebn(Z2);
            aZ4 = modmul(a->curve->w.a, Z4, a->curve->p);
            freebn(Z4);
        }

        X2 = modmul(a->x, a->x, a->curve->p);
        _3X2 = ecf_treble(X2, a->curve);
        freebn(X2);
        M = ecf_add(_3X2, aZ4, a->curve);
        freebn(_3X2);
        freebn(aZ4);
    }

    /* X' = M^2 - 2*S */
    {
        Bignum M2, _2S;

        M2 = ecf_square(M, a->curve);
        _2S = ecf_double(S, a->curve);
        outx = modsub(M2, _2S, a->curve->p);
        freebn(M2);
        freebn(_2S);
    }

    /* Y' = M*(S - X') - 8*Y^4 */
    {
        Bignum SX, MSX, Eight, Y2, Y4, _8Y4;

        SX = modsub(S, outx, a->curve->p);
        freebn(S);
        MSX = modmul(M, SX, a->curve->p);
        freebn(SX);
        freebn(M);
        Y2 = ecf_square(a->y, a->curve);
        Y4 = ecf_square(Y2, a->curve);
        freebn(Y2);
        Eight = bignum_from_long(8);
        _8Y4 = modmul(Eight, Y4, a->curve->p);
        freebn(Eight);
        freebn(Y4);
        outy = modsub(MSX, _8Y4, a->curve->p);
        freebn(MSX);
        freebn(_8Y4);
    }

    /* Z' = 2*Y*Z */
    {
        Bignum YZ;

        if (a->z == NULL) {
            YZ = copybn(a->y);
        } else {
            YZ = modmul(a->y, a->z, a->curve->p);
        }

        outz = ecf_double(YZ, a->curve);
        freebn(YZ);
    }

    return ec_point_new(a->curve, outx, outy, outz, false);
}

static struct ec_point *ecp_doublem(const struct ec_point *a)
{
    Bignum z, outx, outz, xpz, xmz;

    z = a->z;
    if (!z) {
        z = One;
    }

    /* 4xz = (x + z)^2 - (x - z)^2 */
    {
        Bignum tmp;

        tmp = ecf_add(a->x, z, a->curve);
        xpz = ecf_square(tmp, a->curve);
        freebn(tmp);

        tmp = modsub(a->x, z, a->curve->p);
        xmz = ecf_square(tmp, a->curve);
        freebn(tmp);
    }

    /* outx = (x + z)^2 * (x - z)^2 */
    outx = modmul(xpz, xmz, a->curve->p);

    /* outz = 4xz * ((x - z)^2 + ((A + 2) / 4)*4xz) */
    {
        Bignum _4xz, tmp, tmp2, tmp3;

        tmp = bignum_from_long(2);
        tmp2 = ecf_add(a->curve->m.a, tmp, a->curve);
        freebn(tmp);

        _4xz = modsub(xpz, xmz, a->curve->p);
        freebn(xpz);
        tmp = modmul(tmp2, _4xz, a->curve->p);
        freebn(tmp2);

        tmp2 = bignum_from_long(4);
        tmp3 = modinv(tmp2, a->curve->p);
        freebn(tmp2);
        if (!tmp3) {
            freebn(tmp);
            freebn(_4xz);
            freebn(outx);
            freebn(xmz);
            return NULL;
        }
        tmp2 = modmul(tmp, tmp3, a->curve->p);
        freebn(tmp);
        freebn(tmp3);

        tmp = ecf_add(xmz, tmp2, a->curve);
        freebn(xmz);
        freebn(tmp2);
        outz = modmul(_4xz, tmp, a->curve->p);
        freebn(_4xz);
        freebn(tmp);
    }

    return ec_point_new(a->curve, outx, NULL, outz, false);
}

/* Forward declaration for Edwards curve doubling */
static struct ec_point *ecp_add(const struct ec_point *a,
                                const struct ec_point *b,
                                bool aminus3);

static struct ec_point *ecp_double(const struct ec_point *a, bool aminus3)
{
    if (a->infinity)
    {
        /* Identity */
        return ec_point_new(a->curve, NULL, NULL, NULL, true);
    }

    if (a->curve->type == EC_EDWARDS)
    {
        return ecp_add(a, a, aminus3);
    }
    else if (a->curve->type == EC_WEIERSTRASS)
    {
        return ecp_doublew(a, aminus3);
    }
    else
    {
        return ecp_doublem(a);
    }
}

static struct ec_point *ecp_addw(const struct ec_point *a,
                                 const struct ec_point *b,
                                 bool aminus3)
{
    Bignum U1, U2, S1, S2, outx, outy, outz;

    /* U1 = X1*Z2^2 */
    /* S1 = Y1*Z2^3 */
    if (b->z) {
        Bignum Z2, Z3;

        Z2 = ecf_square(b->z, a->curve);
        U1 = modmul(a->x, Z2, a->curve->p);
        Z3 = modmul(Z2, b->z, a->curve->p);
        freebn(Z2);
        S1 = modmul(a->y, Z3, a->curve->p);
        freebn(Z3);
    } else {
        U1 = copybn(a->x);
        S1 = copybn(a->y);
    }

    /* U2 = X2*Z1^2 */
    /* S2 = Y2*Z1^3 */
    if (a->z) {
        Bignum Z2, Z3;

        Z2 = ecf_square(a->z, b->curve);
        U2 = modmul(b->x, Z2, b->curve->p);
        Z3 = modmul(Z2, a->z, b->curve->p);
        freebn(Z2);
        S2 = modmul(b->y, Z3, b->curve->p);
        freebn(Z3);
    } else {
        U2 = copybn(b->x);
        S2 = copybn(b->y);
    }

    /* Check if multiplying by self */
    if (bignum_cmp(U1, U2) == 0)
    {
        freebn(U1);
        freebn(U2);
        if (bignum_cmp(S1, S2) == 0)
        {
            freebn(S1);
            freebn(S2);
            return ecp_double(a, aminus3);
        }
        else
        {
            freebn(S1);
            freebn(S2);
            /* Infinity */
            return ec_point_new(a->curve, NULL, NULL, NULL, true);
        }
    }

    {
        Bignum H, R, UH2, H3;

        /* H = U2 - U1 */
        H = modsub(U2, U1, a->curve->p);
        freebn(U2);

        /* R = S2 - S1 */
        R = modsub(S2, S1, a->curve->p);
        freebn(S2);

        /* X3 = R^2 - H^3 - 2*U1*H^2 */
        {
            Bignum R2, H2, _2UH2, first;

            H2 = ecf_square(H, a->curve);
            UH2 = modmul(U1, H2, a->curve->p);
            freebn(U1);
            H3 = modmul(H2, H, a->curve->p);
            freebn(H2);
            R2 = ecf_square(R, a->curve);
            _2UH2 = ecf_double(UH2, a->curve);
            first = modsub(R2, H3, a->curve->p);
            freebn(R2);
            outx = modsub(first, _2UH2, a->curve->p);
            freebn(first);
            freebn(_2UH2);
        }

        /* Y3 = R*(U1*H^2 - X3) - S1*H^3 */
        {
            Bignum RUH2mX, UH2mX, SH3;

            UH2mX = modsub(UH2, outx, a->curve->p);
            freebn(UH2);
            RUH2mX = modmul(R, UH2mX, a->curve->p);
            freebn(UH2mX);
            freebn(R);
            SH3 = modmul(S1, H3, a->curve->p);
            freebn(S1);
            freebn(H3);

            outy = modsub(RUH2mX, SH3, a->curve->p);
            freebn(RUH2mX);
            freebn(SH3);
        }

        /* Z3 = H*Z1*Z2 */
        if (a->z && b->z) {
            Bignum ZZ;

            ZZ = modmul(a->z, b->z, a->curve->p);
            outz = modmul(H, ZZ, a->curve->p);
            freebn(H);
            freebn(ZZ);
        } else if (a->z) {
            outz = modmul(H, a->z, a->curve->p);
            freebn(H);
        } else if (b->z) {
            outz = modmul(H, b->z, a->curve->p);
            freebn(H);
        } else {
            outz = H;
        }
    }

    return ec_point_new(a->curve, outx, outy, outz, false);
}

static struct ec_point *ecp_addm(const struct ec_point *a,
                                 const struct ec_point *b,
                                 const struct ec_point *base)
{
    Bignum outx, outz, az, bz;

    az = a->z;
    if (!az) {
        az = One;
    }
    bz = b->z;
    if (!bz) {
        bz = One;
    }

    /* a-b is maintained at 1 due to Montgomery ladder implementation */
    /* Xa+b = Za-b * ((Xa - Za)*(Xb + Zb) + (Xa + Za)*(Xb - Zb))^2 */
    /* Za+b = Xa-b * ((Xa - Za)*(Xb + Zb) - (Xa + Za)*(Xb - Zb))^2 */
    {
        Bignum tmp, tmp2, tmp3, tmp4;

        /* (Xa + Za) * (Xb - Zb) */
        tmp = ecf_add(a->x, az, a->curve);
        tmp2 = modsub(b->x, bz, a->curve->p);
        tmp3 = modmul(tmp, tmp2, a->curve->p);
        freebn(tmp);
        freebn(tmp2);

        /* (Xa - Za) * (Xb + Zb) */
        tmp = modsub(a->x, az, a->curve->p);
        tmp2 = ecf_add(b->x, bz, a->curve);
        tmp4 = modmul(tmp, tmp2, a->curve->p);
        freebn(tmp);
        freebn(tmp2);

        tmp = ecf_add(tmp3, tmp4, a->curve);
        outx = ecf_square(tmp, a->curve);
        freebn(tmp);

        tmp = modsub(tmp3, tmp4, a->curve->p);
        freebn(tmp3);
        freebn(tmp4);
        tmp2 = ecf_square(tmp, a->curve);
        freebn(tmp);
        outz = modmul(base->x, tmp2, a->curve->p);
        freebn(tmp2);
    }

    return ec_point_new(a->curve, outx, NULL, outz, false);
}

static struct ec_point *ecp_adde(const struct ec_point *a,
                                 const struct ec_point *b)
{
    Bignum outx, outy, dmul;

    /* outx = (a->x * b->y + b->x * a->y) /
     *        (1 + a->curve->e.d * a->x * b->x * a->y * b->y) */
    {
        Bignum tmp, tmp2, tmp3, tmp4;

        tmp = modmul(a->x, b->y, a->curve->p);
        tmp2 = modmul(b->x, a->y, a->curve->p);
        tmp3 = ecf_add(tmp, tmp2, a->curve);

        tmp4 = modmul(tmp, tmp2, a->curve->p);
        freebn(tmp);
        freebn(tmp2);
        dmul = modmul(a->curve->e.d, tmp4, a->curve->p);
        freebn(tmp4);

        tmp = ecf_add(One, dmul, a->curve);
        tmp2 = modinv(tmp, a->curve->p);
        freebn(tmp);
        if (!tmp2)
        {
            freebn(tmp3);
            freebn(dmul);
            return NULL;
        }

        outx = modmul(tmp3, tmp2, a->curve->p);
        freebn(tmp3);
        freebn(tmp2);
    }

    /* outy = (a->y * b->y + a->x * b->x) /
     *        (1 - a->curve->e.d * a->x * b->x * a->y * b->y) */
    {
        Bignum tmp, tmp2, tmp3, tmp4;

        tmp = modsub(One, dmul, a->curve->p);
        freebn(dmul);

        tmp2 = modinv(tmp, a->curve->p);
        freebn(tmp);
        if (!tmp2)
        {
            freebn(outx);
            return NULL;
        }

        tmp = modmul(a->y, b->y, a->curve->p);
        tmp3 = modmul(a->x, b->x, a->curve->p);
        tmp4 = ecf_add(tmp, tmp3, a->curve);
        freebn(tmp);
        freebn(tmp3);

        outy = modmul(tmp4, tmp2, a->curve->p);
        freebn(tmp4);
        freebn(tmp2);
    }

    return ec_point_new(a->curve, outx, outy, NULL, false);
}

static struct ec_point *ecp_add(const struct ec_point *a,
                                const struct ec_point *b,
                                bool aminus3)
{
    if (a->curve != b->curve) {
        return NULL;
    }

    /* Check if multiplying by infinity */
    if (a->infinity) return ec_point_copy(b);
    if (b->infinity) return ec_point_copy(a);

    if (a->curve->type == EC_EDWARDS)
    {
        return ecp_adde(a, b);
    }

    if (a->curve->type == EC_WEIERSTRASS)
    {
        return ecp_addw(a, b, aminus3);
    }

    return NULL;
}

static struct ec_point *ecp_mul_(
    const struct ec_point *a, const Bignum b, bool aminus3)
{
    struct ec_point *A, *ret;
    int bits, i;

    A = ec_point_copy(a);
    ret = ec_point_new(a->curve, NULL, NULL, NULL, true);

    bits = bignum_bitcount(b);
    for (i = 0; i < bits; ++i)
    {
        if (bignum_bit(b, i))
        {
            struct ec_point *tmp = ecp_add(ret, A, aminus3);
            ec_point_free(ret);
            ret = tmp;
        }
        if (i+1 != bits)
        {
            struct ec_point *tmp = ecp_double(A, aminus3);
            ec_point_free(A);
            A = tmp;
        }
    }

    ec_point_free(A);
    return ret;
}

static struct ec_point *ecp_mulw(const struct ec_point *a, const Bignum b)
{
    struct ec_point *ret = ecp_mul_(a, b, ec_aminus3(a->curve));

    if (!ecp_normalise(ret)) {
        ec_point_free(ret);
        return NULL;
    }

    return ret;
}

static struct ec_point *ecp_mule(const struct ec_point *a, const Bignum b)
{
    int i;
    struct ec_point *ret;

    ret = ec_point_new(a->curve, NULL, NULL, NULL, true);

    for (i = bignum_bitcount(b); i >= 0 && ret; --i)
    {
        {
            struct ec_point *tmp = ecp_double(ret, false);
            ec_point_free(ret);
            ret = tmp;
        }
        if (ret && bignum_bit(b, i))
        {
            struct ec_point *tmp = ecp_add(ret, a, false);
            ec_point_free(ret);
            ret = tmp;
        }
    }

    return ret;
}

static struct ec_point *ecp_mulm(const struct ec_point *p, const Bignum n)
{
    struct ec_point *P1, *P2;
    int bits, i;

    /* P1 <- P and P2 <- [2]P */
    P2 = ecp_double(p, false);
    P1 = ec_point_copy(p);

    /* for i = bits − 2 down to 0 */
    bits = bignum_bitcount(n);
    for (i = bits - 2; i >= 0; --i)
    {
        if (!bignum_bit(n, i))
        {
            /* P2 <- P1 + P2 */
            struct ec_point *tmp = ecp_addm(P1, P2, p);
            ec_point_free(P2);
            P2 = tmp;

            /* P1 <- [2]P1 */
            tmp = ecp_double(P1, false);
            ec_point_free(P1);
            P1 = tmp;
        }
        else
        {
            /* P1 <- P1 + P2 */
            struct ec_point *tmp = ecp_addm(P1, P2, p);
            ec_point_free(P1);
            P1 = tmp;

            /* P2 <- [2]P2 */
            tmp = ecp_double(P2, false);
            ec_point_free(P2);
            P2 = tmp;
        }
    }

    ec_point_free(P2);

    if (!ecp_normalise(P1)) {
        ec_point_free(P1);
        return NULL;
    }

    return P1;
}

/* Not static because it is used by sshecdsag.c to generate a new key */
struct ec_point *ecp_mul(const struct ec_point *a, const Bignum b)
{
    if (a->curve->type == EC_WEIERSTRASS) {
        return ecp_mulw(a, b);
    } else if (a->curve->type == EC_EDWARDS) {
        return ecp_mule(a, b);
    } else {
        return ecp_mulm(a, b);
    }
}

static struct ec_point *ecp_summul(const Bignum a, const Bignum b,
                                   const struct ec_point *point)
{
    struct ec_point *aG, *bP, *ret;
    bool aminus3;

    if (point->curve->type != EC_WEIERSTRASS) {
        return NULL;
    }

    aminus3 = ec_aminus3(point->curve);

    aG = ecp_mul_(&point->curve->w.G, a, aminus3);
    if (!aG) return NULL;
    bP = ecp_mul_(point, b, aminus3);
    if (!bP) {
        ec_point_free(aG);
        return NULL;
    }

    ret = ecp_add(aG, bP, aminus3);

    ec_point_free(aG);
    ec_point_free(bP);

    if (!ecp_normalise(ret)) {
        ec_point_free(ret);
        return NULL;
    }

    return ret;
}
static Bignum *ecp_edx(const struct ec_curve *curve, const Bignum y)
{
    /* Get the x value on the given Edwards curve for a given y */
    Bignum x, xx;

    /* xx = (y^2 - 1) / (d * y^2 + 1) */
    {
        Bignum tmp, tmp2, tmp3;

        tmp = ecf_square(y, curve);
        tmp2 = modmul(curve->e.d, tmp, curve->p);
        tmp3 = ecf_add(tmp2, One, curve);
        freebn(tmp2);
        tmp2 = modinv(tmp3, curve->p);
        freebn(tmp3);
        if (!tmp2) {
            freebn(tmp);
            return NULL;
        }

        tmp3 = modsub(tmp, One, curve->p);
        freebn(tmp);
        xx = modmul(tmp3, tmp2, curve->p);
        freebn(tmp3);
        freebn(tmp2);
    }

    /* x = xx^((p + 3) / 8) */
    {
        Bignum tmp, tmp2;

        tmp = bignum_add_long(curve->p, 3);
        tmp2 = bignum_rshift(tmp, 3);
        freebn(tmp);
        x = modpow(xx, tmp2, curve->p);
        freebn(tmp2);
    }

    /* if x^2 - xx != 0 then x = x*(2^((p - 1) / 4)) */
    {
        Bignum tmp, tmp2;

        tmp = ecf_square(x, curve);
        tmp2 = modsub(tmp, xx, curve->p);
        freebn(tmp);
        freebn(xx);
        if (bignum_cmp(tmp2, Zero)) {
            Bignum tmp3;

            freebn(tmp2);

            tmp = modsub(curve->p, One, curve->p);
            tmp2 = bignum_rshift(tmp, 2);
            freebn(tmp);
            tmp = bignum_from_long(2);
            tmp3 = modpow(tmp, tmp2, curve->p);
            freebn(tmp);
            freebn(tmp2);

            tmp = modmul(x, tmp3, curve->p);
            freebn(x);
            freebn(tmp3);
            x = tmp;
        } else {
            freebn(tmp2);
        }
    }

    /* if x % 2 != 0 then x = p - x */
    if (bignum_bit(x, 0)) {
        Bignum tmp = modsub(curve->p, x, curve->p);
        freebn(x);
        x = tmp;
    }

    return x;
}

/* ----------------------------------------------------------------------
 * Public point from private
 */

struct ec_point *ec_public(const Bignum privateKey, const struct ec_curve *curve)
{
    if (curve->type == EC_WEIERSTRASS) {
        return ecp_mul(&curve->w.G, privateKey);
    } else if (curve->type == EC_EDWARDS) {
        /* hash = H(sk) (where hash creates 2 * fieldBits)
         * b = fieldBits
         * a = 2^(b-2) + SUM(2^i * h_i) for i = 2 -> b-2
         * publicKey = aB */
        struct ec_point *ret;
        unsigned char hash[512/8];
        Bignum a;
        int i, keylen;
        SHA512_State s;
        SHA512_Init(&s);

        keylen = curve->fieldBits / 8;
        for (i = 0; i < keylen; ++i)
            put_byte(&s, bignum_byte(privateKey, i));
        SHA512_Final(&s, hash);

        /* The second part is simply turning the hash into a Bignum,
         * however the 2^(b-2) bit *must* be set, and the bottom 3
         * bits *must* not be */
        hash[0] &= 0xf8; /* Unset bottom 3 bits (if set) */
        hash[31] &= 0x7f; /* Unset above (b-2) */
        hash[31] |= 0x40; /* Set 2^(b-2) */
        /* Chop off the top part and convert to int */
        a = bignum_from_bytes_le(hash, 32);

        ret = ecp_mul(&curve->e.B, a);
        freebn(a);
        return ret;
    } else {
        return NULL;
    }
}

/* ----------------------------------------------------------------------
 * Basic sign and verify routines
 */

static bool _ecdsa_verify(const struct ec_point *publicKey,
                          const unsigned char *data, const int dataLen,
                          const Bignum r, const Bignum s)
{
    int z_bits, n_bits;
    Bignum z;
    bool valid = false;

    if (publicKey->curve->type != EC_WEIERSTRASS) {
        return false;
    }

    /* Sanity checks */
    if (bignum_cmp(r, Zero) == 0 || bignum_cmp(r, publicKey->curve->w.n) >= 0
        || bignum_cmp(s, Zero) == 0 || bignum_cmp(s, publicKey->curve->w.n) >= 0)
    {
        return false;
    }

    /* z = left most bitlen(curve->n) of data */
    z = bignum_from_bytes(data, dataLen);
    n_bits = bignum_bitcount(publicKey->curve->w.n);
    z_bits = bignum_bitcount(z);
    if (z_bits > n_bits)
    {
        Bignum tmp = bignum_rshift(z, z_bits - n_bits);
        freebn(z);
        z = tmp;
    }

    /* Ensure z in range of n */
    {
        Bignum tmp = bigmod(z, publicKey->curve->w.n);
        freebn(z);
        z = tmp;
    }

    /* Calculate signature */
    {
        Bignum w, x, u1, u2;
        struct ec_point *tmp;

        w = modinv(s, publicKey->curve->w.n);
        if (!w) {
            freebn(z);
            return false;
        }
        u1 = modmul(z, w, publicKey->curve->w.n);
        u2 = modmul(r, w, publicKey->curve->w.n);
        freebn(w);

        tmp = ecp_summul(u1, u2, publicKey);
        freebn(u1);
        freebn(u2);
        if (!tmp) {
            freebn(z);
            return false;
        }

        x = bigmod(tmp->x, publicKey->curve->w.n);
        ec_point_free(tmp);

        valid = (bignum_cmp(r, x) == 0);
        freebn(x);
    }

    freebn(z);

    return valid;
}

static void _ecdsa_sign(const Bignum privateKey, const struct ec_curve *curve,
                        const unsigned char *data, const int dataLen,
                        Bignum *r, Bignum *s)
{
    unsigned char digest[20];
    int z_bits, n_bits;
    Bignum z, k;
    struct ec_point *kG;

    *r = NULL;
    *s = NULL;

    if (curve->type != EC_WEIERSTRASS) {
        return;
    }

    /* z = left most bitlen(curve->n) of data */
    z = bignum_from_bytes(data, dataLen);
    n_bits = bignum_bitcount(curve->w.n);
    z_bits = bignum_bitcount(z);
    if (z_bits > n_bits)
    {
        Bignum tmp;
        tmp = bignum_rshift(z, z_bits - n_bits);
        freebn(z);
        z = tmp;
    }

    /* Generate k between 1 and curve->n, using the same deterministic
     * k generation system we use for conventional DSA. */
    SHA_Simple(data, dataLen, digest);
    k = dss_gen_k("ECDSA deterministic k generator", curve->w.n, privateKey,
                  digest, sizeof(digest));

    kG = ecp_mul(&curve->w.G, k);
    if (!kG) {
        freebn(z);
        freebn(k);
        return;
    }

    /* r = kG.x mod n */
    *r = bigmod(kG->x, curve->w.n);
    ec_point_free(kG);

    /* s = (z + r * priv)/k mod n */
    {
        Bignum rPriv, zMod, first, firstMod, kInv;
        rPriv = modmul(*r, privateKey, curve->w.n);
        zMod = bigmod(z, curve->w.n);
        freebn(z);
        first = bigadd(rPriv, zMod);
        freebn(rPriv);
        freebn(zMod);
        firstMod = bigmod(first, curve->w.n);
        freebn(first);
        kInv = modinv(k, curve->w.n);
        freebn(k);
        if (!kInv) {
            freebn(firstMod);
            freebn(*r);
            return;
        }
        *s = modmul(firstMod, kInv, curve->w.n);
        freebn(firstMod);
        freebn(kInv);
    }
}

/* ----------------------------------------------------------------------
 * Misc functions
 */

static Bignum BinarySource_get_mp_le(BinarySource *src)
{
    ptrlen mp_str = get_string(src);
    return bignum_from_bytes_le(mp_str.ptr, mp_str.len);
}
#define get_mp_le(src) BinarySource_get_mp_le(BinarySource_UPCAST(src))

static bool decodepoint_ed(const char *p, int length, struct ec_point *point)
{
    /* Got some conversion to do, first read in the y co-ord */
    bool negative;

    point->y = bignum_from_bytes_le((const unsigned char*)p, length);
    if ((unsigned)bignum_bitcount(point->y) > point->curve->fieldBits) {
        freebn(point->y);
        point->y = NULL;
        return false;
    }
    /* Read x bit and then reset it */
    negative = bignum_bit(point->y, point->curve->fieldBits - 1);
    bignum_set_bit(point->y, point->curve->fieldBits - 1, 0);
    bn_restore_invariant(point->y);

    /* Get the x from the y */
    point->x = ecp_edx(point->curve, point->y);
    if (!point->x) {
        freebn(point->y);
        point->y = NULL;
        return false;
    }
    if (negative) {
        Bignum tmp = modsub(point->curve->p, point->x, point->curve->p);
        freebn(point->x);
        point->x = tmp;
    }

    /* Verify the point is on the curve */
    if (!ec_point_verify(point)) {
        freebn(point->x);
        point->x = NULL;
        freebn(point->y);
        point->y = NULL;
        return false;
    }

    return true;
}

static bool decodepoint(const char *p, int length, struct ec_point *point)
{
    if (point->curve->type == EC_EDWARDS) {
        return decodepoint_ed(p, length, point);
    }

    if (length < 1 || p[0] != 0x04) /* Only support uncompressed point */
        return false;
    /* Skip compression flag */
    ++p;
    --length;
    /* The two values must be equal length */
    if (length % 2 != 0) {
        point->x = NULL;
        point->y = NULL;
        point->z = NULL;
        return false;
    }
    length = length / 2;
    point->x = bignum_from_bytes(p, length);
    p += length;
    point->y = bignum_from_bytes(p, length);
    point->z = NULL;

    /* Verify the point is on the curve */
    if (!ec_point_verify(point)) {
        freebn(point->x);
        point->x = NULL;
        freebn(point->y);
        point->y = NULL;
        return false;
    }

    return true;
}

static bool BinarySource_get_point(BinarySource *src, struct ec_point *point)
{
    ptrlen str = get_string(src);
    if (get_err(src)) return false;
    return decodepoint(str.ptr, str.len, point);
}
#define get_point(src, pt) BinarySource_get_point(BinarySource_UPCAST(src), pt)

/* ----------------------------------------------------------------------
 * Exposed ECDSA interface
 */

struct ecsign_extra {
    struct ec_curve *(*curve)(void);
    const struct ssh_hashalg *hash;

    /* These fields are used by the OpenSSH PEM format importer/exporter */
    const unsigned char *oid;
    int oidlen;
};

static void ecdsa_freekey(ssh_key *key)
{
    struct ec_key *ec;

    if (!key) return;
    ec = container_of(key, struct ec_key, sshk);

    if (ec->publicKey.x)
        freebn(ec->publicKey.x);
    if (ec->publicKey.y)
        freebn(ec->publicKey.y);
    if (ec->publicKey.z)
        freebn(ec->publicKey.z);
    if (ec->privateKey)
        freebn(ec->privateKey);
    sfree(ec);
}

static ssh_key *ecdsa_new_pub(const ssh_keyalg *self, ptrlen data)
{
    const struct ecsign_extra *extra =
        (const struct ecsign_extra *)self->extra;
    BinarySource src[1];
    struct ec_key *ec;
    struct ec_curve *curve;

    BinarySource_BARE_INIT(src, data.ptr, data.len);
    get_string(src);

    curve = extra->curve();
    assert(curve->type == EC_WEIERSTRASS || curve->type == EC_EDWARDS);

    /* Curve name is duplicated for Weierstrass form */
    if (curve->type == EC_WEIERSTRASS) {
        if (!ptrlen_eq_string(get_string(src), curve->name))
            return NULL;
    }

    ec = snew(struct ec_key);
    ec->sshk.vt = self;

    ec->publicKey.curve = curve;
    ec->publicKey.infinity = false;
    ec->publicKey.x = NULL;
    ec->publicKey.y = NULL;
    ec->publicKey.z = NULL;
    ec->privateKey = NULL;
    if (!get_point(src, &ec->publicKey)) {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    if (!ec->publicKey.x || !ec->publicKey.y ||
        bignum_cmp(ec->publicKey.x, curve->p) >= 0 ||
        bignum_cmp(ec->publicKey.y, curve->p) >= 0)
    {
        ecdsa_freekey(&ec->sshk);
        ec = NULL;
    }

    return &ec->sshk;
}

static char *ecdsa_cache_str(ssh_key *key)
{
    struct ec_key *ec = container_of(key, struct ec_key, sshk);
    char *p;
    int len, i, pos, nibbles;
    static const char hex[] = "0123456789abcdef";
    if (!ec->publicKey.x || !ec->publicKey.y || !ec->publicKey.curve)
        return NULL;

    len = 4 + 2 + 1;                  /* 2 x "0x", punctuation, \0 */
    if (ec->publicKey.curve->name)
        len += strlen(ec->publicKey.curve->name); /* Curve name */
    len += 4 * (bignum_bitcount(ec->publicKey.x) + 15) / 16;
    len += 4 * (bignum_bitcount(ec->publicKey.y) + 15) / 16;
    p = snewn(len, char);

    pos = 0;
    if (ec->publicKey.curve->name)
        pos += sprintf(p + pos, "%s,", ec->publicKey.curve->name);
    pos += sprintf(p + pos, "0x");
    nibbles = (3 + bignum_bitcount(ec->publicKey.x)) / 4;
    if (nibbles < 1)
        nibbles = 1;
    for (i = nibbles; i--;) {
        p[pos++] =
            hex[(bignum_byte(ec->publicKey.x, i / 2) >> (4 * (i % 2))) & 0xF];
    }
    pos += sprintf(p + pos, ",0x");
    nibbles = (3 + bignum_bitcount(ec->publicKey.y)) / 4;
    if (nibbles < 1)
        nibbles = 1;
    for (i = nibbles; i--;) {
        p[pos++] =
            hex[(bignum_byte(ec->publicKey.y, i / 2) >> (4 * (i % 2))) & 0xF];
    }
    p[pos] = '\0';
    return p;
}

static void ecdsa_public_blob(ssh_key *key, BinarySink *bs)
{
    struct ec_key *ec = container_of(key, struct ec_key, sshk);
    int pointlen;
    int i;

    if (ec->publicKey.curve->type == EC_EDWARDS) {
        /* Edwards compressed form "ssh-ed25519" point y[:-1] + x[0:1] */

        pointlen = ec->publicKey.curve->fieldBits / 8;

        assert(pointlen >= 2);

        put_stringz(bs, ec->sshk.vt->ssh_id);
        put_uint32(bs, pointlen);

        /* Unset last bit of y and set first bit of x in its place */
        for (i = 0; i < pointlen - 1; ++i)
            put_byte(bs, bignum_byte(ec->publicKey.y, i));
        /* Unset last bit of y and set first bit of x in its place */
        put_byte(bs, ((bignum_byte(ec->publicKey.y, i) & 0x7f) |
                                (bignum_bit(ec->publicKey.x, 0) << 7)));
    } else if (ec->publicKey.curve->type == EC_WEIERSTRASS) {
        assert(ec->publicKey.curve->name);

        pointlen = (bignum_bitcount(ec->publicKey.curve->p) + 7) / 8;

        put_stringz(bs, ec->sshk.vt->ssh_id);
        put_stringz(bs, ec->publicKey.curve->name);
        put_uint32(bs, (2 * pointlen) + 1);
        put_byte(bs, 0x04);
        for (i = pointlen; i--;)
            put_byte(bs, bignum_byte(ec->publicKey.x, i));
        for (i = pointlen; i--;)
            put_byte(bs, bignum_byte(ec->publicKey.y, i));
    } else {
        assert(0 && "Bad key type in ecdsa_public_blob");
    }
}

static void ecdsa_private_blob(ssh_key *key, BinarySink *bs)
{
    struct ec_key *ec = container_of(key, struct ec_key, sshk);
    int keylen;
    int i;

    assert(ec->privateKey);

    if (ec->publicKey.curve->type == EC_EDWARDS) {
        /* Unsigned */
        keylen = (bignum_bitcount(ec->privateKey) + 7) / 8;
    } else {
        /* Signed */
        keylen = (bignum_bitcount(ec->privateKey) + 8) / 8;
    }

    put_uint32(bs, keylen);
    if (ec->publicKey.curve->type == EC_EDWARDS) {
        /* Little endian */
        for (i = 0; i < keylen; ++i)
            put_byte(bs, bignum_byte(ec->privateKey, i));
    } else {
        for (i = keylen; i--;)
            put_byte(bs, bignum_byte(ec->privateKey, i));
    }
}

static ssh_key *ecdsa_new_priv(const ssh_keyalg *self, ptrlen pub, ptrlen priv)
{
    BinarySource src[1];
    ssh_key *sshk;
    struct ec_key *ec;
    struct ec_point *publicKey;

    sshk = ecdsa_new_pub(self, pub);
    if (!sshk)
        return NULL;

    ec = container_of(sshk, struct ec_key, sshk);
    BinarySource_BARE_INIT(src, priv.ptr, priv.len);

    if (ec->publicKey.curve->type != EC_WEIERSTRASS
        && ec->publicKey.curve->type != EC_EDWARDS) {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    if (ec->publicKey.curve->type == EC_EDWARDS) {
        ec->privateKey = get_mp_le(src);
    } else {
        ec->privateKey = get_mp_ssh2(src);
    }
    if (!ec->privateKey) {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    /* Check that private key generates public key */
    publicKey = ec_public(ec->privateKey, ec->publicKey.curve);

    if (!publicKey ||
        bignum_cmp(publicKey->x, ec->publicKey.x) ||
        bignum_cmp(publicKey->y, ec->publicKey.y))
    {
        ecdsa_freekey(&ec->sshk);
        ec = NULL;
    }
    ec_point_free(publicKey);

    return &ec->sshk;
}

static ssh_key *ed25519_new_priv_openssh(const ssh_keyalg *self,
                                         BinarySource *src)
{
    struct ec_key *ec;
    struct ec_point *publicKey;
    ptrlen p, q;

    p = get_string(src);
    q = get_string(src);
    if (get_err(src) || p.len != 32 || q.len != 64)
        return NULL;

    ec = snew(struct ec_key);
    ec->sshk.vt = self;

    ec->publicKey.curve = ec_ed25519();
    ec->publicKey.infinity = false;
    ec->privateKey = NULL;
    ec->publicKey.x = NULL;
    ec->publicKey.z = NULL;
    ec->publicKey.y = NULL;

    if (!decodepoint_ed(p.ptr, p.len, &ec->publicKey))
    {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    ec->privateKey = bignum_from_bytes_le(q.ptr, 32);

    /* Check that private key generates public key */
    publicKey = ec_public(ec->privateKey, ec->publicKey.curve);

    if (!publicKey ||
        bignum_cmp(publicKey->x, ec->publicKey.x) ||
        bignum_cmp(publicKey->y, ec->publicKey.y))
    {
        ecdsa_freekey(&ec->sshk);
        ec = NULL;
    }
    ec_point_free(publicKey);

    /* The OpenSSH format for ed25519 private keys also for some
     * reason encodes an extra copy of the public key in the second
     * half of the secret-key string. Check that that's present and
     * correct as well, otherwise the key we think we've imported
     * won't behave identically to the way OpenSSH would have treated
     * it. */
    if (0 != memcmp((const char *)q.ptr + 32, p.ptr, 32)) {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    return &ec->sshk;
}

static void ed25519_openssh_blob(ssh_key *key, BinarySink *bs)
{
    struct ec_key *ec = container_of(key, struct ec_key, sshk);
    strbuf *pub;

    int pointlen;
    int keylen;
    int i;

    assert(ec->publicKey.curve->type == EC_EDWARDS);

    pointlen = (bignum_bitcount(ec->publicKey.y) + 7) / 8;
    keylen = (bignum_bitcount(ec->privateKey) + 7) / 8;

    /* Encode the public point */
    pub = strbuf_new();
    put_uint32(pub, pointlen);
    for (i = 0; i < pointlen - 1; ++i)
        put_byte(pub, bignum_byte(ec->publicKey.y, i));
    /* Unset last bit of y and set first bit of x in its place */
    put_byte(pub, ((bignum_byte(ec->publicKey.y, i) & 0x7f) |
                   (bignum_bit(ec->publicKey.x, 0) << 7)));

    put_data(bs, pub->s, pub->len);

    put_uint32(bs, keylen + pointlen);
    for (i = 0; i < keylen; ++i)
        put_byte(bs, bignum_byte(ec->privateKey, i));
    /* Now encode an extra copy of the public point as the second half
     * of the private key string, as the OpenSSH format for some
     * reason requires */
    put_data(bs, pub->s + 4, pub->len - 4);

    strbuf_free(pub);
}

static ssh_key *ecdsa_new_priv_openssh(const ssh_keyalg *self,
                                       BinarySource *src)
{
    const struct ecsign_extra *extra =
        (const struct ecsign_extra *)self->extra;
    struct ec_key *ec;
    struct ec_curve *curve;
    struct ec_point *publicKey;

    get_string(src);

    curve = extra->curve();
    assert(curve->type == EC_WEIERSTRASS);

    ec = snew(struct ec_key);
    ec->sshk.vt = self;

    ec->publicKey.curve = curve;
    ec->publicKey.infinity = false;
    ec->publicKey.x = NULL;
    ec->publicKey.y = NULL;
    ec->publicKey.z = NULL;
    if (!get_point(src, &ec->publicKey)) {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }
    ec->privateKey = NULL;

    if (!ec->publicKey.x || !ec->publicKey.y ||
        bignum_cmp(ec->publicKey.x, curve->p) >= 0 ||
        bignum_cmp(ec->publicKey.y, curve->p) >= 0)
    {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    ec->privateKey = get_mp_ssh2(src);
    if (ec->privateKey == NULL)
    {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    /* Now check that the private key makes the public key */
    publicKey = ec_public(ec->privateKey, ec->publicKey.curve);
    if (!publicKey)
    {
        ecdsa_freekey(&ec->sshk);
        return NULL;
    }

    if (bignum_cmp(ec->publicKey.x, publicKey->x) ||
        bignum_cmp(ec->publicKey.y, publicKey->y))
    {
        /* Private key doesn't make the public key on the given curve */
        ecdsa_freekey(&ec->sshk);
        ec_point_free(publicKey);
        return NULL;
    }

    ec_point_free(publicKey);

    return &ec->sshk;
}

static void ecdsa_openssh_blob(ssh_key *key, BinarySink *bs)
{
    struct ec_key *ec = container_of(key, struct ec_key, sshk);

    int pointlen;
    int i;

    assert(ec->publicKey.curve->type == EC_WEIERSTRASS);

    pointlen = (bignum_bitcount(ec->publicKey.curve->p) + 7) / 8;

    put_stringz(bs, ec->publicKey.curve->name);

    put_uint32(bs, 1 + (pointlen * 2));
    put_byte(bs, 0x04);
    for (i = pointlen; i--; )
        put_byte(bs, bignum_byte(ec->publicKey.x, i));
    for (i = pointlen; i--; )
        put_byte(bs, bignum_byte(ec->publicKey.y, i));

    put_mp_ssh2(bs, ec->privateKey);
}

static int ecdsa_pubkey_bits(const ssh_keyalg *self, ptrlen blob)
{
    ssh_key *sshk;
    struct ec_key *ec;
    int ret;

    sshk = ecdsa_new_pub(self, blob);
    if (!sshk)
        return -1;

    ec = container_of(sshk, struct ec_key, sshk);
    ret = ec->publicKey.curve->fieldBits;
    ecdsa_freekey(&ec->sshk);

    return ret;
}

static bool ecdsa_verify(ssh_key *key, ptrlen sig, ptrlen data)
{
    struct ec_key *ec = container_of(key, struct ec_key, sshk);
    const struct ecsign_extra *extra =
        (const struct ecsign_extra *)ec->sshk.vt->extra;
    BinarySource src[1];
    ptrlen sigstr;
    bool ret;

    if (!ec->publicKey.x || !ec->publicKey.y || !ec->publicKey.curve)
        return false;

    BinarySource_BARE_INIT(src, sig.ptr, sig.len);

    /* Check the signature starts with the algorithm name */
    if (!ptrlen_eq_string(get_string(src), ec->sshk.vt->ssh_id))
        return false;

    sigstr = get_string(src);
    if (get_err(src))
        return false;

    if (ec->publicKey.curve->type == EC_EDWARDS) {
        struct ec_point *r;
        int pointlen = ec->publicKey.curve->fieldBits / 8;
        Bignum s, h;

        /* Check that the signature is two times the length of a point */
        if (sigstr.len != pointlen * 2) {
            return false;
        }

        /* Check it's the 256 bit field so that SHA512 is the correct hash */
        if (ec->publicKey.curve->fieldBits != 256) {
            return false;
        }

        /* Get the signature */
        r = ec_point_new(ec->publicKey.curve, NULL, NULL, NULL, false);
        if (!r) {
            return false;
        }
        if (!decodepoint(sigstr.ptr, pointlen, r)) {
            ec_point_free(r);
            return false;
        }
        s = bignum_from_bytes_le(
            (const char *)sigstr.ptr + pointlen, pointlen);

        /* Get the hash of the encoded value of R + encoded value of pk + message */
        {
            int i;
            unsigned char digest[512 / 8];
            SHA512_State hs;
            SHA512_Init(&hs);

            /* Add encoded r (no need to encode it again, it was in
             * the signature) */
            put_data(&hs, sigstr.ptr, pointlen);

            /* Encode pk and add it */
            for (i = 0; i < pointlen - 1; ++i)
                put_byte(&hs, bignum_byte(ec->publicKey.y, i));
            /* Unset last bit of y and set first bit of x in its place */
            put_byte(&hs, ((bignum_byte(ec->publicKey.y, i) & 0x7f) |
                           (bignum_bit(ec->publicKey.x, 0) << 7)));

            /* Add the message itself */
            put_data(&hs, data.ptr, data.len);

            /* Get the hash */
            SHA512_Final(&hs, digest);

            /* Convert to Bignum */
            h = bignum_from_bytes_le(digest, sizeof(digest));
        }

        /* Verify sB == r + h*publicKey */
        {
            struct ec_point *lhs, *rhs, *tmp;

            /* lhs = sB */
            lhs = ecp_mul(&ec->publicKey.curve->e.B, s);
            freebn(s);
            if (!lhs) {
                ec_point_free(r);
                freebn(h);
                return false;
            }

            /* rhs = r + h*publicKey */
            tmp = ecp_mul(&ec->publicKey, h);
            freebn(h);
            if (!tmp) {
                ec_point_free(lhs);
                ec_point_free(r);
                return false;
            }
            rhs = ecp_add(r, tmp, false);
            ec_point_free(r);
            ec_point_free(tmp);
            if (!rhs) {
                ec_point_free(lhs);
                return false;
            }

            /* Check the point is the same */
            ret = !bignum_cmp(lhs->x, rhs->x);
            if (ret) {
                ret = !bignum_cmp(lhs->y, rhs->y);
                if (ret) {
                    ret = true;
                }
            }
            ec_point_free(lhs);
            ec_point_free(rhs);
        }
    } else {
        Bignum r, s;
        unsigned char digest[512 / 8];
        int digestLen;
        ssh_hash *hashctx;

        BinarySource_BARE_INIT(src, sigstr.ptr, sigstr.len);

        r = get_mp_ssh2(src);
        s = get_mp_ssh2(src);
        if (get_err(src)) {
            freebn(r);
            freebn(s);
            return false;
        }

        digestLen = extra->hash->hlen;
        assert(digestLen <= sizeof(digest));
        hashctx = ssh_hash_new(extra->hash);
        put_data(hashctx, data.ptr, data.len);
        ssh_hash_final(hashctx, digest);

        /* Verify the signature */
        ret = _ecdsa_verify(&ec->publicKey, digest, digestLen, r, s);

        freebn(r);
        freebn(s);
    }

    return ret;
}

static void ecdsa_sign(ssh_key *key, const void *data, int datalen,
                       unsigned flags, BinarySink *bs)
{
    struct ec_key *ec = container_of(key, struct ec_key, sshk);
    const struct ecsign_extra *extra =
        (const struct ecsign_extra *)ec->sshk.vt->extra;
    unsigned char digest[512 / 8];
    int digestLen;
    Bignum r = NULL, s = NULL;
    int i;

    assert(ec->privateKey);
    assert(ec->publicKey.curve);

    if (ec->publicKey.curve->type == EC_EDWARDS) {
        struct ec_point *rp;
        int pointlen = ec->publicKey.curve->fieldBits / 8;

        /* hash = H(sk) (where hash creates 2 * fieldBits)
         * b = fieldBits
         * a = 2^(b-2) + SUM(2^i * h_i) for i = 2 -> b-2
         * r = H(h[b/8:b/4] + m)
         * R = rB
         * S = (r + H(encodepoint(R) + encodepoint(pk) + m) * a) % l */
        {
            unsigned char hash[512/8];
            Bignum a;
            SHA512_State hs;
            SHA512_Init(&hs);

            for (i = 0; i < pointlen; ++i)
                put_byte(&hs, bignum_byte(ec->privateKey, i));

            SHA512_Final(&hs, hash);

            /* The second part is simply turning the hash into a
             * Bignum, however the 2^(b-2) bit *must* be set, and the
             * bottom 3 bits *must* not be */
            hash[0] &= 0xf8; /* Unset bottom 3 bits (if set) */
            hash[31] &= 0x7f; /* Unset above (b-2) */
            hash[31] |= 0x40; /* Set 2^(b-2) */
            /* Chop off the top part and convert to int */
            a = bignum_from_bytes_le(hash, 32);

            SHA512_Init(&hs);
            put_data(&hs, hash+(ec->publicKey.curve->fieldBits / 8),
                     ((ec->publicKey.curve->fieldBits / 4) -
                      (ec->publicKey.curve->fieldBits / 8)));
            put_data(&hs, data, datalen);
            SHA512_Final(&hs, hash);

            r = bignum_from_bytes_le(hash, 512/8);
            rp = ecp_mul(&ec->publicKey.curve->e.B, r);
            assert(rp);

            /* Now calculate s */
            SHA512_Init(&hs);
            /* Encode the point R */
            for (i = 0; i < pointlen - 1; ++i)
                put_byte(&hs, bignum_byte(rp->y, i));
            /* Unset last bit of y and set first bit of x in its place */
            put_byte(&hs, ((bignum_byte(rp->y, i) & 0x7f) |
                           (bignum_bit(rp->x, 0) << 7)));

            /* Encode the point pk */
            for (i = 0; i < pointlen - 1; ++i)
                put_byte(&hs, bignum_byte(ec->publicKey.y, i));
            /* Unset last bit of y and set first bit of x in its place */
            put_byte(&hs, ((bignum_byte(ec->publicKey.y, i) & 0x7f) |
                           (bignum_bit(ec->publicKey.x, 0) << 7)));

            /* Add the message */
            put_data(&hs, data, datalen);
            SHA512_Final(&hs, hash);

            {
                Bignum tmp, tmp2;

                tmp = bignum_from_bytes_le(hash, 512/8);
                tmp2 = modmul(tmp, a, ec->publicKey.curve->e.l);
                freebn(a);
                freebn(tmp);
                tmp = bigadd(r, tmp2);
                freebn(r);
                freebn(tmp2);
                s = bigmod(tmp, ec->publicKey.curve->e.l);
                freebn(tmp);
            }
        }

        /* Format the output */
        put_stringz(bs, ec->sshk.vt->ssh_id);
        pointlen = ec->publicKey.curve->fieldBits / 8;
        put_uint32(bs, pointlen * 2);

        /* Encode the point */
        for (i = 0; i < pointlen - 1; ++i)
            put_byte(bs, bignum_byte(rp->y, i));
        /* Unset last bit of y and set first bit of x in its place */
        put_byte(bs, ((bignum_byte(rp->y, i) & 0x7f) |
                                (bignum_bit(rp->x, 0) << 7)));
        ec_point_free(rp);

        /* Encode the int */
        for (i = 0; i < pointlen; ++i)
            put_byte(bs, bignum_byte(s, i));
        freebn(s);
    } else {
        ssh_hash *hashctx;
        strbuf *substr;

        digestLen = extra->hash->hlen;
        assert(digestLen <= sizeof(digest));
        hashctx = ssh_hash_new(extra->hash);
        put_data(hashctx, data, datalen);
        ssh_hash_final(hashctx, digest);

        /* Do the signature */
        _ecdsa_sign(ec->privateKey, ec->publicKey.curve, digest, digestLen, &r, &s);
        assert(r);
        assert(s);

        /* Format the output */
        put_stringz(bs, ec->sshk.vt->ssh_id);

        substr = strbuf_new();
        put_mp_ssh2(substr, r);
        put_mp_ssh2(substr, s);
        put_stringsb(bs, substr);

        freebn(r);
        freebn(s);
    }
}

const struct ecsign_extra sign_extra_ed25519 = {
    ec_ed25519, NULL,
    NULL, 0,
};
const ssh_keyalg ssh_ecdsa_ed25519 = {
    ecdsa_new_pub,
    ecdsa_new_priv,
    ed25519_new_priv_openssh,

    ecdsa_freekey,
    ecdsa_sign,
    ecdsa_verify,
    ecdsa_public_blob,
    ecdsa_private_blob,
    ed25519_openssh_blob,
    ecdsa_cache_str,

    ecdsa_pubkey_bits,

    "ssh-ed25519",
    "ssh-ed25519",
    &sign_extra_ed25519,
    0, /* no supported flags */
};

/* OID: 1.2.840.10045.3.1.7 (ansiX9p256r1) */
static const unsigned char nistp256_oid[] = {
    0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07
};
const struct ecsign_extra sign_extra_nistp256 = {
    ec_p256, &ssh_sha256,
    nistp256_oid, lenof(nistp256_oid),
};
const ssh_keyalg ssh_ecdsa_nistp256 = {
    ecdsa_new_pub,
    ecdsa_new_priv,
    ecdsa_new_priv_openssh,

    ecdsa_freekey,
    ecdsa_sign,
    ecdsa_verify,
    ecdsa_public_blob,
    ecdsa_private_blob,
    ecdsa_openssh_blob,
    ecdsa_cache_str,

    ecdsa_pubkey_bits,

    "ecdsa-sha2-nistp256",
    "ecdsa-sha2-nistp256",
    &sign_extra_nistp256,
    0, /* no supported flags */
};

/* OID: 1.3.132.0.34 (secp384r1) */
static const unsigned char nistp384_oid[] = {
    0x2b, 0x81, 0x04, 0x00, 0x22
};
const struct ecsign_extra sign_extra_nistp384 = {
    ec_p384, &ssh_sha384,
    nistp384_oid, lenof(nistp384_oid),
};
const ssh_keyalg ssh_ecdsa_nistp384 = {
    ecdsa_new_pub,
    ecdsa_new_priv,
    ecdsa_new_priv_openssh,

    ecdsa_freekey,
    ecdsa_sign,
    ecdsa_verify,
    ecdsa_public_blob,
    ecdsa_private_blob,
    ecdsa_openssh_blob,
    ecdsa_cache_str,

    ecdsa_pubkey_bits,

    "ecdsa-sha2-nistp384",
    "ecdsa-sha2-nistp384",
    &sign_extra_nistp384,
    0, /* no supported flags */
};

/* OID: 1.3.132.0.35 (secp521r1) */
static const unsigned char nistp521_oid[] = {
    0x2b, 0x81, 0x04, 0x00, 0x23
};
const struct ecsign_extra sign_extra_nistp521 = {
    ec_p521, &ssh_sha512,
    nistp521_oid, lenof(nistp521_oid),
};
const ssh_keyalg ssh_ecdsa_nistp521 = {
    ecdsa_new_pub,
    ecdsa_new_priv,
    ecdsa_new_priv_openssh,

    ecdsa_freekey,
    ecdsa_sign,
    ecdsa_verify,
    ecdsa_public_blob,
    ecdsa_private_blob,
    ecdsa_openssh_blob,
    ecdsa_cache_str,

    ecdsa_pubkey_bits,

    "ecdsa-sha2-nistp521",
    "ecdsa-sha2-nistp521",
    &sign_extra_nistp521,
    0, /* no supported flags */
};

/* ----------------------------------------------------------------------
 * Exposed ECDH interface
 */

struct eckex_extra {
    struct ec_curve *(*curve)(void);
};

static Bignum ecdh_calculate(const Bignum private,
                             const struct ec_point *public)
{
    struct ec_point *p;
    Bignum ret;
    p = ecp_mul(public, private);
    if (!p) return NULL;
    ret = p->x;
    p->x = NULL;

    if (p->curve->type == EC_MONTGOMERY) {
        /*
         * Endianness-swap. The Curve25519 algorithm definition
         * assumes you were doing your computation in arrays of 32
         * little-endian bytes, and now specifies that you take your
         * final one of those and convert it into a bignum in
         * _network_ byte order, i.e. big-endian.
         *
         * In particular, the spec says, you convert the _whole_ 32
         * bytes into a bignum. That is, on the rare occasions that
         * p->x has come out with the most significant 8 bits zero, we
         * have to imagine that being represented by a 32-byte string
         * with the last byte being zero, so that has to be converted
         * into an SSH-2 bignum with the _low_ byte zero, i.e. a
         * multiple of 256.
         */
        int i;
        int bytes = (p->curve->fieldBits+7) / 8;
        unsigned char *byteorder = snewn(bytes, unsigned char);
        for (i = 0; i < bytes; ++i) {
            byteorder[i] = bignum_byte(ret, i);
        }
        freebn(ret);
        ret = bignum_from_bytes(byteorder, bytes);
        smemclr(byteorder, bytes);
        sfree(byteorder);
    }

    ec_point_free(p);
    return ret;
}

const char *ssh_ecdhkex_curve_textname(const struct ssh_kex *kex)
{
    const struct eckex_extra *extra = (const struct eckex_extra *)kex->extra;
    struct ec_curve *curve = extra->curve();
    return curve->textname;
}

struct ec_key *ssh_ecdhkex_newkey(const struct ssh_kex *kex)
{
    const struct eckex_extra *extra = (const struct eckex_extra *)kex->extra;
    struct ec_curve *curve;
    struct ec_key *key;
    struct ec_point *publicKey;

    curve = extra->curve();

    key = snew(struct ec_key);

    key->sshk.vt = NULL;
    key->publicKey.curve = curve;

    if (curve->type == EC_MONTGOMERY) {
        unsigned char bytes[32] = {0};
        int i;

        for (i = 0; i < sizeof(bytes); ++i)
        {
            bytes[i] = (unsigned char)random_byte();
        }
        bytes[0] &= 248;
        bytes[31] &= 127;
        bytes[31] |= 64;
        key->privateKey = bignum_from_bytes_le(bytes, sizeof(bytes));
        smemclr(bytes, sizeof(bytes));
        if (!key->privateKey) {
            sfree(key);
            return NULL;
        }
        publicKey = ecp_mul(&key->publicKey.curve->m.G, key->privateKey);
        if (!publicKey) {
            freebn(key->privateKey);
            sfree(key);
            return NULL;
        }
        key->publicKey.x = publicKey->x;
        key->publicKey.y = publicKey->y;
        key->publicKey.z = NULL;
        sfree(publicKey);
    } else {
        key->privateKey = bignum_random_in_range(One, key->publicKey.curve->w.n);
        if (!key->privateKey) {
            sfree(key);
            return NULL;
        }
        publicKey = ecp_mul(&key->publicKey.curve->w.G, key->privateKey);
        if (!publicKey) {
            freebn(key->privateKey);
            sfree(key);
            return NULL;
        }
        key->publicKey.x = publicKey->x;
        key->publicKey.y = publicKey->y;
        key->publicKey.z = NULL;
        sfree(publicKey);
    }
    return key;
}

void ssh_ecdhkex_getpublic(struct ec_key *ec, BinarySink *bs)
{
    int i;
    int pointlen;

    pointlen = (bignum_bitcount(ec->publicKey.curve->p) + 7) / 8;

    if (ec->publicKey.curve->type == EC_WEIERSTRASS) {
        put_byte(bs, 0x04);
        for (i = pointlen; i--;)
            put_byte(bs, bignum_byte(ec->publicKey.x, i));
        for (i = pointlen; i--;)
            put_byte(bs, bignum_byte(ec->publicKey.y, i));
    } else {
        for (i = 0; i < pointlen; ++i)
            put_byte(bs, bignum_byte(ec->publicKey.x, i));
    }
}

Bignum ssh_ecdhkex_getkey(struct ec_key *ec,
                          const void *remoteKey, int remoteKeyLen)
{
    struct ec_point remote;
    Bignum ret;

    if (ec->publicKey.curve->type == EC_WEIERSTRASS) {
        remote.curve = ec->publicKey.curve;
        remote.infinity = false;
        if (!decodepoint(remoteKey, remoteKeyLen, &remote)) {
            return NULL;
        }
    } else {
        /* Point length has to be the same length */
        if (remoteKeyLen != (bignum_bitcount(ec->publicKey.curve->p) + 7) / 8) {
            return NULL;
        }

        remote.curve = ec->publicKey.curve;
        remote.infinity = false;
        remote.x = bignum_from_bytes_le((const unsigned char *)remoteKey,
                                        remoteKeyLen);
        remote.y = NULL;
        remote.z = NULL;
    }

    ret = ecdh_calculate(ec->privateKey, &remote);
    if (remote.x) freebn(remote.x);
    if (remote.y) freebn(remote.y);
    return ret;
}

void ssh_ecdhkex_freekey(struct ec_key *key)
{
    ecdsa_freekey(&key->sshk);
}

static const struct eckex_extra kex_extra_curve25519 = { ec_curve25519 };
static const struct ssh_kex ssh_ec_kex_curve25519 = {
    "curve25519-sha256@libssh.org", NULL, KEXTYPE_ECDH,
    &ssh_sha256, &kex_extra_curve25519,
};

const struct eckex_extra kex_extra_nistp256 = { ec_p256 };
static const struct ssh_kex ssh_ec_kex_nistp256 = {
    "ecdh-sha2-nistp256", NULL, KEXTYPE_ECDH,
    &ssh_sha256, &kex_extra_nistp256,
};

const struct eckex_extra kex_extra_nistp384 = { ec_p384 };
static const struct ssh_kex ssh_ec_kex_nistp384 = {
    "ecdh-sha2-nistp384", NULL, KEXTYPE_ECDH,
    &ssh_sha384, &kex_extra_nistp384,
};

const struct eckex_extra kex_extra_nistp521 = { ec_p521 };
static const struct ssh_kex ssh_ec_kex_nistp521 = {
    "ecdh-sha2-nistp521", NULL, KEXTYPE_ECDH,
    &ssh_sha512, &kex_extra_nistp521,
};

static const struct ssh_kex *const ec_kex_list[] = {
    &ssh_ec_kex_curve25519,
    &ssh_ec_kex_nistp256,
    &ssh_ec_kex_nistp384,
    &ssh_ec_kex_nistp521,
};

const struct ssh_kexes ssh_ecdh_kex = {
    sizeof(ec_kex_list) / sizeof(*ec_kex_list),
    ec_kex_list
};

/* ----------------------------------------------------------------------
 * Helper functions for finding key algorithms and returning auxiliary
 * data.
 */

const ssh_keyalg *ec_alg_by_oid(int len, const void *oid,
                                        const struct ec_curve **curve)
{
    static const ssh_keyalg *algs_with_oid[] = {
        &ssh_ecdsa_nistp256,
        &ssh_ecdsa_nistp384,
        &ssh_ecdsa_nistp521,
    };
    int i;

    for (i = 0; i < lenof(algs_with_oid); i++) {
        const ssh_keyalg *alg = algs_with_oid[i];
        const struct ecsign_extra *extra =
            (const struct ecsign_extra *)alg->extra;
        if (len == extra->oidlen && !memcmp(oid, extra->oid, len)) {
            *curve = extra->curve();
            return alg;
        }
    }
    return NULL;
}

const unsigned char *ec_alg_oid(const ssh_keyalg *alg,
                                int *oidlen)
{
    const struct ecsign_extra *extra = (const struct ecsign_extra *)alg->extra;
    *oidlen = extra->oidlen;
    return extra->oid;
}

const int ec_nist_curve_lengths[] = { 256, 384, 521 };
const int n_ec_nist_curve_lengths = lenof(ec_nist_curve_lengths);

bool ec_nist_alg_and_curve_by_bits(
    int bits, const struct ec_curve **curve, const ssh_keyalg **alg)
{
    switch (bits) {
      case 256: *alg = &ssh_ecdsa_nistp256; break;
      case 384: *alg = &ssh_ecdsa_nistp384; break;
      case 521: *alg = &ssh_ecdsa_nistp521; break;
      default: return false;
    }
    *curve = ((struct ecsign_extra *)(*alg)->extra)->curve();
    return true;
}

bool ec_ed_alg_and_curve_by_bits(
    int bits, const struct ec_curve **curve, const ssh_keyalg **alg)
{
    switch (bits) {
      case 256: *alg = &ssh_ecdsa_ed25519; break;
      default: return false;
    }
    *curve = ((struct ecsign_extra *)(*alg)->extra)->curve();
    return true;
}

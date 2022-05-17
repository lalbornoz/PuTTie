BEGIN_ENUM_TYPE(hashalg)
    ENUM_VALUE("md5", &ssh_md5)
    ENUM_VALUE("sha1", &ssh_sha1)
    ENUM_VALUE("sha1_sw", &ssh_sha1_sw)
    ENUM_VALUE("sha256", &ssh_sha256)
    ENUM_VALUE("sha384", &ssh_sha384)
    ENUM_VALUE("sha512", &ssh_sha512)
    ENUM_VALUE("sha256_sw", &ssh_sha256_sw)
    ENUM_VALUE("sha384_sw", &ssh_sha384_sw)
    ENUM_VALUE("sha512_sw", &ssh_sha512_sw)
#if HAVE_SHA_NI
    ENUM_VALUE("sha1_ni", &ssh_sha1_ni)
    ENUM_VALUE("sha256_ni", &ssh_sha256_ni)
#endif
#if HAVE_NEON_CRYPTO
    ENUM_VALUE("sha1_neon", &ssh_sha1_neon)
    ENUM_VALUE("sha256_neon", &ssh_sha256_neon)
#endif
#if HAVE_NEON_SHA512
    ENUM_VALUE("sha384_neon", &ssh_sha384_neon)
    ENUM_VALUE("sha512_neon", &ssh_sha512_neon)
#endif
    ENUM_VALUE("sha3_224", &ssh_sha3_224)
    ENUM_VALUE("sha3_256", &ssh_sha3_256)
    ENUM_VALUE("sha3_384", &ssh_sha3_384)
    ENUM_VALUE("sha3_512", &ssh_sha3_512)
    ENUM_VALUE("shake256_114bytes", &ssh_shake256_114bytes)
    ENUM_VALUE("blake2b", &ssh_blake2b)
END_ENUM_TYPE(hashalg)

BEGIN_ENUM_TYPE(macalg)
    ENUM_VALUE("hmac_md5", &ssh_hmac_md5)
    ENUM_VALUE("hmac_sha1", &ssh_hmac_sha1)
    ENUM_VALUE("hmac_sha1_buggy", &ssh_hmac_sha1_buggy)
    ENUM_VALUE("hmac_sha1_96", &ssh_hmac_sha1_96)
    ENUM_VALUE("hmac_sha1_96_buggy", &ssh_hmac_sha1_96_buggy)
    ENUM_VALUE("hmac_sha256", &ssh_hmac_sha256)
    ENUM_VALUE("poly1305", &ssh2_poly1305)
END_ENUM_TYPE(macalg)

BEGIN_ENUM_TYPE(keyalg)
    ENUM_VALUE("dsa", &ssh_dsa)
    ENUM_VALUE("rsa", &ssh_rsa)
    ENUM_VALUE("ed25519", &ssh_ecdsa_ed25519)
    ENUM_VALUE("ed448", &ssh_ecdsa_ed448)
    ENUM_VALUE("p256", &ssh_ecdsa_nistp256)
    ENUM_VALUE("p384", &ssh_ecdsa_nistp384)
    ENUM_VALUE("p521", &ssh_ecdsa_nistp521)
    ENUM_VALUE("dsa-cert", &opensshcert_ssh_dsa)
    ENUM_VALUE("rsa-cert", &opensshcert_ssh_rsa)
    ENUM_VALUE("ed25519-cert", &opensshcert_ssh_ecdsa_ed25519)
    ENUM_VALUE("p256-cert", &opensshcert_ssh_ecdsa_nistp256)
    ENUM_VALUE("p384-cert", &opensshcert_ssh_ecdsa_nistp384)
    ENUM_VALUE("p521-cert", &opensshcert_ssh_ecdsa_nistp521)
END_ENUM_TYPE(keyalg)

BEGIN_ENUM_TYPE(cipheralg)
    ENUM_VALUE("3des_ctr", &ssh_3des_ssh2_ctr)
    ENUM_VALUE("3des_ssh2", &ssh_3des_ssh2)
    ENUM_VALUE("3des_ssh1", &ssh_3des_ssh1)
    ENUM_VALUE("des_cbc", &ssh_des)
    ENUM_VALUE("aes256_ctr", &ssh_aes256_sdctr)
    ENUM_VALUE("aes256_cbc", &ssh_aes256_cbc)
    ENUM_VALUE("aes192_ctr", &ssh_aes192_sdctr)
    ENUM_VALUE("aes192_cbc", &ssh_aes192_cbc)
    ENUM_VALUE("aes128_ctr", &ssh_aes128_sdctr)
    ENUM_VALUE("aes128_cbc", &ssh_aes128_cbc)
    ENUM_VALUE("aes256_ctr_sw", &ssh_aes256_sdctr_sw)
    ENUM_VALUE("aes256_cbc_sw", &ssh_aes256_cbc_sw)
    ENUM_VALUE("aes192_ctr_sw", &ssh_aes192_sdctr_sw)
    ENUM_VALUE("aes192_cbc_sw", &ssh_aes192_cbc_sw)
    ENUM_VALUE("aes128_ctr_sw", &ssh_aes128_sdctr_sw)
    ENUM_VALUE("aes128_cbc_sw", &ssh_aes128_cbc_sw)
#if HAVE_AES_NI
    ENUM_VALUE("aes256_ctr_ni", &ssh_aes256_sdctr_ni)
    ENUM_VALUE("aes256_cbc_ni", &ssh_aes256_cbc_ni)
    ENUM_VALUE("aes192_ctr_ni", &ssh_aes192_sdctr_ni)
    ENUM_VALUE("aes192_cbc_ni", &ssh_aes192_cbc_ni)
    ENUM_VALUE("aes128_ctr_ni", &ssh_aes128_sdctr_ni)
    ENUM_VALUE("aes128_cbc_ni", &ssh_aes128_cbc_ni)
#endif
#if HAVE_NEON_CRYPTO
    ENUM_VALUE("aes256_ctr_neon", &ssh_aes256_sdctr_neon)
    ENUM_VALUE("aes256_cbc_neon", &ssh_aes256_cbc_neon)
    ENUM_VALUE("aes192_ctr_neon", &ssh_aes192_sdctr_neon)
    ENUM_VALUE("aes192_cbc_neon", &ssh_aes192_cbc_neon)
    ENUM_VALUE("aes128_ctr_neon", &ssh_aes128_sdctr_neon)
    ENUM_VALUE("aes128_cbc_neon", &ssh_aes128_cbc_neon)
#endif
    ENUM_VALUE("blowfish_ctr", &ssh_blowfish_ssh2_ctr)
    ENUM_VALUE("blowfish_ssh2", &ssh_blowfish_ssh2)
    ENUM_VALUE("blowfish_ssh1", &ssh_blowfish_ssh1)
    ENUM_VALUE("arcfour256", &ssh_arcfour256_ssh2)
    ENUM_VALUE("arcfour128", &ssh_arcfour128_ssh2)
    ENUM_VALUE("chacha20_poly1305", &ssh2_chacha20_poly1305)
END_ENUM_TYPE(cipheralg)

BEGIN_ENUM_TYPE(dh_group)
    ENUM_VALUE("group1", &ssh_diffiehellman_group1_sha1)
    ENUM_VALUE("group14", &ssh_diffiehellman_group14_sha256)
END_ENUM_TYPE(dh_group)

BEGIN_ENUM_TYPE(ecdh_alg)
    ENUM_VALUE("curve25519", &ssh_ec_kex_curve25519)
    ENUM_VALUE("curve448", &ssh_ec_kex_curve448)
    ENUM_VALUE("nistp256", &ssh_ec_kex_nistp256)
    ENUM_VALUE("nistp384", &ssh_ec_kex_nistp384)
    ENUM_VALUE("nistp521", &ssh_ec_kex_nistp521)
END_ENUM_TYPE(ecdh_alg)

BEGIN_ENUM_TYPE(rsaorder)
    ENUM_VALUE("exponent_first", RSA_SSH1_EXPONENT_FIRST)
    ENUM_VALUE("modulus_first", RSA_SSH1_MODULUS_FIRST)
END_ENUM_TYPE(rsaorder)

BEGIN_ENUM_TYPE(primegenpolicy)
    ENUM_VALUE("probabilistic", &primegen_probabilistic)
    ENUM_VALUE("provable_fast", &primegen_provable_fast)
    ENUM_VALUE("provable_maurer_simple", &primegen_provable_maurer_simple)
    ENUM_VALUE("provable_maurer_complex", &primegen_provable_maurer_complex)
END_ENUM_TYPE(primegenpolicy)

BEGIN_ENUM_TYPE(argon2flavour)
    ENUM_VALUE("d", Argon2d)
    ENUM_VALUE("i", Argon2i)
    ENUM_VALUE("id", Argon2id)
    /* I expect to forget which spelling I chose, so let's support many */
    ENUM_VALUE("argon2d", Argon2d)
    ENUM_VALUE("argon2i", Argon2i)
    ENUM_VALUE("argon2id", Argon2id)
    ENUM_VALUE("Argon2d", Argon2d)
    ENUM_VALUE("Argon2i", Argon2i)
    ENUM_VALUE("Argon2id", Argon2id)
END_ENUM_TYPE(argon2flavour)

BEGIN_ENUM_TYPE(fptype)
    ENUM_VALUE("md5", SSH_FPTYPE_MD5)
    ENUM_VALUE("sha256", SSH_FPTYPE_SHA256)
END_ENUM_TYPE(fptype)

/*
 * cproxy.h already has a list macro mapping protocol-specified
 * strings to the list of HTTP Digest hash functions. Rather than
 * invent a separate one for testcrypt, reuse the existing names.
 */
BEGIN_ENUM_TYPE(httpdigesthash)
    #define DECL_ARRAY(id, str, alg, bits, accepted) ENUM_VALUE(str, id)
    HTTP_DIGEST_HASHES(DECL_ARRAY)
    #undef DECL_ARRAY
END_ENUM_TYPE(httpdigesthash)

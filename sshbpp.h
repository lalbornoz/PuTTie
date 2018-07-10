/*
 * Abstraction of the binary packet protocols used in SSH.
 */

#ifndef PUTTY_SSHBPP_H
#define PUTTY_SSHBPP_H

typedef struct BinaryPacketProtocol BinaryPacketProtocol;

struct BinaryPacketProtocolVtable {
    void (*free)(BinaryPacketProtocol *); 
    void (*handle_input)(BinaryPacketProtocol *);
    PktOut *(*new_pktout)(int type);
    void (*format_packet)(BinaryPacketProtocol *, PktOut *);
};

struct BinaryPacketProtocol {
    const struct BinaryPacketProtocolVtable *vt;
    bufchain *in_raw, *out_raw;
    PacketQueue *in_pq;
    PacketLogSettings *pls;
    void *logctx;

    int seen_disconnect;
    char *error;
};

#define ssh_bpp_free(bpp) ((bpp)->vt->free(bpp))
#define ssh_bpp_handle_input(bpp) ((bpp)->vt->handle_input(bpp))
#define ssh_bpp_new_pktout(bpp, type) ((bpp)->vt->new_pktout(type))
#define ssh_bpp_format_packet(bpp, pkt) ((bpp)->vt->format_packet(bpp, pkt))

BinaryPacketProtocol *ssh1_bpp_new(void);
void ssh1_bpp_new_cipher(BinaryPacketProtocol *bpp,
                         const struct ssh_cipher *cipher,
                         const void *session_key);
void ssh1_bpp_start_compression(BinaryPacketProtocol *bpp);

BinaryPacketProtocol *ssh2_bpp_new(void);
void ssh2_bpp_new_outgoing_crypto(
    BinaryPacketProtocol *bpp,
    const struct ssh2_cipher *cipher, const void *ckey, const void *iv,
    const struct ssh_mac *mac, int etm_mode, const void *mac_key,
    const struct ssh_compress *compression);
void ssh2_bpp_new_incoming_crypto(
    BinaryPacketProtocol *bpp,
    const struct ssh2_cipher *cipher, const void *ckey, const void *iv,
    const struct ssh_mac *mac, int etm_mode, const void *mac_key,
    const struct ssh_compress *compression);

BinaryPacketProtocol *ssh2_bare_bpp_new(void);

#endif /* PUTTY_SSHBPP_H */

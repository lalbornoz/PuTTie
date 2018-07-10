/*
 * Trivial binary packet protocol for the 'bare' ssh-connection
 * protocol used in PuTTY's SSH-2 connection sharing system.
 */

#include <assert.h>

#include "putty.h"
#include "ssh.h"
#include "sshbpp.h"
#include "sshcr.h"

struct ssh2_bare_bpp_state {
    int crState;
    long packetlen, maxlen;
    unsigned char *data;
    unsigned long incoming_sequence, outgoing_sequence;
    PktIn *pktin;

    BinaryPacketProtocol bpp;
};

static void ssh2_bare_bpp_free(BinaryPacketProtocol *bpp);
static void ssh2_bare_bpp_handle_input(BinaryPacketProtocol *bpp);
static PktOut *ssh2_bare_bpp_new_pktout(int type);
static void ssh2_bare_bpp_format_packet(BinaryPacketProtocol *bpp, PktOut *);

const struct BinaryPacketProtocolVtable ssh2_bare_bpp_vtable = {
    ssh2_bare_bpp_free,
    ssh2_bare_bpp_handle_input,
    ssh2_bare_bpp_new_pktout,
    ssh2_bare_bpp_format_packet,
};

BinaryPacketProtocol *ssh2_bare_bpp_new(void)
{
    struct ssh2_bare_bpp_state *s = snew(struct ssh2_bare_bpp_state);
    memset(s, 0, sizeof(*s));
    s->bpp.vt = &ssh2_bare_bpp_vtable;
    return &s->bpp;
}

static void ssh2_bare_bpp_free(BinaryPacketProtocol *bpp)
{
    struct ssh2_bare_bpp_state *s =
        FROMFIELD(bpp, struct ssh2_bare_bpp_state, bpp);
    if (s->pktin)
        ssh_unref_packet(s->pktin);
    sfree(s);
}

static void ssh2_bare_bpp_handle_input(BinaryPacketProtocol *bpp)
{
    struct ssh2_bare_bpp_state *s =
        FROMFIELD(bpp, struct ssh2_bare_bpp_state, bpp);

    crBegin(s->crState);

    while (1) {
        /* Read the length field. */
        {
            unsigned char lenbuf[4];
            crMaybeWaitUntilV(bufchain_try_fetch_consume(
                                  s->bpp.in_raw, lenbuf, 4));
            s->packetlen = toint(GET_32BIT_MSB_FIRST(lenbuf));
        }

        if (s->packetlen <= 0 || s->packetlen >= (long)OUR_V2_PACKETLIMIT) {
            s->bpp.error = dupstr("Invalid packet length received");
            crStopV;
        }

        /*
         * Allocate the packet to return, now we know its length.
         */
        s->pktin = snew_plus(PktIn, s->packetlen);
        s->pktin->qnode.prev = s->pktin->qnode.next = NULL;
        s->maxlen = 0;
        s->pktin->refcount = 1;
        s->data = snew_plus_get_aux(s->pktin);

        s->pktin->encrypted_len = s->packetlen;

        s->pktin->sequence = s->incoming_sequence++;

        /*
         * Read the remainder of the packet.
         */
        crMaybeWaitUntilV(bufchain_try_fetch_consume(
                              s->bpp.in_raw, s->data, s->packetlen));

        /*
         * The data we just read is precisely the initial type byte
         * followed by the packet payload.
         */
        s->pktin->type = s->data[0];
        s->data++;
        s->packetlen--;
        BinarySource_INIT(s->pktin, s->data, s->packetlen);

        /*
         * Log incoming packet, possibly omitting sensitive fields.
         */
        if (s->bpp.logctx) {
            logblank_t blanks[MAX_BLANKS];
            int nblanks = ssh2_censor_packet(
                s->bpp.pls, s->pktin->type, FALSE,
                make_ptrlen(s->data, s->packetlen), blanks);
            log_packet(s->bpp.logctx, PKT_INCOMING, s->pktin->type,
                       ssh2_pkt_type(s->bpp.pls->kctx, s->bpp.pls->actx,
                                     s->pktin->type),
                       get_ptr(s->pktin), get_avail(s->pktin), nblanks, blanks,
                       &s->pktin->sequence, 0, NULL);
        }

        pq_push(s->bpp.in_pq, s->pktin);

        {
            int type = s->pktin->type;
            s->pktin = NULL;

            if (type == SSH2_MSG_DISCONNECT)
                s->bpp.seen_disconnect = TRUE;
        }
    }
    crFinishV;
}

static PktOut *ssh2_bare_bpp_new_pktout(int pkt_type)
{
    PktOut *pkt = ssh_new_packet();
    pkt->length = 4; /* space for packet length */
    pkt->type = pkt_type;
    put_byte(pkt, pkt_type);
    return pkt;
}

static void ssh2_bare_bpp_format_packet(BinaryPacketProtocol *bpp, PktOut *pkt)
{
    struct ssh2_bare_bpp_state *s =
        FROMFIELD(bpp, struct ssh2_bare_bpp_state, bpp);

    if (s->bpp.logctx) {
        ptrlen pktdata = make_ptrlen(pkt->data + 5, pkt->length - 5);
        logblank_t blanks[MAX_BLANKS];
        int nblanks = ssh2_censor_packet(
            s->bpp.pls, pkt->type, TRUE, pktdata, blanks);
        log_packet(s->bpp.logctx, PKT_OUTGOING, pkt->type,
                   ssh2_pkt_type(s->bpp.pls->kctx, s->bpp.pls->actx,
                                 pkt->type),
                   pktdata.ptr, pktdata.len, nblanks, blanks,
                   &s->outgoing_sequence,
                   pkt->downstream_id, pkt->additional_log_text);
    }

    s->outgoing_sequence++;        /* only for diagnostics, really */

    PUT_32BIT(pkt->data, pkt->length - 4);
    bufchain_add(s->bpp.out_raw, pkt->data, pkt->length);

    ssh_free_pktout(pkt);
}

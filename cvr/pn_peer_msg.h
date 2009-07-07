/**
 * Copyright (C) 2008-2009 Felipe Contreras
 * Copyright (C) 1998-2006 Pidgin (see pidgin-copyright)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PN_PEER_MSG_H
#define PN_PEER_MSG_H

typedef struct PnPeerMsg PnPeerMsg;

struct PnPeerCall;
struct PnPeerLink;
struct MsnSession;
struct MsnMessage;
struct MsnSwitchBoard;

#include "io/pn_buffer.h"

#include <glib/gstdio.h>

/**
 * A SLP Message  This contains everything that we will need to send a SLP
 * Message even if has to be sent in several parts.
 */
struct PnPeerMsg
{
    struct PnPeerCall *call; /**< The call to which this slp message belongs (if applicable). */
    struct PnPeerLink *link; /**< The peer link through which this slp message is being sent. */
    struct MsnSession *session;

    long session_id;
    long id;
    long ack_id;
    long ack_sub_id;
    guint64 ack_size;
    long app_id;

    gboolean sip; /**< A flag that states if this is a SIP slp message. */
    long flags;

    FILE *fp;
    gchar *buffer;
    guint64 offset;
    guint64 size;

    GList *msgs; /**< The real messages. */

    struct MsnMessage *msg; /**< The temporary real message that will be sent. */

#ifdef PECAN_DEBUG_SLP
    const gchar *info;
    gboolean text_body;
#endif

    unsigned int ref_count;
    struct MsnSwitchBoard *swboard;
};

PnPeerMsg *pn_peer_msg_new(struct PnPeerLink *link);
void pn_peer_msg_free(PnPeerMsg *peer_msg);
PnPeerMsg *pn_peer_msg_ref(PnPeerMsg *peer_msg);
PnPeerMsg *pn_peer_msg_unref(PnPeerMsg *peer_msg);

void pn_peer_msg_set_body(PnPeerMsg *peer_msg,
                          gconstpointer *body,
                          guint64 size);
void pn_peer_msg_set_image(PnPeerMsg *peer_msg,
                           PnBuffer *image);
void pn_peer_msg_open_file(PnPeerMsg *peer_msg,
                           const char *file_name);
PnPeerMsg *pn_peer_msg_sip_new(struct PnPeerCall *call,
                               int cseq,
                               const char *header,
                               const char *branch,
                               const char *content_type,
                               const char *content);

#ifdef PECAN_DEBUG_SLP
void pn_peer_msg_show(struct MsnMessage *msg);
#endif

#endif /* PN_PEER_MSG_H */

/**
 * Copyright (C) 2007-2008 Felipe Contreras
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include "transaction_private.h"
#include "msn_log.h"

#include <string.h>

#ifdef HAVE_LIBPURPLE
/* libpurple stuff. */
#include <eventloop.h>
#endif /* HAVE_LIBPURPLE */

MsnTransaction *
msn_transaction_new(MsnCmdProc *cmdproc, const char *command,
					const char *format, ...)
{
	MsnTransaction *trans;
	va_list arg;

	g_return_val_if_fail(command != NULL, NULL);

	trans = g_new0(MsnTransaction, 1);

	trans->cmdproc = cmdproc;
	trans->command = g_strdup(command);

	if (format != NULL)
	{
		va_start(arg, format);
		trans->params = g_strdup_vprintf(format, arg);
		va_end(arg);
	}

	/* trans->queue = g_queue_new(); */

	return trans;
}

void
msn_transaction_destroy(MsnTransaction *trans)
{
	g_return_if_fail(trans != NULL);

	g_free(trans->command);
	g_free(trans->params);
	g_free(trans->payload);

#if 0
	if (trans->pendent_cmd != NULL)
		msn_message_unref(trans->pendent_msg);
#endif

#if 0
	MsnTransaction *elem;
	if (trans->queue != NULL)
	{
		while ((elem = g_queue_pop_head(trans->queue)) != NULL)
			msn_transaction_destroy(elem);

		g_queue_free(trans->queue);
	}
#endif

	if (trans->callbacks != NULL && trans->has_custom_callbacks)
		g_hash_table_destroy(trans->callbacks);

#ifdef HAVE_LIBPURPLE
	if (trans->timer)
		purple_timeout_remove(trans->timer);
#endif /* HAVE_LIBPURPLE */

	g_free(trans);
}

char *
msn_transaction_to_string(MsnTransaction *trans)
{
	char *str;

	g_return_val_if_fail(trans != NULL, FALSE);

	if (trans->params != NULL)
		str = g_strdup_printf("%s %u %s\r\n", trans->command, trans->trId, trans->params);
	else
		str = g_strdup_printf("%s %u\r\n", trans->command, trans->trId);

	return str;
}

void
msn_transaction_queue_cmd(MsnTransaction *trans, MsnCommand *cmd)
{
	msn_log ("cmd=%p", cmd);
	trans->pendent_cmd = cmd;
	msn_command_ref(cmd);
}

void
msn_transaction_unqueue_cmd(MsnTransaction *trans, MsnCmdProc *cmdproc)
{
	MsnCommand *cmd;

	cmd = trans->pendent_cmd;
	msn_log ("cmd=%p", cmd);

	g_return_if_fail(cmd != NULL);

	msn_cmdproc_process_cmd(cmdproc, cmd);
	msn_command_unref(cmd);

	trans->pendent_cmd = NULL;
}

#if 0
void
msn_transaction_queue(MsnTransaction *trans, MsnTransaction *elem)
{
	if (trans->queue == NULL)
		trans->queue = g_queue_new();

	g_queue_push_tail(trans->queue, elem);
}

void
msn_transaction_unqueue(MsnTransaction *trans, MsnCmdProc *cmdproc)
{
	MsnTransaction *elem;

	while ((elem = g_queue_pop_head(trans->queue)) != NULL)
		msn_cmdproc_send_trans(cmdproc, elem);
}
#endif

void
msn_transaction_set_payload(MsnTransaction *trans,
							const gchar *payload,
							gsize payload_len)
{
	g_return_if_fail(trans   != NULL);
	g_return_if_fail(payload != NULL);

	trans->payload = g_strdup(payload);
	trans->payload_len = payload_len ? payload_len : strlen(trans->payload);
}

void
msn_transaction_set_data(MsnTransaction *trans, void *data)
{
	g_return_if_fail(trans != NULL);

	trans->data = data;
}

void
msn_transaction_add_cb(MsnTransaction *trans,
					   const gchar *answer,
					   MsnTransCb cb)
{
	g_return_if_fail(trans  != NULL);
	g_return_if_fail(answer != NULL);

	if (trans->callbacks == NULL)
	{
		trans->has_custom_callbacks = TRUE;
		trans->callbacks = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
												 NULL);
	}
	else if (trans->has_custom_callbacks != TRUE)
	{
		g_return_if_reached ();
	}

	g_hash_table_insert(trans->callbacks, g_strdup (answer), cb);
}

static gboolean
transaction_timeout(gpointer data)
{
	MsnTransaction *trans;

	trans = data;
	g_return_val_if_fail(trans != NULL, FALSE);

        msn_log ("cmd=[%s],trid=[%d],params=[%s]",
                 trans->command, trans->trId, trans->params);

	if (trans->timeout_cb != NULL)
		trans->timeout_cb(trans->cmdproc, trans);

	return FALSE;
}

void
msn_transaction_set_timeout_cb(MsnTransaction *trans, MsnTimeoutCb cb)
{
#ifdef HAVE_LIBPURPLE
	if (trans->timer)
	{
		msn_error ("this shouldn't be happening");
		purple_timeout_remove(trans->timer);
	}
	trans->timeout_cb = cb;
	trans->timer = purple_timeout_add(60000, transaction_timeout, trans);
#endif /* HAVE_LIBPURPLE */
}

void
msn_transaction_set_error_cb(MsnTransaction *trans, MsnErrorCb cb)
{
	trans->error_cb = cb;
}
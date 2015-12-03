/**
 * \file messages.c
 * \author Radek Krejci <rkrejci@cesnet.cz>
 * \brief libnetconf2 - NETCONF messages functions
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <libyang/libyang.h>

#include "libnetconf.h"
#include "messages_p.h"

const char *rpcedit_dfltop2str[] = {NULL, "merge", "replace", "none"};
const char *rpcedit_testopt2str[] = {NULL, "test-then-set", "set", "test-only"};
const char *rpcedit_erropt2str[] = {NULL, "stop-on-error", "continue-on-error", "rollback-on-error"};

API struct nc_rpc *
nc_rpc_generic(const struct lyd_node *data, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_generic *rpc;

    if (data->next || (data->prev != data)) {
        ERR("Generic RPC must have a single root node.");
        return NULL;
    }

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_GENERIC;
    if (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE) {
        rpc->data = lyd_dup(data, 1);
    } else {
        rpc->data = (struct lyd_node *)data;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_generic_xml(const char *xml_str, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_generic_xml *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_GENERIC_XML;
    if (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE) {
        rpc->xml_str = strdup(xml_str);
    } else {
        rpc->xml_str = (char *)xml_str;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_getconfig(NC_DATASTORE source, const char *filter, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_getconfig *rpc;

    if ((filter[0] != '<') && (filter[0] != '/') && !isalpha(filter[0])) {
        ERR("Filter must either be an XML subtree or an XPath expression.");
        return NULL;
    }

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_GETCONFIG;
    rpc->source = source;
    if (filter && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->filter = strdup(filter);
    } else {
        rpc->filter = (char *)filter;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_edit(NC_DATASTORE target, NC_RPC_EDIT_DFLTOP default_op, NC_RPC_EDIT_TESTOPT test_opt,
            NC_RPC_EDIT_ERROPT error_opt, const char *edit_content, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_edit *rpc;

    if ((edit_content[0] != '<') && !isalpha(edit_content[0])) {
        ERR("<edit-config> content must either be a URL or a config (XML).");
        return NULL;
    }

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_EDIT;
    rpc->target = target;
    rpc->default_op = default_op;
    rpc->test_opt = test_opt;
    rpc->error_opt = error_opt;
    if (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE) {
        rpc->edit_cont = strdup(edit_content);
    } else {
        rpc->edit_cont = (char *)edit_content;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_copy(NC_DATASTORE target, const char *url_trg, NC_DATASTORE source, const char *url_or_config_src,
            NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_copy *rpc;

    if (url_or_config_src && (url_or_config_src[0] != '<') && !isalpha(url_or_config_src[0])) {
        ERR("<copy-config> source is neither a URL nor a config (XML).");
        return NULL;
    }

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_COPY;
    rpc->target = target;
    if (url_trg && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->url_trg = strdup(url_trg);
    } else {
        rpc->url_trg = (char *)url_trg;
    }
    rpc->source = source;
    if (url_or_config_src && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->url_config_src = strdup(url_or_config_src);
    } else {
        rpc->url_config_src = (char *)url_or_config_src;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_delete(NC_DATASTORE target, const char *url, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_delete *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_DELETE;
    rpc->target = target;
    if (url && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->url = strdup(url);
    } else {
        rpc->url = (char *)url;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_lock(NC_DATASTORE target)
{
    struct nc_rpc_lock *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_LOCK;
    rpc->target = target;

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_unlock(NC_DATASTORE target)
{
    struct nc_rpc_lock *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_UNLOCK;
    rpc->target = target;

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_get(const char *filter, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_get *rpc;

    if ((filter[0] != '<') && (filter[0] != '/') && !isalpha(filter[0])) {
        ERR("Filter must either be an XML subtree or an XPath expression.");
        return NULL;
    }

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_GET;
    if (filter && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->filter = strdup(filter);
    } else {
        rpc->filter = (char *)filter;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_kill(uint32_t session_id)
{
    struct nc_rpc_kill *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_KILL;
    rpc->sid = session_id;

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_commit(int confirmed, uint32_t confirm_timeout, const char *persist, const char *persist_id,
              NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_commit *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_COMMIT;
    rpc->confirmed = confirmed;
    rpc->confirm_timeout = confirm_timeout;
    if (persist && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->persist = strdup(persist);
    } else {
        rpc->persist = (char *)persist;
    }
    if (persist_id && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->persist_id = strdup(persist_id);
    } else {
        rpc->persist_id = (char *)persist_id;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_discard(void)
{
    struct nc_rpc *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_DISCARD;

    return rpc;
}

API struct nc_rpc *
nc_rpc_cancel(const char *persist_id, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_cancel *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_CANCEL;
    if (persist_id && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->persist_id = strdup(persist_id);
    } else {
        rpc->persist_id = (char *)persist_id;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_validate(NC_DATASTORE source, const char *url_or_config, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_validate *rpc;

    if (url_or_config && (url_or_config[0] != '<') && !isalpha(url_or_config[0])) {
        ERR("<validate> source is neither a URL nor a config (XML).");
        return NULL;
    }

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_VALIDATE;
    rpc->source = source;
    if (url_or_config && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->url_config_src = strdup(url_or_config);
    } else {
        rpc->url_config_src = (char *)url_or_config;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_getschema(const char *identifier, const char *version, const char *format, NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_getschema *rpc;

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_GETSCHEMA;
    if (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE) {
        rpc->identifier = strdup(identifier);
    } else {
        rpc->identifier = (char *)identifier;
    }
    if (version && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->version = strdup(version);
    } else {
        rpc->version = (char *)version;
    }
    if (format && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->format = strdup(format);
    } else {
        rpc->format = (char *)format;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

API struct nc_rpc *
nc_rpc_subscribe(const char *stream_name, const char *filter, const char *start_time, const char *stop_time,
                 NC_RPC_PARAMTYPE paramtype)
{
    struct nc_rpc_subscribe *rpc;

    if ((filter[0] != '<') && (filter[0] != '/') && !isalpha(filter[0])) {
        ERR("Filter must either be an XML subtree or an XPath expression.");
        return NULL;
    }

    rpc = malloc(sizeof *rpc);
    if (!rpc) {
        ERRMEM;
        return NULL;
    }

    rpc->type = NC_RPC_SUBSCRIBE;
    if (stream_name && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->stream = strdup(stream_name);
    } else {
        rpc->stream = (char *)stream_name;
    }
    if (filter && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->filter = strdup(filter);
    } else {
        rpc->filter = (char *)filter;
    }
    if (start_time && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->start = strdup(start_time);
    } else {
        rpc->start = (char *)start_time;
    }
    if (stop_time && (paramtype == NC_RPC_PARAMTYPE_DUP_AND_FREE)) {
        rpc->stop = strdup(stop_time);
    } else {
        rpc->stop = (char *)stop_time;
    }
    rpc->free = (paramtype == NC_RPC_PARAMTYPE_CONST ? 0 : 1);

    return (struct nc_rpc *)rpc;
}

static void
nc_server_rpc_free(struct nc_server_rpc *rpc)
{
    lyxml_free(rpc->tree->schema->module->ctx, rpc->root);
    lyd_free(rpc->tree);
}

API void
nc_rpc_free(struct nc_rpc *rpc)
{
    struct nc_rpc_generic *rpc_generic;
    struct nc_rpc_generic_xml *rpc_generic_xml;
    struct nc_rpc_getconfig *rpc_getconfig;
    struct nc_rpc_edit *rpc_edit;
    struct nc_rpc_copy *rpc_copy;
    struct nc_rpc_delete *rpc_delete;
    struct nc_rpc_get *rpc_get;
    struct nc_rpc_commit *rpc_commit;
    struct nc_rpc_cancel *rpc_cancel;
    struct nc_rpc_validate *rpc_validate;
    struct nc_rpc_getschema *rpc_getschema;
    struct nc_rpc_subscribe *rpc_subscribe;

    if (!rpc) {
        return;
    }

    switch (rpc->type) {
    case NC_RPC_GENERIC:
        rpc_generic = (struct nc_rpc_generic *)rpc;
        if (rpc_generic->free) {
            lyd_free(rpc_generic->data);
        }
        break;
    case NC_RPC_GENERIC_XML:
        rpc_generic_xml = (struct nc_rpc_generic_xml *)rpc;
        if (rpc_generic_xml->free) {
            free(rpc_generic_xml->xml_str);
        }
        break;
    case NC_RPC_GETCONFIG:
        rpc_getconfig = (struct nc_rpc_getconfig *)rpc;
        if (rpc_getconfig->free) {
            free(rpc_getconfig->filter);
        }
        break;
    case NC_RPC_EDIT:
        rpc_edit = (struct nc_rpc_edit *)rpc;
        if (rpc_edit->free) {
            free(rpc_edit->edit_cont);
        }
        break;
    case NC_RPC_COPY:
        rpc_copy = (struct nc_rpc_copy *)rpc;
        if (rpc_copy->free) {
            free(rpc_copy->url_config_src);
        }
        break;
    case NC_RPC_DELETE:
        rpc_delete = (struct nc_rpc_delete *)rpc;
        if (rpc_delete->free) {
            free(rpc_delete->url);
        }
        break;
    case NC_RPC_GET:
        rpc_get = (struct nc_rpc_get *)rpc;
        if (rpc_get->free) {
            free(rpc_get->filter);
        }
        break;
    case NC_RPC_COMMIT:
        rpc_commit = (struct nc_rpc_commit *)rpc;
        if (rpc_commit->free) {
            free(rpc_commit->persist);
            free(rpc_commit->persist_id);
        }
        break;
    case NC_RPC_CANCEL:
        rpc_cancel = (struct nc_rpc_cancel *)rpc;
        if (rpc_cancel->free) {
            free(rpc_cancel->persist_id);
        }
        break;
    case NC_RPC_VALIDATE:
        rpc_validate = (struct nc_rpc_validate *)rpc;
        if (rpc_validate->free) {
            free(rpc_validate->url_config_src);
        }
        break;
    case NC_RPC_GETSCHEMA:
        rpc_getschema = (struct nc_rpc_getschema *)rpc;
        if (rpc_getschema->free) {
            free(rpc_getschema->identifier);
            free(rpc_getschema->version);
            free(rpc_getschema->format);
        }
        break;
    case NC_RPC_SUBSCRIBE:
        rpc_subscribe = (struct nc_rpc_subscribe *)rpc;
        if (rpc_subscribe->free) {
            free(rpc_subscribe->stream);
            free(rpc_subscribe->filter);
            free(rpc_subscribe->start);
            free(rpc_subscribe->stop);
        }
        break;
    case NC_RPC_KILL:
    case NC_RPC_DISCARD:
    case NC_RPC_LOCK:
    case NC_RPC_UNLOCK:
        /* nothing special needed */
        break;
    }

    free(rpc);
}

API void
nc_reply_free(struct nc_reply *reply)
{
    struct nc_reply_error *error;
    struct nc_reply_data *data;
    struct lyd_node *node;
    int i, j;

    if (!reply) {
        return;
    }

    switch(reply->type) {
    case NC_REPLY_DATA:
        data = (struct nc_reply_data *)reply;
        for (node = data->data; data->data; node = data->data) {
            data->data = node->next;
            lyd_free(node);
        }
        break;
    case NC_REPLY_OK:
        /* nothing to free */
        break;
    case NC_REPLY_ERROR:
        error = (struct nc_reply_error *)reply;
        for (i = 0; i < error->err_count; ++i) {
            lydict_remove(error->ctx, error->err[i].type);
            lydict_remove(error->ctx, error->err[i].tag);
            lydict_remove(error->ctx, error->err[i].severity);
            lydict_remove(error->ctx, error->err[i].apptag);
            lydict_remove(error->ctx, error->err[i].path);
            lydict_remove(error->ctx, error->err[i].message);
            lydict_remove(error->ctx, error->err[i].message_lang);
            lydict_remove(error->ctx, error->err[i].sid);
            for (j = 0; j < error->err[i].attr_count; ++j) {
                lydict_remove(error->ctx, error->err[i].attr[j]);
            }
            free(error->err[i].attr);
            for (j = 0; j < error->err[i].elem_count; ++j) {
                lydict_remove(error->ctx, error->err[i].elem[j]);
            }
            free(error->err[i].elem);
            for (j = 0; j < error->err[i].ns_count; ++j) {
                lydict_remove(error->ctx, error->err[i].ns[j]);
            }
            free(error->err[i].ns);
            for (j = 0; j < error->err[i].other_count; ++j) {
                lyxml_free(error->ctx, error->err[i].other[j]);
            }
            free(error->err[i].other);
        }
        break;
    case NC_REPLY_NOTIF:
        nc_notif_free((struct nc_notif *)reply);
        break;
    }
    free(reply);
}

API void
nc_notif_free(struct nc_notif *notif)
{
    if (!notif) {
        return;
    }

    lyd_free(notif->tree);
    free(notif);
}

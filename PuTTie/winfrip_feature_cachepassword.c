/*
 * winfrip_feature_cachepassword.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop
#include "tree234.h"

#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_rtl_tree.h"
#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_cachepassword.h"

/*
 * Private variables
 */

static bool		WffcpCachePasswords = false;
static WfrTree *	WffcpLoginTree = NULL;

/*
 * Private subroutine prototypes
 */

static void WffcpTreeFreeItem(WfrTreeItem *item);

/*
 * Private subroutines
 */

static void
WffcpTreeFreeItem(
	WfrTreeItem *	item
	)
{
	if (item->value) {
		smemclr(item->value, strlen(item->value));
		WFR_FREE(item->value);
		item->value = NULL;
	}
	WFR_FREE_IF_NOTNULL(item->key);
	item->key = NULL;
}

/*
 * Public subroutines
 */

WfReturn
WffCachePasswordOperation(
	WffCachePasswordOp	op,
	Conf *			conf,
	const char *		hostname,
	int			port,
	const char *		username,
	char **			ppassword
	)
{
	bool		cache_passwords;
	WfrTreeItem *	item;
	char *		password = NULL;
	char *		session_key = NULL;
	size_t		session_key_size;
	WfrStatus	status;


	if (op == WFF_CACHEPASSWORD_OP_RECONF) {
		cache_passwords = conf_get_bool(conf, CONF_frip_cache_passwords);
		if (!cache_passwords && WffcpCachePasswords) {
			if (WFR_FAILURE(status = WfrTreeClear(
				&WffcpLoginTree, WffcpTreeFreeItem)))
			{
				WFR_DEBUG_FAIL();
			}
		}
		WffcpCachePasswords = cache_passwords;
		return WF_RETURN_CONTINUE;
	} else if (!WffcpCachePasswords) {
		return WF_RETURN_CONTINUE;
	}

	if (!WffcpLoginTree) {
		WfrTreeInit(&WffcpLoginTree);
	}

	switch (op) {
	case WFF_CACHEPASSWORD_OP_RECONF:
		break;

	case WFF_CACHEPASSWORD_OP_GET:
		if (WFR_SUCCESS(status = WfrSnDuprintF(
			&session_key, NULL, "%s@%s:%d",
			username, hostname, port))
		&&  WFR_SUCCESS(status = WfrTreeGet(
			WffcpLoginTree, session_key,
			WFR_TREE_ITYPE_STRING, &item))
		&&  WFR_SUCCESS_POSIX(status, (password = strdup(item->value))))
		{
			*ppassword = password;
		} else {
			*ppassword = NULL;
		}
		WFR_FREE_IF_NOTNULL(session_key);
		break;

	case WFF_CACHEPASSWORD_OP_SET:
		if (WFR_FAILURE(status = WfrSnDuprintF(
			&session_key, &session_key_size, "%s@%s:%d",
			username, hostname, port))
		||  WFR_FAILURE_POSIX(status, (password = strdup(*ppassword)))
		||  WFR_FAILURE(status = WfrTreeSet(
			WffcpLoginTree, session_key,
			WFR_TREE_ITYPE_STRING, password,
			strlen(password) + 1, WffcpTreeFreeItem)))
		{
			WFR_FREE_IF_NOTNULL(password);
			WFR_DEBUG_FAIL();
		}
		WFR_FREE_IF_NOTNULL(session_key);
		break;

	case WFF_CACHEPASSWORD_OP_DELETE:
		if (WFR_FAILURE(status = WfrSnDuprintF(
			&session_key, &session_key_size, "%s@%s:%d",
			username, hostname, port))
		||  WFR_FAILURE(status = WfrTreeDelete(
			WffcpLoginTree, NULL, session_key,
			WFR_TREE_ITYPE_STRING, WffcpTreeFreeItem)))
		{
			WFR_DEBUG_FAIL();
		}
		WFR_FREE_IF_NOTNULL(session_key);
		break;

	case WFF_CACHEPASSWORD_OP_CLEAR:
		if (WFR_FAILURE(status = WfrTreeClear(
			&WffcpLoginTree, WffcpTreeFreeItem)))
		{
			WFR_DEBUG_FAIL();
		}
		break;

	default:
		WFR_DEBUG_FAIL();
	}

	return WF_RETURN_CONTINUE;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */

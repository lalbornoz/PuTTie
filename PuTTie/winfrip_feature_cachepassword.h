/*
 * winfrip_feature_cachepassword.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_FEATURE_CACHEPASSWORD_H
#define PUTTY_WINFRIP_FEATURE_CACHEPASSWORD_H

/*
 * Public defaults
 */

#define	WFF_CACHE_PASSWORDS	false

/*
 * Public type definitions and subroutine prototypes used by/in:
 * windows/window.c:WndProc()
 */

typedef enum WffCachePasswordOp {
	WFF_CACHEPASSWORD_OP_RECONF		= 1,
	WFF_CACHEPASSWORD_OP_GET		= 2,
	WFF_CACHEPASSWORD_OP_SET		= 3,
	WFF_CACHEPASSWORD_OP_DELETE		= 4,
	WFF_CACHEPASSWORD_OP_CLEAR		= 5,
	WFF_CACHEPASSWORD_OP_SERIALISE		= 6,
	WFF_CACHEPASSWORD_OP_DESERIALISE	= 7,
} WffCachePasswordOp;

WfReturn	WffCachePasswordOperation(WffCachePasswordOp op, Conf *conf, const char *hostname, int port, const char *username, char **ppassword, BinarySink *serbuf);

#endif // !PUTTY_WINFRIP_FEATURE_CACHEPASSWORD_H

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */

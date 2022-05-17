/*
 * winfrip_storage_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#ifndef PUTTY_WINFRIP_STORAGE_PRIV_H
#define PUTTY_WINFRIP_STORAGE_PRIV_H

/*
 * Preprocessor macros private to PuTTie/winfrip_storage*.c
 */

/*
 * XXX
 */
#ifdef WINFRIP_DEBUG
#define WINFRIP_STORAGE_DEBUG
#endif /* WINFRIP_DEBUG */

/*
 * XXX
 */
#ifdef WINFRIP_STORAGE_DEBUG
/* {{{ WINFRIP_STORAGE_DEBUG */
#define WFSP_DEBUG_ASSERT(expr) do {							\
	if (!(expr)) {												\
		WFSP_DEBUGF("assertion failure: %s\n"					\
					"GetLastError(): 0x%08x",					\
					#expr, GetLastError());						\
	}															\
} while (0)
#define WFSP_DEBUG_ASSERTF(expr, fmt, ...) do {					\
	if (!(expr)) {												\
		WFSP_DEBUGF(fmt "\n" "assertion failure: %s\n"			\
				      "GetLastError(): 0x%08x",					\
				      ##__VA_ARGS__, #expr, GetLastError());	\
	}															\
} while (0)
#define WFSP_DEBUG_FAIL()										\
	WFSP_DEBUGF("failure condition", __FILE__, __func__, __LINE__)
#define WFSP_DEBUGF(fmt, ...)									\
	wfcp_debugf(fmt, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
/* }}} */
#else
/* {{{ !WINFRIP_STORAGE_DEBUG */
#define WFSP_DEBUG_ASSERT(expr)									\
	(void)(expr)
#define WFSP_DEBUG_ASSERTF(expr, fmt, ...)						\
	(void)(expr)
#define WFSP_DEBUG_FAIL()
#define WFSP_DEBUGF(fmt, ...)
/* }}} */
#endif /* WINFRIP_STORAGE_DEBUG */

/*
 * Type definitions private to PuTTie/winfrip_storage*.c prototypes
 */

/*
 * XXX
 */

typedef uint32_t WfspHash;

/*
 * XXX
 */

typedef enum WfspHMapValueType {
	WFSP_HMAP_VTYPE_NONE		= 0,
	WFSP_HMAP_VTYPE_FILENAME	= 1,
	WFSP_HMAP_VTYPE_FONTSPEC	= 2,
	WFSP_HMAP_VTYPE_INT			= 3,
	WFSP_HMAP_VTYPE_STRING		= 4,
	WFSP_HMAP_VTYPE_ITEM		= 5,
	WFSP_HMAP_VTYPE_MAX			= 5,
} WfspHMapValueType;

/*
 * XXX
 */

typedef struct WfspHMapItem {
	struct WfspHMapItem *	next;

	void *					key;
	WfspHash				key_hash;
	size_t					key_len;

	void *					value;
	size_t					value_len;
	WfspHMapValueType		value_type;
} WfspHMapItem;
#define WFSP_HMAP_ITEM_EMPTY			\
	((WfspHMapItem){					\
	.next = NULL,						\
	.key = NULL,						\
	.key_hash = 0,						\
	.key_len = 0,						\
	.value = NULL,						\
	.value_len = 0,						\
	.value_type = WFSP_HMAP_VTYPE_NONE,	\
	})

/*
 * XXX
 */

typedef struct WfspHMapBin {
	WfspHMapItem *	item_first, *item_last;
} WfspHMapBin;
#define WFSP_HMAP_BIN_EMPTY	\
	((WfspHMapBin){			\
	.item_first = NULL,		\
	.item_last = NULL,		\
	})

/*
 * XXX
 */

typedef struct WfspHashMap {
	size_t			bin_bits;
	WfspHMapBin **	binv;
} WfspHashMap;
#define WFSP_HMAP_EMPTY	\
	((WfspHashMap){		\
	.bin_bits = 64,		\
	.binv = NULL,		\
	})

/*
 * XXX
 */

typedef enum WfspType {
	WFSP_TYPE_NONE			= 0,
	WFSP_TYPE_SESSION		= 1,
	WFSP_TYPE_SESSIONS		= 2,
	WFSP_TYPE_HOST_KEY		= 3,
	WFSP_TYPE_RANDOM_SEED	= 4,
	WFSP_TYPE_MAX			= 5,
} WfspType;

/*
 * XXX
 */

typedef struct WfspNone {
	void *	null;
} WfspNone;
#define WFSP_NONE_EMPTY		\
	((WfspNone){			\
	.null = NULL,			\
	})

/*
 * XXX
 */

typedef struct WfspSession {
	void *	null;
} WfspSession;
#define WFSP_SESSION_EMPTY	\
	((WfspSession){			\
	.null = NULL,			\
	})

/*
 * XXX
 */

typedef struct WfspSessions {
	void *	null;
} WfspSessions;
#define WFSP_SESSIONS_EMPTY	\
	((WfspSessions){		\
	.null = NULL,			\
	})

/*
 * XXX
 */

typedef struct WfspHostKey {
	const char *	name;
	int				port;
} WfspHostKey;
#define WFSP_HOST_KEY_EMPTY	\
	((WfspHostKey){			\
	.name = NULL,			\
	.port = -1,				\
	})

/*
 * XXX
 */

typedef struct WfspRandomSeed {
	void *	null;
} WfspRandomSeed;
#define WFSP_RANDOM_SEED_EMPTY	\
	((WfspRandomSeed){			\
	.null = NULL,				\
	})

/*
 * XXX
 */

typedef struct WfspItem {
	bool					dirty;
	size_t					rc;

	WfspType				type;
	union {
	struct WfspNone			none;
	struct WfspSession		session;
	struct WfspSessions		sessions;
	struct WfspHostKey		host_key;
	struct WfspRandomSeed	random_seed;
	}					u;

	WfspHashMap			hash_map;
} WfspItem;
#define WFSP_ITEM_EMPTY				\
	((WfspItem){					\
	.dirty = false,					\
	.rc = 0,						\
	.type = WFSP_TYPE_NONE,			\
	.u.none = WFSP_NONE_EMPTY,		\
	.hash_map = WFSP_HMAP_EMPTY,	\
	})

/*
 * Public subroutines private to PuTTie/winfrip_storage*.c prototypes
 * defined in winfrip_storage_priv.c
 */

int wfsp_add_session(WfspItem *item, const char *sessionname);
int wfsp_add_session_key(WfspItem *item, const char *key, void *value, size_t value_len, WfspHMapValueType value_type);
int wfsp_add_host_key(WfspItem *item, const char *hostname, int port, const char *keytype, const char *key);
int wfsp_add_random_seed(WfspItem *item, void *data, int len);

int wfsp_clear_sessions(WfspItem *item);
int wfsp_clear_host_keys(WfspItem *item);
int wfsp_clear_random_seed(WfspItem *item);

int wfsp_del_session(WfspItem *item, const char *sessionname);
int wfsp_del_host_key(WfspItem *item, const char *hostname, int port);

int wfsp_enumerate_sessions(WfspItem *item, const char **psessionname);

WfspHash wfsp_hash_fnv32a(void *buf, size_t buf_len, size_t nbits);

#endif /* !PUTTY_WINFRIP_STORAGE_PRIV_H */

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */

/*
 * winfrip_storage_priv.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2021 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "putty.h"

#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"
#include "FySTY/winfrip_storage_priv.h"

/*
 * Private subroutine prototypes
 */

static int wfsp_add_item(WfspItem *item, WfspItem *pitem_new, void *key, size_t key_len, void *value, size_t value_len, WfspHMapValueType value_type);
static int wfsp_clear_items(WfspItem *item, WfspType type);
static int wfsp_del_item(WfspItem *item, WfspType type, void *key, size_t key_len);
static int wfsp_enumerate_items(WfspItem *item, WfspType type, void **pkey, size_t *pkey_len);

/*
 * Private subroutines
 */

static int wfsp_add_item(WfspItem *item,
			 WfspItem *pitem_new,
			 void *key, size_t key_len,
			 void *value, size_t value_len,
			 WfspHMapValueType value_type)
{
    WfspHMapBin *binv_new = NULL;
    WfspItem *item_new = NULL;
    size_t nbit;
    int rc;


    WFSP_DEBUG_ASSERT(item);
    WFSP_DEBUG_ASSERT(pitem_new);
    WFSP_DEBUG_ASSERT(key);
    WFSP_DEBUG_ASSERT(key_len > 0);
    WFSP_DEBUG_ASSERT(value);
    WFSP_DEBUG_ASSERT(value_len > 0);
    WFSP_DEBUG_ASSERT(value_type <= WFSP_HMAP_VTYPE_MAX);

    /*
     * XXX
     */

    if (!(item_new = snew(WfspItem))) {
	WFSP_DEBUG_FAIL(); rc = -errno; goto err;
    } else {
	*item_new = WFSP_ITEM_EMPTY;
	if (!(item_new->key = snewn(key_len, uint8_t))
	||  !(item_new->value = snewn(value_len, uint8_t))) {
	    WFSP_DEBUG_FAIL(); rc = -errno; goto err;
	} else {
	    if (!item[type].hash_map.binv) {
		if (!(binv_new = snewn(item[type].hash_map.bin_bits, WfspHMapBin *))) {
		    WFSP_DEBUG_FAIL(); rc = -errno; goto err;
		} else {
		    ZeroMemory(binv_new, sizeof(*binv_new) * item[type].hash_map.bin_bits);
		}
	    }
	}
    }

    /*
     * XXX
     */

    if (binv_new) {
	item[type].hash_map.binv = binv_new;
    }
    item[type].hash_map.binv[key_hash & ((1 << item[type].hash_map.bin_bits) - 1)] = item_new;

    memcpy(item_new->key, key, key_len);
    item_new->key_hash = wfsp_fnv32a(key, key_len, sizeof(item_new->key_hash) * 8);
    item_new->key_len = key_len;

    memcpy(item_new->value, value, value_len);
    item_new->value_len = value_len;
    item_new->value_type = value_type;

    return ERROR_SUCCESS;

err:
    if (item_new) {
	if (item_new->key) {
	    sfree(item_new->key);
	}
	if (item_new->value) {
	    sfree(item_new->value);
	}
	sfree(item_new);
    }
    if (binv_new) {
	sfree(binv_new);
    }
    return rc;
}

/*
 * Public subroutines private to FySTY/winfrip_storage*.c prototypes
 * defined in winfrip_storage_priv.c
 */

int wfsp_add_session(WfspItem *item, const char *sessionname)
{
    WfspItem *item_new = NULL, *item_session_new = NULL;
    int rc = 0;


    /*
     * XXX
     */

    if (!(item_session_new = snew(WfspItem))) {
	WFSP_DEBUG_FAIL(); return -errno;
    } else {
	*item_session_new = WFSP_ITEM_EMPTY;
	item_session_new->type = WFSP_TYPE_SESSION;
	if (!(item_session_new->u.session.name = dupstr(sessionname))) {
	    rc = -errno; sfree(item_session_new); WFSP_DEBUG_FAIL(); return rc;
	}
    }

    rc = wfsp_add_item(item, &item_new, sessionname, strlen(sessionname),
		       item_session_new, sizeof(item_session_new),
		       WFSP_HMAP_VTYPE_ITEM);
    if (rc < 0) {
	sfree(item_new);
	sfree(item_session_new);
    }

    return rc;
}

int wfsp_add_session_key(WfspItem *item, const char *key, void *value, size_t value_len, WfspHMapValueType value_type)
{
    return -ENOSYS;
}

int wfsp_add_host_key(WfspItem *item, const char *hostname, int port, const char *keytype, const char *key)
{
    return -ENOSYS;
}

int wfsp_add_random_seed(WfspItem *item, void *data, int len)
{
    return -ENOSYS;
}


int wfsp_clear_sessions(WfspItem *item)
{
    return -ENOSYS;
}

int wfsp_clear_host_keys(WfspItem *item)
{
    return -ENOSYS;
}

int wfsp_clear_random_seed(WfspItem *item)
{
    return -ENOSYS;
}


int wfsp_del_session(WfspItem *item, const char *sessionname)
{
    return -ENOSYS;
}

int wfsp_del_host_key(WfspItem *item, const char *hostname, int port)
{
    return -ENOSYS;
}


int wfsp_enumerate_sessions(WfspItem *item, const char **psessionname)
{
    return -ENOSYS;
}


WfspHash wfsp_hash_fnv32a(void *buf, size_t buf_len, size_t nbits)
{
    WfspHash fnv32_hash = 0x811c9dc5, fnv32_prime = 0x1000193;
    uint8_t *p, *q;


    /*
     * XXX
     */

    p = (uint8_t *)buf; q = p + buf_len;
    while (p < q) {
	fnv32_hash ^= (WfspHash)*p++;
	fnv32_hash *= fnv32_prime;
    }
    fnv32_hash &= ((2 << (nbits - 1)) - 1);
    return fnv32_hash;
}

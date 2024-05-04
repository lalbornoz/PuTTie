/*
 * stubs-cachepassword.c: cache passwords stub function
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_cachepassword.h"

WfReturn
WffCachePasswordOperation(
	WffCachePasswordOp	op,
	Conf *			conf,
	const char *		hostname,
	int			port,
	const char *		username,
	char **			ppassword,
	BinarySink *		serbuf
)
{
	(void)op;
	(void)conf;
	(void)hostname;
	(void)port;
	(void)username;
	(void)ppassword;
	(void)serbuf;
	return WF_RETURN_CONTINUE;
}

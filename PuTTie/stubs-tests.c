/*
 * stubs-tests.c: stub functions for test*
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_urls.h"
#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_rtl_pcre2.h"
#include "PuTTie/winfrip_rtl_terminal.h"
#include "PuTTie/winfrip_rtl_windows.h"

void
add_session_to_jumplist(const char * const sessionname)
{
	(void)sessionname;
}

WfReturn
WffUrlsOperation(WffUrlsOp op, Conf *conf, HWND hwnd, UINT message, unsigned long *tattr, Terminal *term, WPARAM wParam, int x, int y)
{
	(void)op;
	(void)conf;
	(void)hwnd;
	(void)message;
	(void)tattr;
	(void)term;
	(void)wParam;
	(void)x;
	(void)y;
	return 0;
}

WfrStatus
WfrConvertUtf8ToUtf16String(const char *in, size_t in_len, wchar_t **poutW)
{
	(void)in;
	(void)in_len;
	(void)poutW;
	return WFR_STATUS_FROM_ERRNO1(ENOSYS);
}

bool
WfsDisableAdapterDisplayErrors(
	void
	)
{
	return false;
}

void
WfsSetAdapterDisplayErrors(
	bool	display_errorsfl
	)
{
	(void)display_errorsfl;
}

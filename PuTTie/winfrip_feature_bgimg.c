/*
 * winfrip_feature_bgimg.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "windows/win-gui-seat.h"
#pragma GCC diagnostic pop

#include <stdbool.h>
#include <sys/stat.h>

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_bgimg.h"
#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_rtl_file.h"
#include "PuTTie/winfrip_rtl_gdi.h"
#include "PuTTie/winfrip_rtl_random.h"
#include "PuTTie/winfrip_rtl_windows.h"

/*
 * Private type definitions
 */

typedef struct WffbpContext {
	WinGuiSeat *	wgs;
} WffbpContext;
#define WFFBP_CONTEXT_EMPTY {				\
	.wgs = NULL,					\
}
#define WFFBP_CONTEXT_INIT(ctx) ({			\
	(ctx) = (WffbpContext)WFFBP_CONTEXT_EMPTY;	\
	WFR_STATUS_CONDITION_SUCCESS;			\
})

typedef enum WffbpState {
	WFFBP_STATE_NONE		= 0,
	WFFBP_STATE_FAILED		= 1,
	WFFBP_STATE_INIT		= 2,
} WffbpState;

typedef enum WffbpSlideshow {
	WFFBP_SLIDESHOW_SINGLE_IMAGE	= 0,
	WFFBP_SLIDESHOW_DEFAULT		= WFFBP_SLIDESHOW_SINGLE_IMAGE,
	WFFBP_SLIDESHOW_SHUFFLE		= 1,
} WffbpSlideshow;

typedef enum WffbpType {
	WFFBP_TYPE_SOLID		= 0,
	WFFBP_TYPE_DEFAULT		= WFFBP_TYPE_SOLID,
	WFFBP_TYPE_IMAGE		= 1,
} WffbpType;

/*
 * Private variables
 */

/*
 * Absolute pathname to background images slideshow directory
 * Vector of background images filenames in background images slideshow directory
 * Absolute pathname to single background image filename
 */
static char *			WffbpDname = NULL;
static size_t			WffbpDnameLen = 0;
static size_t			WffbpDnameFileC = 0;
static char **			WffbpDnameFileV = NULL;
static char *			WffbpFname = NULL;

/*
 * Critical section serialising access to WffbpDname, event handle both used by the
 * slideshow directory change notification thread and the thread handle to the latter
 */
static CRITICAL_SECTION		WffbpDnameCriticalSection;
static HANDLE			WffbpDnameEvent;
static HANDLE			WffbpDnameWatchThread;

/*
 * Current and old device context handles
 * Current background image state
 * Current slideshow timer context passed to WffbpSlideshowTimerFunction()
 */
static HDC			WffbphDC = NULL;
static HGDIOBJ			WffbphDCOld = NULL;
static WffbpState		WffbpStateCurrent = WFFBP_STATE_NONE;
static WffbpContext *		WffbpTimerContext = NULL;
static int			WffbpSlideshowFrequency = -1;

/*
 * External subroutine prototypes
 */

// window.c
void		reset_window(WinGuiSeat *, int);

/*
 * Private subroutine prototypes
 */

static void		WffbpConfigPanelSlideshow(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void		WffbpConfigPanelStyle(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);
static void		WffbpConfigPanelType(dlgcontrol *ctrl, dlgparam *dlg, void *data, int event);

static WfrStatus	WffbpGetFname(Conf *conf, bool reshuffle, wchar_t **pbg_bmp_fname_w);
static WfrStatus	WffbpGetFnameShuffle(Conf *conf, bool reshuffle, wchar_t **pbg_bmp_fname_w);
static WfrStatus	WffbpGetFnameSingle(Conf *conf, bool reshuffle, wchar_t **pbg_bmp_fname_w);
static WfrStatus	WffbpSet(Conf *conf, HDC hdc, bool force, bool reshuffle, bool shutupfl);

static WfrStatus	WffbpSlideshowReconf(Conf *conf, HWND hWnd);
static WfrStatus	WffbpSlideshowReconfShuffle(Conf *conf, HWND hWnd);
static WfrStatus	WffbpSlideshowReconfSingle(Conf *conf, HWND hWnd);
static void		WffbpSlideshowTimerFunction(void *ctx, unsigned long now);

static WfReturn		WffbpOpDirChange(Conf *conf, HWND hwnd);
static WfReturn		WffbpOpDraw(int char_width, Conf *conf, HDC hdc, int font_height, int len, int nbg, bool *pbgfl, int rc_width, int x, int y);
static WfReturn		WffbpOpInit(Conf *conf, HWND hwnd);
static WfReturn		WffbpOpReconf(Conf *conf, HDC hdc, HWND hwnd);
static WfReturn		WffbpOpSize(Conf *conf, HDC hdc);

/*
 * Private subroutines
 */

static void
WffbpConfigPanelSlideshow(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Single image", WFFBP_SLIDESHOW_SINGLE_IMAGE);
		dlg_listbox_addwithid(ctrl, dlg, "Melbourne shuffle!", WFFBP_SLIDESHOW_SHUFFLE);

		switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
		case WFFBP_SLIDESHOW_SINGLE_IMAGE:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFBP_SLIDESHOW_SHUFFLE:
			dlg_listbox_select(ctrl, dlg, 1); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(
			conf, CONF_frip_bgimg_slideshow,
			dlg_listbox_getid(ctrl, dlg,
			dlg_listbox_index(ctrl, dlg)));
		break;
	}
}

static void
WffbpConfigPanelStyle(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Absolute", WFR_TI_STYLE_ABSOLUTE);
		dlg_listbox_addwithid(ctrl, dlg, "Center", WFR_TI_STYLE_CENTER);
		dlg_listbox_addwithid(ctrl, dlg, "Fit", WFR_TI_STYLE_FIT);
		dlg_listbox_addwithid(ctrl, dlg, "Stretch", WFR_TI_STYLE_STRETCH);
		dlg_listbox_addwithid(ctrl, dlg, "Tile", WFR_TI_STYLE_TILE);

		switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
		case WFR_TI_STYLE_ABSOLUTE:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFR_TI_STYLE_CENTER:
			dlg_listbox_select(ctrl, dlg, 1); break;
		case WFR_TI_STYLE_FIT:
			dlg_listbox_select(ctrl, dlg, 2); break;
		case WFR_TI_STYLE_STRETCH:
			dlg_listbox_select(ctrl, dlg, 3); break;
		case WFR_TI_STYLE_TILE:
			dlg_listbox_select(ctrl, dlg, 4); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(
			conf, CONF_frip_bgimg_style,
			dlg_listbox_getid(ctrl, dlg,
			dlg_listbox_index(ctrl, dlg)));
		break;
	}
}

static void
WffbpConfigPanelType(
	dlgcontrol *	ctrl,
	dlgparam *	dlg,
	void *		data,
	int		event
	)
{
	Conf *	conf = (Conf *)data;


	switch (event) {
	case EVENT_REFRESH:
		dlg_update_start(ctrl, dlg);
		dlg_listbox_clear(ctrl, dlg);
		dlg_listbox_addwithid(ctrl, dlg, "Solid", WFFBP_TYPE_SOLID);
		dlg_listbox_addwithid(ctrl, dlg, "Image", WFFBP_TYPE_IMAGE);

		switch (conf_get_int(conf, CONF_frip_bgimg_type)) {
		case WFFBP_TYPE_SOLID:
			dlg_listbox_select(ctrl, dlg, 0); break;
		case WFFBP_TYPE_IMAGE:
			dlg_listbox_select(ctrl, dlg, 1); break;
		default:
			WFR_DEBUG_FAIL(); break;
		}
		dlg_update_done(ctrl, dlg);
		break;

	case EVENT_SELCHANGE:
	case EVENT_VALCHANGE:
		conf_set_int(
			conf, CONF_frip_bgimg_type,
			dlg_listbox_getid(ctrl, dlg,
			dlg_listbox_index(ctrl, dlg)));
		break;
	}
}


static WfrStatus
WffbpGetFname(
	Conf *		conf,
	bool		reshuffle,
	wchar_t **	pbg_fname_w
	)
{
	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	default:
		return WFR_STATUS_FROM_ERRNO1(EINVAL);

	case WFFBP_SLIDESHOW_SINGLE_IMAGE:
		return WffbpGetFnameSingle(conf, reshuffle, pbg_fname_w);

	case WFFBP_SLIDESHOW_SHUFFLE:
		return WffbpGetFnameShuffle(conf, reshuffle, pbg_fname_w);
	}
}

static WfrStatus
WffbpGetFnameShuffle(
	Conf *		conf,
	bool		reshuffle,
	wchar_t **	pbg_fname_w
	)
{
	char *		bg_fname;
	size_t		bg_fname_idx;
	size_t		bg_fname_len;
	size_t		bg_fname_size;
	WfrStatus	status;


	(void)conf;

	if (!reshuffle && WffbpFname) {
		bg_fname = WffbpFname;
		bg_fname_len = strlen(bg_fname);
		status = WfrConvertUtf8ToUtf16String(bg_fname, bg_fname_len, pbg_fname_w);
	} else if ((WffbpDnameFileC > 0) && (WffbpDnameFileV != NULL)) {
		if (WFR_SUCCESS(status = WfrGenRandom(
			(PUCHAR)&bg_fname_idx, sizeof(bg_fname_idx))))
		{
			bg_fname_idx %= WffbpDnameFileC;
			bg_fname_len = strlen(
				  WffbpDnameFileV[bg_fname_idx]
				? WffbpDnameFileV[bg_fname_idx]
				: "");
			bg_fname_size = WffbpDnameLen + 1 + bg_fname_len + 1;

			if ((WffbpDnameLen == 0)
			||  (WffbpDnameFileV[bg_fname_idx] == NULL)
			||  (bg_fname_len == 0))
			{
				status = WFR_STATUS_FROM_ERRNO1(EINVAL);
			} else if (WFR_NEWN(status, bg_fname, bg_fname_size, char)) {
				EnterCriticalSection(&WffbpDnameCriticalSection);
				WFR_SNPRINTF(
					bg_fname, bg_fname_size, "%*.*s\\%s",
					(int)WffbpDnameLen, (int)WffbpDnameLen, WffbpDname,
					WffbpDnameFileV[bg_fname_idx]);
				LeaveCriticalSection(&WffbpDnameCriticalSection);

				bg_fname_len = strlen(bg_fname);
				status = WfrConvertUtf8ToUtf16String(bg_fname, bg_fname_len, pbg_fname_w);
				if (WFR_FAILURE(status)) {
					WFR_FREE(bg_fname);
				} else {
					WFR_FREE_IF_NOTNULL(WffbpFname);
					WffbpFname = bg_fname;
				}
			}
		}
	} else {
		pbg_fname_w = NULL;
		status = WFR_STATUS_CONDITION_SUCCESS;
	}

	return status;
}

static WfrStatus
WffbpGetFnameSingle(
	Conf *		conf,
	bool		reshuffle,
	wchar_t **	pbg_fname_w
	)
{
	char *		bg_fname;
	Filename *	bg_fname_conf;
	size_t		bg_fname_len;
	WfrStatus	status;


	(void)reshuffle;

	bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
	bg_fname = bg_fname_conf->path;
	status = WFR_STATUS_CONDITION_SUCCESS;

	if (WFR_SUCCESS(status)) {
		bg_fname_len = strlen(bg_fname);
		status = WfrConvertUtf8ToUtf16String(bg_fname, bg_fname_len, pbg_fname_w);
	}

	return status;
}

static WfrStatus
WffbpSet(
	Conf *	conf,
	HDC	hdc,
	bool	force,
	bool	reshuffle,
	bool	shutupfl
	)
{
	wchar_t *	bg_bmp_fname_w = NULL;
	HDC		bg_hdc = NULL;
	HGDIOBJ		bg_hdc_old = NULL;
	int		bg_height, bg_width;
	HBITMAP		bmp_src = NULL;
	WfrStatus	status;


	switch (WffbpStateCurrent) {
	default:
		break;
	case WFFBP_STATE_NONE:
		break;

	case WFFBP_STATE_FAILED:
		if (!force) {
			return WFR_STATUS_FROM_ERRNO1(EINVAL);
		}
		break;

	case WFFBP_STATE_INIT:
		if (!force) {
			return WFR_STATUS_CONDITION_SUCCESS;
		}
		break;
	}


	(void)WfrFreeBitmapDC(&WffbphDC, &WffbphDCOld);

	if (WFR_SUCCESS(status = WffbpGetFname(conf, reshuffle, &bg_bmp_fname_w))
	&&  bg_bmp_fname_w)
	{
		if (WFR_SUCCESS(status = WfrLoadImage(
				&bg_hdc, &bg_hdc_old, &bg_height,
				&bg_width, &bmp_src, bg_bmp_fname_w, hdc))
		&&  WFR_SUCCESS(status = WfrTransferImage(
				bg_hdc, bg_height, bg_width, bmp_src,
				hdc, conf_get_int(conf, CONF_frip_bgimg_padding),
				conf_get_int(conf, CONF_frip_bgimg_style)))
		&&  WFR_SUCCESS(status = WfrBlendImage(
				bg_hdc, bg_height, bg_width,
				conf_get_int(conf, CONF_frip_bgimg_opacity))))
		{
			(void)DeleteObject(bmp_src);

			WffbphDC = bg_hdc;
			WffbphDCOld = bg_hdc_old;
			WffbpStateCurrent = WFFBP_STATE_INIT;
			WFR_FREE(bg_bmp_fname_w);
		} else {
			(void)WfrFreeBitmapDC(&bg_hdc, &bg_hdc_old);
			if (bmp_src) {
				(void)DeleteObject(bmp_src);
			}

			WffbpStateCurrent = WFFBP_STATE_FAILED;
			WFR_FREE_IF_NOTNULL(bg_bmp_fname_w);
		}
	}

	if (!shutupfl) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "setting background image");
	}

	return status;
}


static WfrStatus
WffbpSlideshowReconf(
	Conf *	conf,
	HWND	hWnd
	)
{
	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	default:
		return WFR_STATUS_FROM_ERRNO1(EINVAL);

	case WFFBP_SLIDESHOW_SHUFFLE:
		return WffbpSlideshowReconfShuffle(conf, hWnd);

	case WFFBP_SLIDESHOW_SINGLE_IMAGE:
		return WffbpSlideshowReconfSingle(conf, hWnd);
	}
}

static WfrStatus
WffbpSlideshowReconfShuffle(
	Conf *	conf,
	HWND	hWnd
	)
{
	char *			bg_dname;
	Filename *		bg_dname_conf;
	size_t			dname_filec_new = 0;
	char **			dname_filev_new = NULL;
	char *			dname_new = NULL;
	int			freq_new;
	WfrStatus		status;
	WffbpContext *		timer_ctx_new = NULL;


	bg_dname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
	bg_dname = bg_dname_conf->path;

	if (WFR_NEW(status, timer_ctx_new, WffbpContext)
	&&  WFR_SUCCESS(status = WfrPathNameToDirectory(bg_dname, &dname_new))
	&&  WFR_SUCCESS(status = WfrEnumerateFilesV(
			dname_new, WFF_BGIMG_FILTER_IMAGE_FILES,
			&dname_filec_new, &dname_filev_new)))
	{
		EnterCriticalSection(&WffbpDnameCriticalSection);
		WFR_FREE_IF_NOTNULL(WffbpDname);
		WffbpDname = dname_new;
		LeaveCriticalSection(&WffbpDnameCriticalSection);

		WffbpDnameLen = strlen(WffbpDname);
		WFR_FREE_IF_NOTNULL(WffbpFname);
		WFR_FREE_VECTOR_IF_NOTNULL(WffbpDnameFileC, WffbpDnameFileV);
		WffbpDnameFileC = dname_filec_new;
		WffbpDnameFileV = dname_filev_new;

		(void)SetEvent(WffbpDnameEvent);

		freq_new = conf_get_int(conf, CONF_frip_bgimg_slideshow_freq) * 1000;
		if ((WffbpSlideshowFrequency != freq_new)
		||  !WffbpTimerContext)
		{
			if (WffbpTimerContext) {
				expire_timer_context(WffbpTimerContext);
				WFR_FREE(WffbpTimerContext);
			}
			WFFBP_CONTEXT_INIT(*timer_ctx_new);
			timer_ctx_new->wgs = (WinGuiSeat *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

			WffbpSlideshowFrequency = freq_new;
			WffbpTimerContext = timer_ctx_new;

			(void)schedule_timer(
				WffbpSlideshowFrequency,
				WffbpSlideshowTimerFunction,
				(void *)WffbpTimerContext);
		}
	} else {
		WFR_FREE_IF_NOTNULL(dname_new);
		WFR_FREE(timer_ctx_new);

		EnterCriticalSection(&WffbpDnameCriticalSection);
		WFR_FREE_IF_NOTNULL(WffbpDname);
		LeaveCriticalSection(&WffbpDnameCriticalSection);

		WffbpDnameLen = 0;
		WFR_FREE_IF_NOTNULL(WffbpFname);
		WFR_FREE_VECTOR_IF_NOTNULL(WffbpDnameFileC, WffbpDnameFileV);
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "configuring background image");

	return status;
}

static WfrStatus
WffbpSlideshowReconfSingle(
	Conf *	conf,
	HWND	hWnd
	)
{
	char *		bg_fname;
	wchar_t *	bg_fnameW = NULL;
	Filename *	bg_fname_conf;
	struct _stat64	statbuf;
	WfrStatus	status;


	(void)hWnd;

	bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
	bg_fname = bg_fname_conf->path;

	if (WFR_SUCCESS(status = WfrConvertUtf8ToUtf16String(bg_fname, strlen(bg_fname), &bg_fnameW))
	&&  WFR_SUCCESS_POSIX(status, (_wstat64(bg_fnameW, &statbuf) == 0))
	&&  WFR_SUCCESS_ERRNO1(status, EINVAL, !(statbuf.st_mode & S_IFDIR)))
	{
		EnterCriticalSection(&WffbpDnameCriticalSection);
		WFR_FREE_IF_NOTNULL(WffbpDname);
		LeaveCriticalSection(&WffbpDnameCriticalSection);

		WffbpDnameLen = 0;
		WFR_FREE_IF_NOTNULL(WffbpFname);
		WFR_FREE_VECTOR_IF_NOTNULL(WffbpDnameFileC, WffbpDnameFileV);

		WffbpSlideshowFrequency = -1;
		if (WffbpTimerContext) {
			expire_timer_context(WffbpTimerContext);
			WFR_FREE(WffbpTimerContext);
		}
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, NULL, "configuring background image");

	WFR_FREE_IF_NOTNULL(bg_fnameW);

	return status;
}

static void
WffbpSlideshowTimerFunction(
	void *		ctx,
	unsigned long	now
	)
{
	WffbpContext *	context = (WffbpContext *)ctx;
	HDC		hDC;


	(void)now;

	hDC = GetDC(context->wgs->term_hwnd);
	(void)WffbpSet(context->wgs->conf, hDC, true, true, true);
	(void)ReleaseDC(context->wgs->term_hwnd, hDC);
	(void)InvalidateRect(context->wgs->term_hwnd, NULL, TRUE);

	reset_window(context->wgs, 2);

	(void)schedule_timer(
		conf_get_int(context->wgs->conf,
		CONF_frip_bgimg_slideshow_freq) * 1000,
		WffbpSlideshowTimerFunction, ctx);
}


static WfReturn
WffbpOpDirChange(
	Conf *	conf,
	HWND	hwnd
	)
{
	(void)WffbpSlideshowReconf(conf, hwnd);
	return WF_RETURN_CONTINUE;
}

static WfReturn
WffbpOpDraw(
	int	char_width,
	Conf *	conf,
	HDC	hdc,
	int	font_height,
	int	len,
	int	nbg,
	bool *	pbgfl,
	int	rc_width,
	int	x,
	int	y
	)
{
	WfReturn	rc;
	BOOL		rc_windows;


	if ((nbg != 258) && (nbg != 259)) {
		rc = WF_RETURN_CONTINUE;
	} else if (!WffbphDC
		|| !WFR_SUCCESS(WffbpSet(conf, hdc, false, true, true)))
	{
		rc = WF_RETURN_FAILURE;
	} else {
		if (rc_width > 0) {
			rc_windows = BitBlt(
				hdc, x, y, rc_width, font_height,
				WffbphDC, x, y, SRCCOPY);
		} else {
			rc_windows = BitBlt(
				hdc, x, y, char_width * len,
				font_height, WffbphDC, x, y, SRCCOPY);
		}

		if (rc_windows > 0) {
			*pbgfl = true;
			rc = WF_RETURN_CONTINUE;
		} else {
			*pbgfl = false;
			rc = WF_RETURN_FAILURE;
		}
	}

	return rc;
}

static WfReturn
WffbpOpInit(
	Conf *	conf,
	HWND	hwnd
	)
{
	WfrStatus	status;


	if (WFR_FAILURE(status = WfrWatchDirectory(
			false, &WffbpDname, &WffbpDnameCriticalSection,
			hwnd, WFF_BGIMG_WM_CHANGEREQUEST, &WffbpDnameEvent,
			&WffbpDnameWatchThread)))
	{
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(
			status, NULL,
			"creating background image directory change notification thread");
		exit(1);
	} else {
		(void)WffbpSlideshowReconf(conf, hwnd);
		return WF_RETURN_CONTINUE;
	}
}

static WfReturn
WffbpOpReconf(
	Conf *	conf,
	HDC	hdc,
	HWND	hwnd
	)
{
	if (WFR_SUCCESS(WffbpSlideshowReconf(conf, hwnd))
	&&  WFR_SUCCESS(WffbpSet(conf, hdc, true, true, false)))
	{
		return WF_RETURN_CONTINUE;
	} else {
		return WF_RETURN_FAILURE;
	}
}

static WfReturn
WffbpOpSize(
	Conf *	conf,
	HDC	hdc
	)
{
	switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
	default:
		return WF_RETURN_FAILURE;

	case WFR_TI_STYLE_ABSOLUTE:
	case WFR_TI_STYLE_CENTER:
	case WFR_TI_STYLE_FIT:
	case WFR_TI_STYLE_STRETCH:
	case WFR_TI_STYLE_TILE:
		if (WFR_SUCCESS(WffbpSet(conf, hdc, true, false, true))) {
			return WF_RETURN_CONTINUE;
		} else {
			return WF_RETURN_FAILURE;
		}
		break;
	}
}

/*
 * Public subroutines private to PuTTie/winfrip*.c
 */

void
WffBgImgConfigPanel(
	struct controlbox *	b
	)
{
	dlgcontrol *		c;
	struct controlset *	s_bgimg_settings, *s_bgimg_params, *s_slideshow;


	/*
	 * The Frippery: background panel.
	 */

	ctrl_settitle(b, "Frippery/Background", "Configure pointless frippery: background image");

	/*
	 * The Frippery: Background image settings controls box.
	 */

	s_bgimg_settings = ctrl_getset(b, "Frippery/Background", "frip_bgimg_settings", "Background image settings");
	c = ctrl_filesel(
		s_bgimg_settings, "Image file/directory:", 'i',
		WFF_BGIMG_FILTER_IMAGE_FILES, FALSE, "Select background image file/directory",
		WFP_HELP_CTX, conf_filesel_handler, I(CONF_frip_bgimg_filename));
	c->fileselect.just_button = false;
	ctrl_text(
		s_bgimg_settings,
		"In order to select an image directory for slideshows, select an arbitrary file inside the directory in question.",
		WFP_HELP_CTX);
	ctrl_droplist(s_bgimg_settings, "Type:", 't', 45, WFP_HELP_CTX, WffbpConfigPanelType, P(NULL));
	ctrl_droplist(s_bgimg_settings, "Style:", 's', 45, WFP_HELP_CTX, WffbpConfigPanelStyle, P(NULL));

	/*
	 * The Frippery: Background image parameters control box.
	 */

	s_bgimg_params = ctrl_getset(b, "Frippery/Background", "frip_bgimg_params", "Background image parameters");
	ctrl_editbox(s_bgimg_params, "Opacity (0-100):", 'p', 15, WFP_HELP_CTX, conf_editbox_handler, I(CONF_frip_bgimg_opacity), ED_INT);
	ctrl_editbox(s_bgimg_params, "Fit padding (0-100):", 'n', 15, WFP_HELP_CTX, conf_editbox_handler, I(CONF_frip_bgimg_padding), ED_INT);

	/*
	 * The Frippery: Slideshow settings control box.
	 */

	s_slideshow = ctrl_getset(b, "Frippery/Background", "frip_bgimg_slideshow", "Slideshow settings");
	ctrl_droplist(s_slideshow, "Slideshow:", 'd', 45, WFP_HELP_CTX, WffbpConfigPanelSlideshow, P(NULL));
	ctrl_editbox(s_slideshow, "Slideshow frequency (in seconds):", 'f', 20, WFP_HELP_CTX, conf_editbox_handler, I(CONF_frip_bgimg_slideshow_freq), ED_INT);
}

/*
 * Public subroutines
 */

WfReturn
WffBgImgOperation(
	WffBgImgOp	op,
	bool *		pbgfl,
	Conf *		conf,
	HDC		hdc_in,
	HWND		hwnd,
	int		char_width,
	int		font_height,
	int		len,
	int		nbg,
	int		rc_width,
	int		x,
	int		y
	)
{
	Filename *	bg_fname_conf;
	HDC		hdc;
	WfReturn	rc;


	if ((conf_get_int(conf, CONF_frip_bgimg_type) != WFFBP_TYPE_IMAGE)
	||  !(bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename))
	||  (!bg_fname_conf->path || (strlen(bg_fname_conf->path) == 0)))
	{
		return WF_RETURN_CONTINUE;
	} else {
		hdc = hdc_in ? hdc_in : GetDC(hwnd);
	}

	switch (op) {
	default:
		rc = WF_RETURN_FAILURE;
		break;

	case WFF_BGIMG_OP_DIRCHANGE:
		rc = WffbpOpDirChange(conf, hwnd);
		break;
	case WFF_BGIMG_OP_DRAW:
		rc = WffbpOpDraw(char_width, conf, hdc, font_height, len, nbg, pbgfl, rc_width, x, y);
		break;
	case WFF_BGIMG_OP_INIT:
		rc = WffbpOpInit(conf, hwnd);
		break;
	case WFF_BGIMG_OP_RECONF:
		rc = WffbpOpReconf(conf, hdc, hwnd);
		break;
	case WFF_BGIMG_OP_SIZE:
		rc = WffbpOpSize(conf, hdc);
		break;
	}

	if (!hdc_in) {
		(void)ReleaseDC(hwnd, hdc);
	}
	return rc;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */

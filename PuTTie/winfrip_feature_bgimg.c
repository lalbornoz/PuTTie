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

#include <bcrypt.h>
#include <ntstatus.h>

#include <gdiplus/gdiplus.h>
#include <gdiplus/gdiplusflat.h>

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_bgimg.h"
#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_rtl_debug.h"
#include "PuTTie/winfrip_rtl_file.h"
#include "PuTTie/winfrip_rtl_gdi.h"

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

static char *			WffbpDname = NULL;
static size_t			WffbpDnameFileC = 0;
static char **			WffbpDnameFileV = NULL;
static size_t			WffbpDnameLen = 0;
static char *			WffbpFname = NULL;

static BCRYPT_ALG_HANDLE	WffbphAlgorithm = NULL;

static HDC			WffbphDC = NULL;
static HGDIOBJ			WffbphDCOld = NULL;

static WffbpState		WffbpStateCurrent = WFFBP_STATE_NONE;

static WffbpContext *		WffbpTimerContext = NULL;

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
static WfrStatus	WffbpGetFnameShuffle(void);
static WfrStatus	WffbpSet(Conf *conf, HDC hdc, bool force, bool reshuffle, bool shutupfl);
static WfrStatus	WffbpSlideshowReconf(Conf *conf, HWND hWnd);
static void		WffbpSlideshowTimerFunction(void *ctx, unsigned long now);

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
	char *		bg_fname;
	Filename *	bg_fname_conf;
	size_t		bg_fname_len;
	WfrStatus	status;


	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		break;

	case WFFBP_SLIDESHOW_SINGLE_IMAGE:
		bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
		bg_fname = bg_fname_conf->path;
		status = WFR_STATUS_CONDITION_SUCCESS;
		break;

	case WFFBP_SLIDESHOW_SHUFFLE:
		if ((!reshuffle && WffbpFname)
		||  (WFR_STATUS_SUCCESS(status = WffbpGetFnameShuffle())))
		{
			bg_fname = WffbpFname;
		}
		break;
	}

	if (WFR_STATUS_SUCCESS(status)) {
		bg_fname_len = strlen(bg_fname);
		status = WfrToWcsDup(bg_fname, bg_fname_len + 1, pbg_fname_w);
	}

	return status;
}

static WfrStatus
WffbpGetFnameShuffle(
	void
	)
{
	char *		bg_fname = NULL;
	size_t		bg_fname_idx;
	size_t		bg_fname_len;
	size_t		bg_fname_size;
	NTSTATUS	ntstatus;
	WfrStatus	status;


	if ((WffbpDnameFileC > 0) && (WffbpDnameFileV != NULL)) {
		if ((WffbphAlgorithm == NULL)
		&&  ((ntstatus = BCryptOpenAlgorithmProvider(
				&WffbphAlgorithm,
				BCRYPT_RNG_ALGORITHM, NULL, 0)) != STATUS_SUCCESS))
		{
			status = WFR_STATUS_FROM_WINDOWS_NT(ntstatus);
		} else if ((ntstatus = BCryptGenRandom(
				WffbphAlgorithm, (PUCHAR)&bg_fname_idx,
				sizeof(bg_fname_idx), 0) != STATUS_SUCCESS))
		{
			status = WFR_STATUS_FROM_WINDOWS_NT(ntstatus);
		} else {
			bg_fname_idx %= WffbpDnameFileC;
			bg_fname_len = strlen(
				  WffbpDnameFileV[bg_fname_idx]
				? WffbpDnameFileV[bg_fname_idx]
				: "");

			if ((WffbpDnameLen == 0)
			||  (WffbpDnameFileV[bg_fname_idx] == NULL)
			||  (bg_fname_len == 0))
			{
				status = WFR_STATUS_FROM_ERRNO1(EINVAL);
			} else if (!(bg_fname = WFR_NEWN(
					bg_fname_size = (WffbpDnameLen + 1 + bg_fname_len + 1),
					char)))
			{
				status = WFR_STATUS_FROM_ERRNO();
			} else {
				WFR_SNPRINTF(
					bg_fname, bg_fname_size, "%*.*s\\%s",
					(int)WffbpDnameLen, (int)WffbpDnameLen, WffbpDname,
					WffbpDnameFileV[bg_fname_idx]);

				WFR_FREE_IF_NOTNULL(WffbpFname);
				WffbpFname = bg_fname;
				status = WFR_STATUS_CONDITION_SUCCESS;
			}
		}
	} else {
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
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
		} else {
			break;
		}

	case WFFBP_STATE_INIT:
		if (!force) {
			return WFR_STATUS_CONDITION_SUCCESS;
		} else {
			break;
		}
	}


	(void)WfrFreeBitmapDC(&WffbphDC, &WffbphDCOld);

	if (WFR_STATUS_SUCCESS(status = WffbpGetFname(conf, reshuffle, &bg_bmp_fname_w))
	&&  WFR_STATUS_SUCCESS(status = WfrLoadImage(
			&bg_hdc, &bg_hdc_old, &bg_height,
			&bg_width, &bmp_src, bg_bmp_fname_w, hdc))
	&&  WFR_STATUS_SUCCESS(status = WfrTransferImage(
			bg_hdc, bg_height, bg_width, bmp_src,
			hdc, conf_get_int(conf, CONF_frip_bgimg_padding),
			conf_get_int(conf, CONF_frip_bgimg_style)))
	&&  WFR_STATUS_SUCCESS(status = WfrBlendImage(
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

	if (!shutupfl) {
		WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "setting background image");
	}

	return status;
}

static WfrStatus
WffbpSlideshowReconf(
	Conf *	conf,
	HWND	hWnd
	)
{
	char *			bg_dname;
	Filename *		bg_dname_conf;
	size_t			dname_filec_new = 0;
	char **			dname_filev_new = NULL;
	char *			dname_new = NULL;
	WfrStatus		status;
	WffbpContext *		timer_ctx_new = NULL;


	switch (conf_get_int(conf, CONF_frip_bgimg_slideshow)) {
	default:
		status = WFR_STATUS_FROM_ERRNO1(EINVAL);
		break;

	case WFFBP_SLIDESHOW_SINGLE_IMAGE:
		WFR_FREE_IF_NOTNULL(WffbpDname);
		WffbpDnameLen = 0;
		WFR_FREE_IF_NOTNULL(WffbpFname);
		WFR_FREE_VECTOR_IF_NOTNULL(WffbpDnameFileC, WffbpDnameFileV);

		if (WffbpTimerContext) {
			expire_timer_context(WffbpTimerContext);
			WFR_FREE(WffbpTimerContext);
		}

		status = WFR_STATUS_CONDITION_SUCCESS;
		break;

	case WFFBP_SLIDESHOW_SHUFFLE:
		bg_dname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename);
		bg_dname = bg_dname_conf->path;

		if (!(timer_ctx_new = WFR_NEW(WffbpContext))) {
			status = WFR_STATUS_FROM_ERRNO();
		} else if (WFR_STATUS_SUCCESS(status = WfrPathNameToDirectory(bg_dname, &dname_new))
			&& WFR_STATUS_SUCCESS(status = WfrEnumerateFilesV(
				dname_new, NULL, &dname_filec_new, &dname_filev_new)))
		{
			WFR_FREE_IF_NOTNULL(WffbpDname);
			WFR_FREE_IF_NOTNULL(WffbpFname);
			WFR_FREE_VECTOR_IF_NOTNULL(WffbpDnameFileC, WffbpDnameFileV);

			WffbpDname = dname_new;
			WffbpDnameLen = strlen(WffbpDname);
			WffbpDnameFileC = dname_filec_new;
			WffbpDnameFileV = dname_filev_new;

			if (WffbpTimerContext) {
				expire_timer_context(WffbpTimerContext);
				WFR_FREE(WffbpTimerContext);
			}
			WFFBP_CONTEXT_INIT(*timer_ctx_new);
			timer_ctx_new->wgs = (WinGuiSeat *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			WffbpTimerContext = timer_ctx_new;

			(void)schedule_timer(
				conf_get_int(conf, CONF_frip_bgimg_slideshow_freq) * 1000,
				WffbpSlideshowTimerFunction, (void *)WffbpTimerContext);
		}

		if (WFR_STATUS_FAILURE(status)) {
			WFR_FREE_IF_NOTNULL(dname_new);
			WFR_FREE(timer_ctx_new);

			WFR_FREE_IF_NOTNULL(WffbpDname);
			WffbpDnameLen = 0;
			WFR_FREE_IF_NOTNULL(WffbpFname);
			WFR_FREE_VECTOR_IF_NOTNULL(WffbpDnameFileC, WffbpDnameFileV);
		}

		break;
	}

	WFR_IF_STATUS_FAILURE_MESSAGEBOX(status, "configuring background image");

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


	if (conf_get_int(conf, CONF_frip_bgimg_type) != WFFBP_TYPE_IMAGE) {
		return WF_RETURN_CONTINUE;
	}
	if (!(bg_fname_conf = conf_get_filename(conf, CONF_frip_bgimg_filename))) {
		return WF_RETURN_CONTINUE;
	} else if (!bg_fname_conf->path || (strlen(bg_fname_conf->path) == 0)) {
		return WF_RETURN_CONTINUE;
	}

	if (hdc_in) {
		hdc = hdc_in;
	} else if (!hdc_in) {
		hdc = GetDC(hwnd);
	}

	switch (op) {
	default:
		rc = WF_RETURN_FAILURE;
		goto out;

	case WFF_BGIMG_OP_DRAW:
		if ((nbg == 258) || (nbg == 259)) {
			if (WffbphDC
			||  WFR_STATUS_SUCCESS(WffbpSet(conf, hdc, false, true, true)))
			{
				if (rc_width > 0) {
					rc = BitBlt(
						hdc, x, y, rc_width, font_height,
						WffbphDC, x, y, SRCCOPY) > 0;
				} else {
					rc = BitBlt(
						hdc, x, y, char_width * len,
						font_height, WffbphDC, x, y, SRCCOPY) > 0;
				}
				if (rc) {
					*pbgfl = true;
					rc = WF_RETURN_CONTINUE;
					goto out;
				} else {
					*pbgfl = false;
					rc = WF_RETURN_FAILURE;
					goto out;
				}
			} else {
				rc = WF_RETURN_FAILURE;
				goto out;
			}
		} else {
			rc = WF_RETURN_CONTINUE;
			goto out;
		}

	case WFF_BGIMG_OP_INIT:
		(void)WffbpSlideshowReconf(conf, hwnd);
		rc = WF_RETURN_CONTINUE;
		break;

	case WFF_BGIMG_OP_RECONF:
		if (WFR_STATUS_SUCCESS(WffbpSlideshowReconf(conf, hwnd))
		&&  WFR_STATUS_SUCCESS(WffbpSet(conf, hdc, true, true, false)))
		{
			rc = WF_RETURN_CONTINUE;
			goto out;
		} else {
			rc = WF_RETURN_FAILURE;
			goto out;
		}

	case WFF_BGIMG_OP_SIZE:
		switch (conf_get_int(conf, CONF_frip_bgimg_style)) {
		default:
			rc = WF_RETURN_FAILURE;
			goto out;

		case WFR_TI_STYLE_ABSOLUTE:
		case WFR_TI_STYLE_TILE:
			rc = WF_RETURN_CONTINUE;
			goto out;

		case WFR_TI_STYLE_CENTER:
		case WFR_TI_STYLE_FIT:
		case WFR_TI_STYLE_STRETCH:
			if (WFR_STATUS_SUCCESS(WffbpSet(conf, hdc, true, false, true))) {
				rc = WF_RETURN_CONTINUE;
				goto out;
			} else {
				rc = WF_RETURN_FAILURE;
				goto out;
			}
		}
	}

out:
	if (!hdc_in) {
		(void)ReleaseDC(hwnd, hdc);
	}
	return rc;
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */

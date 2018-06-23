/*
 * winfrip_priv.h - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
 */

#ifndef PUTTY_WINFRIP_PRIV_H
#define PUTTY_WINFRIP_PRIV_H

/*
 * Private enumerations
 */

/*
 * XXX document
 */
typedef enum WinFripBgImgState {
    WINFRIP_BGIMG_STATE_NONE		= 0,
    WINFRIP_BGIMG_STATE_FAILED		= 1,
    WINFRIP_BGIMG_STATE_INIT		= 2,
} WinFripBgImgState;

/*
 * XXX document
 */
typedef enum WinFripBgImgStyle {
    WINFRIP_BGIMG_STYLE_ABSOLUTE	= 0,
    WINFRIP_BGIMG_STYLE_CENTER		= 1,
    WINFRIP_BGIMG_STYLE_STRETCH		= 2,
    WINFRIP_BGIMG_STYLE_TILE		= 3,
} WinFripBgImgStyle;

/*
 * XXX document
 */
typedef enum WinFripBgImgType {
    WINFRIP_BGIMG_TYPE_SOLID		= 0,
    WINFRIP_BGIMG_TYPE_IMAGE		= 1,
} WinFripBgImgType;

/*
 * XXX document
 */
typedef enum WinFripMouseRmb {
    WINFRIP_MOUSE_RMB_NORMAL		= 0,
    WINFRIP_MOUSE_RMB_INHIBIT		= 1,
} WinFripMouseRmb;

/*
 * XXX document
 */
typedef enum WinFripTranspLevel {
    WINFRIP_TRANSP_LEVEL_OFF		= 255,
    WINFRIP_TRANSP_LEVEL_LOW		= 255 - 16,
    WINFRIP_TRANSP_LEVEL_MEDIUM		= 255 - 32,
    WINFRIP_TRANSP_LEVEL_HIGH		= 255 - 48,
} WinFripTranspLevel;

/*
 * XXX document
 */
typedef enum WinFripTranspOpaqueOn {
    WINFRIP_TRANSP_OPAQUE_NEVER		= 0,
    WINFRIP_TRANSP_OPAQUE_FOCUS_KILL	= 1,
    WINFRIP_TRANSP_OPAQUE_FOCUS_SET	= 2,
} WinFripTranspOpaqueOn;

/*
 * XXX document
 */
typedef enum WinFripTranspSetting {
    WINFRIP_TRANSP_SETTING_OFF		= 0,
    WINFRIP_TRANSP_SETTING_LOW		= 1,
    WINFRIP_TRANSP_SETTING_MEDIUM	= 2,
    WINFRIP_TRANSP_SETTING_HIGH		= 3,
    WINFRIP_TRANSP_SETTING_CUSTOM	= 4,
} WinFripTranspSetting;

/*
 * XXX document
 */
typedef enum WinFripUrlsState {
    WINFRIP_URLS_STATE_NONE		= 0,
    WINFRIP_URLS_STATE_SELECT		= 1,
    WINFRIP_URLS_STATE_CLICK		= 2,
    WINFRIP_URLS_STATE_CLEAR		= 3,
} WinFripUrlsState;

/*
 * Public subroutine private to FySTY/winfrip*.c prototypes
 */

BOOL winfrip_towcsdup(char *in, size_t in_size, wchar_t **pout_w);

#endif

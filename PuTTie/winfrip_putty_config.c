/*
 * winfrip_putty_config.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_feature_bgimg.h"
#include "PuTTie/winfrip_feature_general.h"
#include "PuTTie/winfrip_feature_mouse.h"
#include "PuTTie/winfrip_feature_storage_general.h"
#include "PuTTie/winfrip_feature_storage_host_ca.h"
#include "PuTTie/winfrip_feature_storage_host_keys.h"
#include "PuTTie/winfrip_feature_storage_sessions.h"
#include "PuTTie/winfrip_feature_trans.h"
#include "PuTTie/winfrip_feature_urls.h"
#include "PuTTie/winfrip_putty_config.h"

/*
 * Public subroutines
 */

void
WfConfigPanel(
	struct controlbox *	b
	)
{
	/*
	 * The Frippery panel.
	 */
	ctrl_settitle(b, "Frippery", "Pointless frippery");

	WffBgImgConfigPanel(b);
	WffGeneralConfigPanel(b);
	WffMouseConfigPanel(b);
	WffsGeneralConfigPanel(b);
	WffsHostCAConfigPanel(b);
	WffsHostKeysConfigPanel(b);
	WffsSessionsConfigPanel(b);
	WffTransConfigPanel(b);
	WffUrlsConfigPanel(b);
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */

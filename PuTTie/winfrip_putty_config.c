/*
 * winfrip_putty_config.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#include "dialog.h"
#include "terminal.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_feature.h"
#include "PuTTie/winfrip_putty_config.h"

/*
 * Public subroutines
 */

void winfrip_config_panel(struct controlbox *b)
{
	/*
	 * The Frippery panel.
	 */
	ctrl_settitle(b, "Frippery", "Pointless frippery");

	winfripp_bgimg_config_panel(b);
	winfripp_general_config_panel(b);
	winfripp_mouse_config_panel(b);
	winfripp_trans_config_panel(b);
	winfripp_urls_config_panel(b);
}

/*
 * vim:noexpandtab sw=4 ts=4 tw=0
 */

/*
 * winfrip_pcre2.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#include "putty.h"
#include "FySTY/winfrip.h"
#include "FySTY/winfrip_priv.h"
#include "FySTY/winfrip_pcre2.h"

/*
 * Public subroutines private to FySTY/winfrip*.c prototypes
 */

void winfripp_pcre2_init(WinFrippP2MGState *state, pcre2_code *code, size_t length, pcre2_match_data *md, wchar_t *subject)
{
    uint32_t newline = 0;

    WINFRIPP_DEBUG_ASSERT(state);
    WINFRIPP_DEBUG_ASSERT(code);
    WINFRIPP_DEBUG_ASSERT(md);
    WINFRIPP_DEBUG_ASSERT(subject);

    state->code = code;
    state->first_match = true;
    state->last_error = 0;
    state->length = length;
    state->md = md;
    state->ovector = pcre2_get_ovector_pointer(state->md);
    state->startoffset = 0;
    state->subject = subject;

    WINFRIPP_DEBUG_ASSERT(state->ovector);
    ZeroMemory(state->ovector, pcre2_get_ovector_count(state->md) * 2 * sizeof(*state->ovector));

    (void)pcre2_pattern_info(state->code, PCRE2_INFO_NEWLINE, &newline);
    state->crlf_is_newline =
	       (newline == PCRE2_NEWLINE_ANY)
	    || (newline == PCRE2_NEWLINE_CRLF)
	    || (newline == PCRE2_NEWLINE_ANYCRLF);
}

WinFrippP2MGReturn winfripp_pcre2_match_global(WinFrippP2MGState *state, size_t *pbegin, size_t *pend)
{
    wchar_t *match_w;
    uint32_t options = 0;
    WinFrippP2MGReturn rc = WINFRIPP_P2MG_ERROR;
    PCRE2_SIZE startchar;


    WINFRIPP_DEBUG_ASSERT(state);
    WINFRIPP_DEBUG_ASSERT(state->code);
    WINFRIPP_DEBUG_ASSERT(state->md);
    WINFRIPP_DEBUG_ASSERT(state->ovector);
    WINFRIPP_DEBUG_ASSERT(state->startoffset < state->length);
    WINFRIPP_DEBUG_ASSERT(state->subject);
    WINFRIPP_DEBUG_ASSERT(pbegin);
    WINFRIPP_DEBUG_ASSERT(pend);

    /*
     * If the previous match was for an empty string, we are finished if we are
     * at the end of the subject. Otherwise, arrange to run another match at the
     * same point to see if a non-empty match can be found.
     */

    if (!state->first_match) {
	if (state->ovector[0] == state->ovector[1]) {
	    if (state->ovector[0] == state->length) {
		*pbegin = state->ovector[0], *pend = state->ovector[1];
		rc = WINFRIPP_P2MG_DONE; goto out;		/* Subject is empty string */
	    } else {
		options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
	    }

    /*
     * If the previous match was not an empty string, there is one tricky case to
     * consider. If a pattern contains \K within a lookbehind assertion at the
     * start, the end of the matched string can be at the offset where the match
     * started. Without special action, this leads to a loop that keeps on matching
     * the same substring. We must detect this case and arrange to move the start on
     * by one character. The pcre2_get_startchar() function returns the starting
     * offset that was passed to pcre2_match().
     */

	} else {
	    if (state->startoffset <= (startchar = pcre2_get_startchar(state->md))) {
		if (startchar >= state->length) {
		    *pbegin = state->ovector[0], *pend = state->ovector[1];
		    rc = WINFRIPP_P2MG_DONE; goto out;		/* Reached end of subject. */
		} else {
		    state->startoffset = startchar + 1;		/* Advance by one character. */
		}
	    }
	}
    }

    /*
     * Execute pcre2_match(), either starting a new iterated sequence of attempts
     * to match or restarting from a/the previous attempt.
     */

match:
    state->last_error =
	    pcre2_match(state->code, state->subject, state->length,
			state->startoffset, options, state->md, NULL);
    if (state->first_match) {
	state->first_match = false;
    }

    switch (state->last_error) {
    case 0:
	state->startoffset = state->ovector[1];
	*pbegin = state->ovector[0], *pend = state->ovector[1];
	rc = WINFRIPP_P2MG_CONTINUE; goto out;

    default:
	if (state->last_error == PCRE2_ERROR_NOMATCH) {
	    if (options == 0) {
		state->last_error = 0; rc = WINFRIPP_P2MG_NO_MATCH; goto out;
	    } else {
		state->startoffset++;				/* Advance by one character. */
		if (state->crlf_is_newline			/* If CRLF is a newline & */
		&&  (state->startoffset < state->length - 1)	/* we are at CRLF, */
		&&  (state->subject[state->startoffset] == '\r')
		&&  (state->subject[state->startoffset + 1] == '\n'))
		{
		    state->startoffset++;			/* Advance by one more. */
		}
		goto match;					/* Go round the loop again */
	    }
	} else {
	    rc = WINFRIPP_P2MG_ERROR; goto out;
	}
	break;
    }

out:
    /*
     * We must guard against patterns such as /(?=.\K)/ that use \K in an
     * assertion to set the start of a match later than its end. In this
     * demonstration program, we just detect this case and give up.
     */

    if (state->ovector[0] > state->ovector[1]) {
	rc = WINFRIPP_P2MG_ERROR;
    }

    return rc;
}

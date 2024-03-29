#include "putty.h"
#include "storage.h"

/* {{{ winfrip */
#include "PuTTie/winfrip_rtl.h"
#include "PuTTie/winfrip_storage.h"
/* winfrip }}} */

const unsigned cmdline_tooltype =
    TOOLTYPE_NONNETWORK |
    TOOLTYPE_NO_VERBOSE_OPTION;

void gui_term_process_cmdline(Conf *conf, char *cmdline)
{
    /* {{{ winfrip */
    WfrStatus    status;


    WfrDebugInit();
    if (WFR_STATUS_FAILURE(status = WfsInit())) {
        WFR_IF_STATUS_FAILURE_MESSAGEBOX1("pterm", status, "initialising storage");
        exit(1);
    } else if (WFR_STATUS_FAILURE(WfsSetBackendFromCmdLine(cmdline))) {
        WFR_IF_STATUS_FAILURE_MESSAGEBOX1("pterm", status, "setting backend");
        exit(1);
    }
    /* winfrip }}} */

    do_defaults(NULL, conf);
    conf_set_str(conf, CONF_remote_cmd, "");

    cmdline = handle_restrict_acl_cmdline_prefix(cmdline);
    if (handle_special_sessionname_cmdline(cmdline, conf) ||
        handle_special_filemapping_cmdline(cmdline, conf))
        return;

    int argc;
    char **argv, **argstart;
    split_into_argv(cmdline, false, &argc, &argv, &argstart);

    for (int i = 0; i < argc; i++) {
        char *arg = argv[i];
        int retd = cmdline_process_param(
            arg, i+1<argc?argv[i+1]:NULL, 1, conf);
        if (retd == -2) {
            cmdline_error("option \"%s\" requires an argument", arg);
        } else if (retd == 2) {
            i++;               /* skip next argument */
        } else if (retd == 1) {
            continue;          /* nothing further needs doing */
        } else if (!strcmp(arg, "-e")) {
            if (i+1 < argc) {
                /* The command to execute is taken to be the unparsed
                 * version of the whole remainder of the command line. */
                conf_set_str(conf, CONF_remote_cmd, argstart[i+1]);
                return;
            } else {
                cmdline_error("option \"%s\" requires an argument", arg);
            }
        } else if (arg[0] == '-') {
            cmdline_error("unrecognised option \"%s\"", arg);
        } else {
            cmdline_error("unexpected non-option argument \"%s\"", arg);
        }
    }

    cmdline_run_saved(conf);

    conf_set_int(conf, CONF_sharrow_type, SHARROW_BITMAP);
}

const struct BackendVtable *backend_vt_from_conf(Conf *conf)
{
    return &conpty_backend;
}

const wchar_t *get_app_user_model_id(void)
{
    return L"SimonTatham.Pterm";
}

void gui_terminal_ready(HWND hwnd, Seat *seat, Backend *backend)
{
}

/* {{{ winfrip */
/*
 * vim:expandtab sw=4 ts=4
 */
/* winfrip }}} */

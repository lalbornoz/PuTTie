/*
 * winfrip_subr_putty_help.c - pointless frippery & tremendous amounts of bloat
 * Copyright (c) 2018, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "putty.h"
#pragma GCC diagnostic pop

#include "PuTTie/winfrip_subr_putty_help.h"

/*
 * Private subroutine prototypes
 */

static void WfpPuttyUsage(void);

/*
 * Private subroutines
 */

static void
WfpPuttyUsage(
	void
)
{
	/*
	 * (based on plink.c:usage(), doc/using.but, and PuTTie/README.md)
	 */

	printf("Usage: putty [options] [user@]host\n");
	printf("       (\"host\" can also be a PuTTY saved session name)\n");
	printf("\n");
	printf("Options:\n");
	printf("  -?\n");
	printf("  -h\n");
	printf("  --help\n");
	printf("            display this help screen and exit\n");
	printf("  -cleanup  remove registry entries, random seed file, jump list\n");
	printf("  -load sessname\n");
	printf("            load settings from saved session\n");
	printf("  -ssh -telnet -rlogin -rlogin -supdup -raw -serial\n");
	printf("            force use of a particular protocol\n");
	printf("  -ssh-connection\n");
	printf("            force use of the bare ssh-connection protocol\n");
	printf("  -v        increase verbosity\n");
	printf("  -l user   connect with specified username\n");
	printf("  -L [listen-IP:]listen-port:host:port\n");
	printf("            forward local port to remote address\n");
	printf("  -R [listen-IP:]listen-port:host:port\n");
	printf("            forward remote port to local address\n");
	printf("  -D [listen-IP:]listen-port\n");
	printf("            dynamic SOCKS-based port forwarding\n");
	printf("  -m file   read remote command(s) from file\n");
	printf("  -P port   connect to specified port\n");
	printf("  -pwfile file\n");
	printf("            login with password read from specified file\n");
	printf("  -pw password\n");       
	printf("            login with password; insecure, use -pwfile\n");
	printf("  -agent    enable use of Pageant\n");
	printf("  -noagent  disable use of Pageant\n");
	printf("  -A -a     enable / disable agent forwarding\n");
	printf("  -X -x     enable / disable X11 forwarding\n");
	printf("  -t -T     enable / disable pty allocation\n");
	printf("  -N        don't start a shell/command (SSH-2 only)\n");
	printf("  -nc host:port\n");
	printf("            open tunnel in place of session (SSH-2 only)\n");
	printf("  -C        enable compression\n");
	printf("  -1 -2     force use of particular SSH protocol version\n");
	printf("  -4 -6     force use of IPv4 or IPv6\n");
	printf("  -i key    private key file for user authentication\n");
	printf("  -cert file\n");
	printf("            certificate file of a signed version of public key\n");
	printf("  -no-trivial-auth\n");
	printf("            disconnect if SSH authentication succeeds trivially\n");
	printf("  -loghost host\n");
	printf("            set logical host name used for SSH host key caching\n");
	printf("  -hostkey keyid\n");
	printf("            manually specify a host key (may be repeated)\n");
	printf("  -pgpfp    print PGP key fingerprints and exit\n");
	printf("  -sercfg configuration-string (e.g. 19200,8,n,1,X)\n");
	printf("            specify the serial configuration (serial only)\n");
	printf("  -sessionlog file\n");
	printf("  -sshlog file\n");
	printf("  -sshrawlog file\n");
	printf("            log protocol details to a file\n");
	printf("  -logoverwrite\n");
	printf("  -logappend\n");
	printf("            control what happens when a log file already exists\n");
	printf("  -proxycmd command\n");
	printf("            use 'command' as local proxy\n");
	printf("  -restrict-acl\n");
	printf("            restrict Windows process ACL\n");
	printf("  -host-ca  launch host CA configuration dialog box\n");
	printf("\n");
	printf("PuTTie-specific options:\n");
	printf("  --file\n");
	printf("  --ephemeral\n");
	printf("            select storage backend\n");
	printf("  --file=registry\n");
	printf("  --ephemeral=file\n");
	printf("  --ephemeral=registry\n");
	printf("            select storage backend and init from other backend\n");
}

/*
 * Public subroutine prototypes private to windows/putty.c
 */

void
WfPuttyCmdLineHelp(
	const char *	cmdline,
	bool *		pexitfl,
	int *		pexit_status
)
{
	const char *	arg, *arg_next;
	char *		arg_;
	size_t		arg_len;
	WfrStatus	status;


	for (arg = cmdline, *pexitfl = false;
	     arg && arg[0] && WFR_SUCCESS(status) && !(*pexitfl);
	     arg = arg_next)
	{
		while (*arg == ' ') {
			arg++;
		}
		arg_next = strchr(arg, ' ');
		arg_len = (arg_next ? (size_t)(arg_next - arg) : strlen(arg));

		if (WFR_NEWN(status, arg_, arg_len + 1, char)) {
			memcpy(arg_, arg, arg_len); arg_[arg_len] = '\0';
			if ((strcmp(arg_, "-?") == 0)
			||  (strcmp(arg_, "-h") == 0)
			||  (strcmp(arg_, "--help") == 0))
			{
				WfpPuttyUsage();
				*pexitfl = true, *pexit_status = 0;
			}
			WFR_FREE(arg_);
		}
	}
}

/*
 * vim:noexpandtab sw=8 ts=8 tw=0
 */

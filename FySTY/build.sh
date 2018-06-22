#!/bin/sh
#

usage() {
	echo "usage: ${0} [-h] [-m] [-u] [[--] make args...]" >&2;
	echo "       -h.......: show this screen" >&2;
	echo "       -m.......: select \`mingw' build type" >&2;
	echo "       -u.......: select \`unix' build type" >&2;
	exit 0;
};

build() {
	local _build_type="" _opt="";

	while getopts hmu _opt; do
	case "${_opt}" in
	m)	_build_type="mingw"; ;;
	u)	_build_type="unix"; ;;
	*)	usage; ;;
	esac; done; shift $((${OPTIND}-1));

	case "${_build_type}" in
	mingw)
		cd windows;
		make -f Makefile.mgw COMPAT=-DNO_MULTIMON TOOLPATH=x86_64-w64-mingw32- "${@}";
		;;
	unix)
		cd unix;
		make "${@}";
		;;
	*)
		usage; ;;
	esac;
};

set -o errexit -o noglob;
build "${@}";

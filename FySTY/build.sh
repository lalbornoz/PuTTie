#!/bin/sh
# Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
#

usage() {
	echo "usage: ${0} [-d] [-h] [-m] [-u] [[--] make args...]" >&2;
	echo "       -d.......: build w/ XFLAGS=-DWINFRIP_DEBUG" >&2;
	echo "       -h.......: show this screen" >&2;
	echo "       -m.......: select \`mingw' build type" >&2;
	echo "       -u.......: select \`unix' build type" >&2;
	exit 0;
};

build() {
	local _build_type="" _dflag=0 _makeflags_extra="" _opt="";

	while getopts dhmu _opt; do
	case "${_opt}" in
	d)	_dflag=1; ;;
	m)	_build_type="mingw"; ;;
	u)	_build_type="unix"; ;;
	*)	usage; ;;
	esac; done; shift $((${OPTIND}-1));

	case "${_build_type}" in
	mingw)
		_makeflags_extra="COMPAT=-DNO_MULTIMON TOOLPATH=x86_64-w64-mingw32-";
		if [ "${_dflag:-0}" -eq 1 ]; then
			_makeflags_extra="${_makeflags_extra:+${_makeflags_extra} }XFLAGS=-DWINFRIP_DEBUG";
		fi;
		cd windows;
		make -f Makefile.mgw ${_makeflags_extra} "${@}";
		;;
	unix)
		cd unix;
		make ${_makeflags_extra} "${@}";
		;;
	*)
		usage; ;;
	esac;
};

set -o errexit -o noglob;
build "${@}";

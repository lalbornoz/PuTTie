#!/bin/sh
# Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
#

usage() {
	echo "usage: ${0} [-c] [-d] [-h] [-m] [-r] [-u] [[--] make args...]" >&2;
	echo "       -c.......: make [ ... ] clean before build" >&2;
	echo "       -d.......: build w/ XFLAGS=-DWINFRIP_DEBUG" >&2;
	echo "       -h.......: show this screen" >&2;
	echo "       -m.......: select \`mingw' build type" >&2;
	echo "       -r.......: call mkfiles.pl before build" >&2;
	echo "       -u.......: select \`unix' build type" >&2;
	exit 0;
};

build() {
	local _build_type="" _cflag=0 _dflag=0 _makeflags_extra="" _opt="" _rflag=0;

	while getopts cdhmru _opt; do
	case "${_opt}" in
	c)	_cflag=1; ;;
	d)	_dflag=1; ;;
	m)	_build_type="mingw"; ;;
	r)	_rflag=1; ;;
	u)	_build_type="unix"; ;;
	*)	usage; ;;
	esac; done; shift $((${OPTIND}-1));

	case "${_build_type}" in
	mingw)
		if [ "${_rflag:-0}" -eq 1 ]; then
			echo ./mkfiles.pl; ./mkfiles.pl;
		fi;
		_makeflags_extra="COMPAT=-DNO_MULTIMON TOOLPATH=x86_64-w64-mingw32-";
		if [ "${_dflag:-0}" -eq 1 ]; then
			_makeflags_extra="${_makeflags_extra:+${_makeflags_extra} }XFLAGS=-DWINFRIP_DEBUG";
		fi;
		cd windows;
		if [ "${_cflag:-0}" -eq 1 ]; then
			make -f Makefile.mgw ${_makeflags_extra} clean;
		fi;
		make -f Makefile.mgw ${_makeflags_extra} "${@}";
		;;
	unix)
		cd unix;
		if [ "${_cflag:-0}" -eq 1 ]; then
			make ${_makeflags_extra} clean;
		fi;
		make ${_makeflags_extra} "${@}";
		;;
	*)
		usage; ;;
	esac;
};

set -o errexit -o noglob;
build "${@}";

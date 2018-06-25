#!/bin/sh
# Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
#

usage() {
	echo "usage: ${0} [-c] [-d] [-h] [-m] [-r] [-u] [-U] [[--] make args...]" >&2;
	echo "       -c.......: make [ ... ] clean before build" >&2;
	echo "       -d.......: build w/ XFLAGS=-DWINFRIP_DEBUG" >&2;
	echo "       -h.......: show this screen" >&2;
	echo "       -m.......: select \`mingw' build type" >&2;
	echo "       -r.......: call mkfiles.pl before build" >&2;
	echo "       -u.......: select \`unix_cmd' build type" >&2;
	echo "       -U.......: select \`unix_gtk' build type" >&2;
};

build() {
	local _build_type="" _cflag=0 _dflag=0 _makeflags_extra="" _opt="" _rflag=0;

	while getopts cdhmruU _opt; do
	case "${_opt}" in
	c)	_cflag=1; ;;
	d)	_dflag=1; ;;
	h)	usage; exit 0; ;;
	m)	_build_type="mingw"; ;;
	r)	_rflag=1; ;;
	u)	_build_type="unix_cmd"; ;;
	U)	_build_type="unix_gtk"; ;;
	*)	usage; exit 1; ;;
	esac; done; shift $((${OPTIND}-1));

	if [ "${_rflag:-0}" -eq 1 ]; then
		echo ./mkfiles.pl; ./mkfiles.pl;
	fi;
	case "${_build_type}" in
	mingw)
		_makeflags_extra="COMPAT=-DNO_MULTIMON TOOLPATH=x86_64-w64-mingw32-";
		if [ "${_dflag:-0}" -eq 1 ]; then
			_makeflags_extra="${_makeflags_extra:+${_makeflags_extra} }XFLAGS=-DDEBUG XFLAGS+=-DWINFRIP_DEBUG";
		fi;
		cd windows;
		if [ "${_cflag:-0}" -eq 1 ]; then
			make -f Makefile.mgw ${_makeflags_extra} clean;
		fi;
		make -f Makefile.mgw ${_makeflags_extra} "${@}";
		;;
	unix_cmd)
		_makeflags_extra="";
		if [ "${_dflag:-0}" -eq 1 ]; then
			_makeflags_extra="${_makeflags_extra:+${_makeflags_extra} }XFLAGS=-DDEBUG";
		fi;
		cd unix;
		if [ "${_cflag:-0}" -eq 1 ]; then
			make -f Makefile.ux ${_makeflags_extra} clean;
		fi;
		make -f Makefile.ux ${_makeflags_extra} "${@}";
		;;
	unix_gtk)
		_makeflags_extra="";
		if [ "${_dflag:-0}" -eq 1 ]; then
			_makeflags_extra="${_makeflags_extra:+${_makeflags_extra} }XFLAGS=-DDEBUG";
		fi;
		cd unix;
		if [ "${_cflag:-0}" -eq 1 ]; then
			make -f Makefile.gtk ${_makeflags_extra} clean;
		fi;
		make -f Makefile.gtk ${_makeflags_extra} "${@}";
		;;
	*)
		echo "error: unknown build type \`${_build_type}'" >&2;
		usage; exit 1; ;;
	esac;
};

set -o errexit -o noglob;
build "${@}";

#!/bin/sh
# Copyright (c) 2018 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
#

usage() {
	echo "usage: ${0} [-c] [-d] [-h] [-i] [-m] [-r] [-R] [-u] [-U] [[--] make args...]" >&2;
	echo "       -c.......: make [ ... ] clean before build" >&2;
	echo "       -d.......: select debug (vs. release) build" >&2;
	echo "       -h.......: show this screen" >&2;
	echo "       -i.......: {clean,install} images {pre,post}-build" >&2;
	echo "       -m.......: select \`mingw' build type" >&2;
	echo "       -r.......: call mkfiles.pl before build" >&2;
	echo "       -R.......: create release archive (implies -i)" >&2;
	echo "       -u.......: select \`unix_cmd' build type" >&2;
	echo "       -U.......: select \`unix_gtk' build type" >&2;
};

build() {
	local	_build_target="release" _build_type=""		\
		_cflag=0 _dflag=0 _iflag=0 _rflag=0 _Rflag=0	\
		_install_dname="" _makeflags_extra="" _opt="";

	while getopts cdhimrRuU _opt; do
	case "${_opt}" in
	c)	_cflag=1; ;;
	d)	_dflag=1; _build_target="debug"; ;;
	h)	usage; exit 0; ;;
	i)	_iflag=1; ;;
	m)	_build_type="mingw"; ;;
	r)	_rflag=1; ;;
	R)	_iflag=1; _Rflag=1; ;;
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
			_makeflags_extra="${_makeflags_extra:+${_makeflags_extra} }XFLAGS=-DDEBUG XFLAGS+=-DWINFRIP_DEBUG XFLAGS+=-g3 XFLAGS+=-O0 LDFLAGS+=-g3";
		fi;
		cd windows;
		_install_dname="FySTY-${_build_target}-$(git rev-parse --short HEAD)";
		if ! [ -d "../FySTY/${_install_dname}" ]; then
			mkdir -p "../FySTY/${_install_dname}";
		fi;
		if [ "${_cflag:-0}" -eq 1 ]; then
			make -f Makefile.mgw ${_makeflags_extra} clean;
			if [ "${_iflag:-0}" -eq 1 ]; then
				find "../FySTY/${_install_dname}" -type f -exec rm -f {} \;
			fi;
		fi;
		make -f Makefile.mgw ${_makeflags_extra} "${@}";
		if [ "${_iflag:-0}" -eq 1 ]; then
			find . -maxdepth 1 -name \*.exe -type f -exec cp -a {} "../FySTY/${_install_dname}" \;
		fi;
		if [ "${_Rflag:-0}" -eq 1 ]; then
			if [ -e "../FySTY/${_install_dname}.zip" ]; then
				rm -f "../FySTY/${_install_dname}.zip";
			fi;
			cd ../FySTY; zip -r "${_install_dname}.zip" "${_install_dname}"; cd "${OLDPWD}";
		fi;
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

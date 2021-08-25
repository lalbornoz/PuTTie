#!/bin/sh
# Copyright (c) 2018, 2019, 2020, 2021 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
#

usage() {
	echo "usage: ${0} [-c] [-d] [-h] [-i] [-j jobs] [-R] [[--] cmake args...]" >&2;
	echo "       -c.......: clean cmake(1) cache file(s) and output directory/ies before build" >&2;
	echo "       -d.......: select Debug (vs. Release) build" >&2;
	echo "       -h.......: show this screen" >&2;
	echo "       -i.......: {clean,install} images {pre,post}-build" >&2;
	echo "       -j.......: set cmake(1) max. job count" >&2;
	echo "       -R.......: create release archive (implies -i)" >&2;
};

build() {
	local	_build_type="Release" _cflag=0 _dflag=0 _iflag=0 _jflag=1	\
		_Rflag=0 _install_dname="" _opt="";

	while getopts cdhij:R _opt; do
	case "${_opt}" in
	c)	_cflag=1; ;;
	d)	_dflag=1; _build_type="Debug"; ;;
	h)	usage; exit 0; ;;
	i)	_iflag=1; ;;
	j)	_jflag="${OPTARG}"; ;;
	R)	_iflag=1; _Rflag=1; ;;
	*)	usage; exit 1; ;;
	esac; done; shift $((${OPTIND}-1));

	_install_dname="FySTY-${_build_type}-$(git rev-parse --short HEAD)";
	if ! [ -d "FySTY/${_install_dname}" ]; then
		mkdir -p "FySTY/${_install_dname}";
	fi;

	if [ "${_cflag:-0}" -eq 1 ]; then
		rm -fr	\
			CMakeCache.txt						\
			CMakeFiles/						\
			windows/CMakeCache.txt					\
			windows/CMakeFiles/					\
			;

		if [ "${_iflag:-0}" -eq 1 ]; then
			find "FySTY/${_install_dname}" -type f -exec rm -f {} \;
		fi;
	fi;

	if ! [ -e CMakeCache.txt ]\
	|| ! [ -e CMakeFiles ]\
	|| ! [ -e windows/CMakeCache.txt ]\
	|| ! [ -e windows/CMakeFiles ]; then
		cmake .	\
			-DCMAKE_BUILD_TYPE="${_build_type}"			\
			-DCMAKE_C_FLAGS_DEBUG="-DDEBUG -DWINFRIP_DEBUG -g3 -O0"	\
			-DCMAKE_C_FLAGS_RELEASE="-g0 -O3"			\
			-DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-mingw.cmake"	\
			;
	fi;

	cmake --build . --parallel "${_jflag:-1}" "${@}";

	if [ "${_iflag:-0}" -eq 1 ]; then
		find .	\
			-maxdepth 1						\
			-mindepth 1						\
			-name \*.exe						\
			\( -not -name test\* \)					\
			-type f							\
			-exec cp -a {} "FySTY/${_install_dname}" \;
	fi;

	if [ "${_Rflag:-0}" -eq 1 ]; then
		if [ -e "FySTY/${_install_dname}.zip" ]; then
			rm -f "FySTY/${_install_dname}.zip";
		fi;
		cd FySTY; zip -r "${_install_dname}.zip" "${_install_dname}"; cd "${OLDPWD}";
	fi;
};

set -o errexit -o noglob -o nounset;
export LANG=C LC_ALL=C; build "${@}";

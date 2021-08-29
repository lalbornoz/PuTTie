#!/bin/sh
# Copyright (c) 2018, 2019, 2020, 2021 Lucio Andrés Illanes Albornoz <lucio@lucioillanes.de>
#

# {{{ build_clean($_build_type, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag)
build_clean() {
	local	_build_type="${1}" _cflag="${2}" _dflag="${3}" _iflag="${4}"	\
		_install_dname="${5}" _jflag="${6}" _Rflag="${7}"		\
		_fname="" _IFS0="${IFS:- 	}";

	if [ "${_cflag:-0}" -eq 1 ]; then
		rm -fr	\
			CMakeCache.txt						\
			CMakeFiles/						\
			windows/CMakeCache.txt					\
			windows/CMakeFiles/					\
			FySTY/pcre2@master/CMakeCache.txt			\
			FySTY/pcre2@master/CMakeFiles/				\
			FySTY/pcre2@master/libpcre2-16.a			\
			FySTY/pcre2@master/libpcre2-16d.a			\
			;
		if [ "${_iflag:-0}" -eq 1 ]\
		&& [ -e "FySTY/${_install_dname}" ]; then
			IFS="
";			for _fname in $(find "FySTY/${_install_dname}" -type f); do
				rm -fr "${_fname}";
			done; IFS="${_IFS0}";
		fi;
	fi;
};
# }}}
# {{{ build_configure($_build_type, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag)
build_configure() {
	local	_build_type="${1}" _cflag="${2}" _dflag="${3}" _iflag="${4}"		\
		_install_dname="${5}" _jflag="${6}" _Rflag="${7}";

	if ! [ -e FySTY/pcre2@master/CMakeCache.txt ]\
	|| ! [ -e FySTY/pcre2@master/CMakeFiles/ ]; then
		cd FySTY/pcre2@master;
		cmake .	\
			-DCMAKE_BUILD_TYPE="${_build_type}"				\
			-DCMAKE_C_FLAGS_DEBUG="-DDEBUG -g3 -O0"				\
			-DCMAKE_C_FLAGS_RELEASE="-g0 -O3"				\
			-DCMAKE_TOOLCHAIN_FILE="../../cmake/toolchain-mingw.cmake"	\
											\
			-DBUILD_SHARED_LIBS=OFF						\
			-DBUILD_STATIC_LIBS=ON						\
			-DPCRE2_BUILD_PCRE2_8=OFF					\
			-DPCRE2_BUILD_PCRE2_16=ON					\
			-DPCRE2_BUILD_PCRE2_32=OFF					\
			-DPCRE2_STATIC_PIC=ON						\
			-DPCRE2_DEBUG=OFF						\
			-DPCRE2_DISABLE_PERCENT_ZT=ON					\
			-DPCRE2_EBCDIC=OFF						\
			-DPCRE2_SUPPORT_UNICODE=ON					\
											\
			-DPCRE2_BUILD_PCRE2GREP=OFF					\
			-DPCRE2_BUILD_TESTS=OFF						\
											\
			-DPCRE2_SUPPORT_LIBBZ2=OFF					\
			-DPCRE2_SUPPORT_LIBZ=OFF					\
			-DPCRE2_SUPPORT_LIBEDIT=OFF					\
			-DPCRE2_SUPPORT_LIBREADLINE=OFF					\
			;
		cd "${OLDPWD}";
	fi;
	if ! [ -e CMakeCache.txt ]\
	|| ! [ -e CMakeFiles/ ]\
	|| ! [ -e windows/CMakeCache.txt ]\
	|| ! [ -e windows/CMakeFiles/ ]; then
		cmake .	\
			-DCMAKE_BUILD_TYPE="${_build_type}"				\
			-DCMAKE_C_FLAGS_DEBUG="-DDEBUG -DWINFRIP_DEBUG -g3 -O0"		\
			-DCMAKE_C_FLAGS_RELEASE="-g0 -O3"				\
			-DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-mingw.cmake"		\
			;
	fi;
};
# }}}
# {{{ build_make($_build_type, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag)
build_make() {
	local	_build_type="${1}" _cflag="${2}" _dflag="${3}" _iflag="${4}"\
		_install_dname="${5}" _jflag="${6}" _Rflag="${7}";

	cd FySTY/pcre2@master;
	cmake --build . --parallel "${_jflag:-1}";
	if [ "x${_build_type}" = "xDebug" ]; then
		ln -fs "libpcre2-16d.a" "libpcre2-16.a";
	fi;
	cd "${OLDPWD}";
	cmake --build . --parallel "${_jflag:-1}" "${@}";
};
# }}}
# {{{ build_install($_build_type, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag)
build_install() {
	local	_build_type="${1}" _cflag="${2}" _dflag="${3}" _iflag="${4}"	\
		_install_dname="${5}" _jflag="${6}" _Rflag="${7}" _fname=""	\
		_IFS0="${IFS:- 	}";

	if [ "${_iflag:-0}" -eq 1 ]; then
		if ! [ -d "FySTY/${_install_dname}" ]; then
			mkdir -p "FySTY/${_install_dname}";
		fi;
		IFS="
";		for _fname in $(find .						\
				-maxdepth 1					\
				-mindepth 1					\
				-name \*.exe					\
				\( -not -name test\* \)				\
				-type f); do
			_fname="${_fname#./}";
			cp -a "${_fname}" "FySTY/${_install_dname}";
			stat "FySTY/${_install_dname}/${_fname}";
		done; IFS="${_IFS0}";
	fi;
	if [ "${_Rflag:-0}" -eq 1 ]; then
		if [ -e "FySTY/${_install_dname}.zip" ]; then
			rm -f "FySTY/${_install_dname}.zip";
		fi;
		cd FySTY; zip -r "${_install_dname}.zip" "${_install_dname}"; cd "${OLDPWD}";
		stat "FySTY/${_install_dname}.zip";
	fi;
};
# }}}

buildp_usage() {
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
	h)	buildp_usage; exit 0; ;;
	i)	_iflag=1; ;;
	j)	_jflag="${OPTARG}"; ;;
	R)	_iflag=1; _Rflag=1; ;;
	*)	buildp_usage; exit 1; ;;
	esac; done; shift $((${OPTIND}-1));
	_install_dname="FySTY-${_build_type}-$(git rev-parse --short HEAD)";

	build_clean "${_build_type}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}";
	build_configure "${_build_type}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}";
	build_make "${_build_type}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}";
	build_install "${_build_type}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}";
};

set -o errexit -o noglob -o nounset;
export LANG=C LC_ALL=C; build "${@}";

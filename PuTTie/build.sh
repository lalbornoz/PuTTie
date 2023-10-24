#!/bin/sh
# Copyright (c) 2018, 2019, 2020, 2021, 2022, 2023 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
#

DBG_ADDR="localhost:1234";
DBG_GDBSERVER_FNAME="Z:/usr/share/win64/gdbserver.exe";

# {{{ build_clang_compile_cmds()
build_clang_compile_cmds() {
	local	_args="" _clang_fname=""			\
		_compile_commands_fname=""			\
		_compile_flags_fname="" _dname="" _fname=""	\
		_pname="" _pwd="" _tmp_fname="" _uname_os="";

	_clang_fname="$(which clang)" || return 1;
	_compile_commands_fname="${0%/*}/../compile_commands.json";
	_pwd="${PWD}";
	_uname_os="$(uname -o)" || return 1;
	if [ "${_uname_os}" = "Cygwin" ]; then
		_pwd="$(cygpath -m "${_pwd}")" || return 1;
	fi;
	_tmp_fname="$(mktemp)" || return 1;
	trap "rm -f \"${_tmp_fname}\" 2>/dev/null" HUP INT TERM USR1 USR2;

	_compile_flags_fname="${0%/*}/compile_flags.txt.tmpl.${_uname_os##*/}";

	_args="$(
		sed						\
			-e 's/^/"/'				\
			-e 's/$/",/'				\
			"${_compile_flags_fname}"		|\
		paste -sd " "					|\
		sed 's/,$//')";
	_args='["'"${_clang_fname}"'", '"${_args}"', "-c", "-o"';

	for _pname in $(					\
		cd "${0%/*}/.." && find .			\
			-not -path "./PuTTie/pcre2/\*"		\
			-not -path "./PuTTie/PuTTie-\*-\*/\*"	\
			-not -path "\*/CMake\*/\*"		\
			-iname \*.c);
	do
		_pname="${_pname#./}";
		_fname="${_pname##*/}"; _dname="${_pname%/*}";
		[ "${_dname}" = "${_pname}" ] && _dname="";

		printf '
	{
		"directory": "%s",
		"arguments": %s, "%s.o", "%s"],
		"file": "%s%s"
	},'							\
			"${_pwd}"				\
			"${_args}" "${_fname%.c}" "${_fname}"	\
			"${_dname:+${_dname}/}" "${_fname}";
	done > "${_tmp_fname}";

	sed	-i""						\
		-e '$s/},$/}/'					\
		-e '1i\
['								\
		-e '1d'						\
		-e '$a\
]'		"${_tmp_fname}";

	mv -i "${_tmp_fname}" "${_compile_commands_fname}";
	if [ -e "${_tmp_fname}" ]; then
		rm -f "${_tmp_fname}" 2>/dev/null;
	fi;
	trap - HUP INT TERM USR1 USR2;
};
# }}}
# {{{ build_dbg_svr($_addr, $_gdbserver_fname, $_exe_fname, ...)
build_dbg_svr() {
	local _addr="${1}" _gdbserver_fname="${2}" _exe_fname="${3}"; shift 3;

	wine	\
		"${_gdbserver_fname}"			\
		"${_addr}"				\
		"${_exe_fname}"			\
		"${@}";
	return "${?}";
};
# }}}
# {{{ build_dbg_cli($_addr, $_exe_fname)
build_dbg_cli() {
	local _addr="${1}" _exe_fname="${2}";

	x86_64-w64-mingw32-gdb				\
		-ex "set pagination off"		\
		-ex "target remote ${_addr}"		\
		"${_exe_fname}";
	return "${?}";
};
# }}}

# {{{ build_clean($_build_type, $_Bflag, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag, $_tflag)
build_clean() {
	local	_build_type="${1}" _Bflag="${2}" _cflag="${3}" _dflag="${4}"	\
		_iflag="${5}" _install_dname="${6}" _jflag="${7}" _Rflag="${8}"	\
		_tflag="${9}"							\
		_fname="" _IFS0="${IFS:- 	}";

	if [ "${_cflag:-0}" -eq 1 ]; then
		rm -fr	\
			CMakeCache.txt			\
			CMakeFiles/			\
			windows/CMakeCache.txt		\
			windows/CMakeFiles/		\
			PuTTie/pcre2/CMakeCache.txt	\
			PuTTie/pcre2/CMakeFiles/	\
			PuTTie/pcre2/libpcre2-16.a	\
			PuTTie/pcre2/libpcre2-16d.a	\
			;
		if [ "${_iflag:-0}" -eq 1 ]\
		&& [ -e "PuTTie/${_install_dname}" ]; then
			IFS="
";			for _fname in $(find "PuTTie/${_install_dname}" -type f); do
				rm -fr "${_fname}";
			done; IFS="${_IFS0}";
		fi;
	fi;
};
# }}}
# {{{ build_configure($_build_type, $_Bflag, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag, $_tflag)
build_configure() {
	local	_build_type="${1}" _Bflag="${2}" _cflag="${3}" _dflag="${4}"	\
		_iflag="${5}" _install_dname="${6}" _jflag="${7}" _Rflag="${8}"	\
		_tflag="${9}"							\
		_git_branch="" _git_commit="" _git_pcre2_commit="pcre2-10.42"	\
		_version_ssh="" _version_text="";

	_git_branch="$(git branch --show-current)" || exit 2;
	_git_commit="$(git rev-parse --short HEAD)" || exit 2;
	_version_text="PuTTie ${_build_type} build (Git commit ${_git_commit} on branch ${_git_branch})";

	if ! [ -e PuTTie/pcre2/CMakeCache.txt ]\
	|| ! [ -e PuTTie/pcre2/CMakeFiles/ ]; then
		cd PuTTie/pcre2;
		git checkout "${_git_pcre2_commit}" || exit 2;
		"${CMAKE}" . \
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
		"${CMAKE}" . \
			-DBUILD_SSH_VERSION="${_version_ssh}"				\
			-DBUILD_TEXT_VERSION="${_version_text}"				\
			-DCMAKE_BUILD_TYPE="${_build_type}"				\
			-DCMAKE_C_FLAGS_DEBUG="-DDEBUG -DWINFRIP_DEBUG -g3 -O0"		\
			-DCMAKE_C_FLAGS_RELEASE="-g0 -O3"				\
			-DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-mingw.cmake"		\
			-DDEFAULT_STORAGE_BACKEND="${_Bflag}"				\
			;
	fi;
};
# }}}
# {{{ build_make($_build_type, $_Bflag, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag, $_tflag)
build_make() {
	local	_build_type="${1}" _Bflag="${2}" _cflag="${3}" _dflag="${4}"	\
		_iflag="${5}" _install_dname="${6}" _jflag="${7}" _Rflag="${8}"	\
		_tflag="${9}";

	cd PuTTie/pcre2;
	"${CMAKE}" --build . --parallel "${_jflag:-1}";
	if [ "x${_build_type}" = "xDebug" ]; then
		ln -fs "libpcre2-16d.a" "libpcre2-16.a";
	fi;
	cd "${OLDPWD}";
	"${CMAKE}" --build . --parallel "${_jflag:-1}" ${_tflag:+--target "${_tflag}"};
};
# }}}
# {{{ build_install($_build_type, $_Bflag, $_cflag, $_dflag, $_iflag, $_install_dname, $_jflag, $_Rflag, $_tflag)
build_install() {
	local	_build_type="${1}" _Bflag="${2}" _cflag="${3}" _dflag="${4}"	\
		_iflag="${5}" _install_dname="${6}" _jflag="${7}" _Rflag="${8}"	\
		_tflag="${9}"							\
		_fname="" _IFS0="${IFS:- 	}";

	if [ "${_iflag:-0}" -eq 1 ]; then
		if ! [ -d "PuTTie/${_install_dname}" ]; then
			mkdir -p "PuTTie/${_install_dname}";
		fi;
		IFS="
";		for _fname in $(find .					\
				-maxdepth 1				\
				-mindepth 1				\
				-iname \*.exe				\
				\( -not -iname \*test\* \)		\
				\( -not -iname bidi_gettype.exe \)	\
				-type f);
		do
			_fname="${_fname#./}";
			cp -a "${_fname}" "PuTTie/${_install_dname}";
			stat "PuTTie/${_install_dname}/${_fname}";
		done; IFS="${_IFS0}";
		cp -a "PuTTie/create_shortcut.exe" "PuTTie/${_install_dname}";
		stat "PuTTie/${_install_dname}/create_shortcut.exe";
		cp -a "PuTTie/README.md" "PuTTie/${_install_dname}";
		stat "PuTTie/${_install_dname}/README.md";
	fi;
	if [ "${_Rflag:-0}" -eq 1 ]; then
		if [ -e "PuTTie/${_install_dname}.zip" ]; then
			rm -f "PuTTie/${_install_dname}.zip";
		fi;
		cd PuTTie; zip -r "${_install_dname}.zip" "${_install_dname}"; cd "${OLDPWD}";
		stat "PuTTie/${_install_dname}.zip";
	fi;
};
# }}}

buildp_usage() {
	echo "usage: ${0} [-B <backend>] [-c] [--clang] [-d] [--dbg-svr <fname> [..]] [--dbg-cli <fname>] [-h] [-i] [-j <jobs>] [-R] [-t <target>]" >&2;
	echo "       -B <backend>..........: set default storage backend to either of ephemeral, file, or registry (default)" >&2;
	echo "       -c....................: clean cmake(1) cache file(s) and output directory/ies before build" >&2;
	echo "       --clang...............: regenerate compile_commands.json" >&2;
	echo "       -d....................: select Debug (vs. Release) build (NB: pass -c when switching between build types)" >&2;
	echo "       --dbg-svr <fname> [..]: run <fname> w/ optional arguments via wine & local gdbserver on port 1234" >&2;
	echo "       --dbg-cli <fname>.....: debug <fname> w/ MingW gdb previously launched w/ --dbg-svr" >&2;
	echo "       -h....................: show this screen" >&2;
	echo "       -i....................: {clean,install} images {pre,post}-build" >&2;
	echo "       -j <jobs>.............: set cmake(1) max. job count" >&2;
	echo "       -R....................: create release archive (implies -i)" >&2;
	echo "       -t <target>...........: build PuTTY <target> instead of default target" >&2;
};

build() {
	local	_build_type="Release" _Bflag="registry" _cflag=0 _clangflag=0	\
		_dflag=0 _dbgflag=0 _iflag=0 _jflag=1 _Rflag=0 _tflag=""	\
		_install_dname="" _opt=""					\
		CMAKE="${CMAKE:-cmake}" OPTIND=1;

	while [ "${#}" -gt 0 ]; do
		case "${1}" in
		--clang)
			_clangflag=1; shift 1;
			;;
		--dbg-svr)
			_dbgflag=1; shift 1;
			if [ "${#}" -lt 1 ]; then
				printf "error: missing <fname> argument to --dbg-svr\n" >&2;
				buildp_usage; exit 1;
			fi;
			;;
		--dbg-cli)
			_dbgflag=2; shift 1;
			if [ "${#}" -lt 1 ]; then
				printf "error: missing <fname> argument to --dbg-cli\n" >&2;
				buildp_usage; exit 1;
			fi;
			;;
		*)	if getopts B:cdhij:Rt: _opt; then
				case "${_opt}" in
				B)	_Bflag="${OPTARG}"; ;;
				c)	_cflag=1; ;;
				d)	_dflag=1; _build_type="Debug"; ;;
				h)	buildp_usage; exit 0; ;;
				i)	_iflag=1; ;;
				j)	_jflag="${OPTARG}"; ;;
				R)	_iflag=1; _Rflag=1; ;;
				t)	_tflag="${OPTARG}"; ;;
				*)	buildp_usage; exit 1; ;;
				esac;
			else
				break;
			fi; ;;
		esac;
	done; shift $((${OPTIND}-1));

	_install_dname="PuTTie-${_Bflag}-${_build_type}-$(git rev-parse --short HEAD)";
	_Bflag="WFS_BACKEND_$(printf "${_Bflag}" | tr a-z A-Z)" || exit 2;

	if [ "$(uname -o 2>/dev/null)" = "Cygwin" ]; then
		export CMAKE="/usr/bin/cmake";
	fi;

	case "${_clangflag}" in
	1)	build_clang_compile_cmds; exit "${?}"; ;;
	esac;

	case "${_dbgflag}" in
	1)	build_dbg_svr "${DBG_ADDR}" "${DBG_GDBSERVER_FNAME}" "${@}"; exit "${?}"; ;;
	2)	build_dbg_cli "${DBG_ADDR}" "${@}"; exit "${?}"; ;;
	esac;

	build_clean "${_build_type}" "${_Bflag}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}" "${_tflag}";
	build_configure "${_build_type}" "${_Bflag}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}" "${_tflag}";
	build_make "${_build_type}" "${_Bflag}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}" "${_tflag}";
	build_install "${_build_type}" "${_Bflag}" "${_cflag}" "${_dflag}" "${_iflag}" "${_install_dname}" "${_jflag}" "${_Rflag}" "${_tflag}";
};

set -o errexit -o noglob -o nounset;
export LANG=C LC_ALL=C; build "${@}";

# vim:fdm=marker sw=8 ts=8 tw=0

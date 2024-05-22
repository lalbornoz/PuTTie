#!/bin/sh
# Copyright (c) 2018, 2019, 2020, 2021, 2022, 2023, 2024 Lucía Andrea Illanes Albornoz <lucia@luciaillanes.de>
#

# {{{ dict_get($_dict, $_key, $_rvalue)
dict_get() {
	local _dg_dict="${1}" _dg_key="${2}" _dg_rvalue="${3#\$}";
	eval ${_dg_rvalue}=\${${_dg_dict}_${_dg_key}};
};
# }}}
# {{{ dict_set($_dict, $_key, $_value)
dict_set() {
	local _ds_dict="${1}" _ds_key="${2}" _ds_value="${3}";
	eval ${_ds_dict}_${_ds_key}=\${_ds_value};
};
# }}}
# {{{ dict_test_int($_dict, $_key, $_test_expr)
dict_test_int() {
	local	_dti_dict="${1}" _dti_key="${2}"	\
		_dti_value="";
	shift 2;

	eval _dti_value=\${${_dti_dict}_${_dti_key}};
	eval [ \"\${_dti_value}\" \"\${@}\" ];
};
# }}}

# {{{ build_clang_compile_cmds($_dict)
build_clang_compile_cmds() {
	local	_bccc_dict="${1}"					\
		_bccc_args="" _clang_fname=""				\
		_bccc_compile_commands_fname=""				\
		_bccc_compile_flags_fname="" _dname="" _fname=""	\
		_bccc_pname="" _pwd="" _tmp_fname="" _uname_os="";
	shift 1;

	_bccc_clang_fname="$(which clang)" || return "${?}";
	_bccc_compile_commands_fname="${0%/*}/../compile_commands.json";
	_bccc_pwd="${PWD}";
	_bccc_uname_os="$(uname -o)" || return "${?}";
	if [ "${_bccc_uname_os}" = "Cygwin" ]; then
		_bccc_pwd="$(cygpath -m "${_bccc_pwd}")" || return "${?}";
	fi;
	_bccc_tmp_fname="$(mktemp)" || return "${?}";
	trap "rm -f \"${_bccc_tmp_fname}\" 2>/dev/null" EXIT HUP INT TERM USR1 USR2;

	_bccc_compile_flags_fname="${0%/*}/compile_flags.txt.tmpl.${_bccc_uname_os##*/}";

	_bccc_args="$(
		sed						\
			-e 's/^/"/'				\
			-e 's/$/",/'				\
			"${_bccc_compile_flags_fname}"		|\
		paste -sd " "					|\
		sed 's/,$//')" || return "${?}";
	_bccc_args='["'"${_bccc_clang_fname}"'", '"${_bccc_args}"', "-c", "-o"';

	for _bccc_pname in $(					\
		cd "${0%/*}/.." && find .			\
			-not -path "./PuTTie/pcre2/\*"		\
			-not -path "./PuTTie/PuTTie-\*-\*/\*"	\
			-not -path "\*/CMake\*/\*"		\
			-iname \*.c);
	do
		_bccc_pname="${_bccc_pname#./}";
		_bccc_fname="${_bccc_pname##*/}";
		_bccc_dname="${_bccc_pname%/*}";
		[ "${_bccc_dname}" = "${_bccc_pname}" ] && _bccc_dname="";

		printf '
	{
		"directory": "%s",
		"arguments": %s, "%s.o", "%s"],
		"file": "%s%s"
	},'							\
			"${_bccc_pwd}"				\
			"${_bccc_args}" "${_bccc_fname%.c}" "${_bccc_fname}"	\
			"${_bccc_dname:+${_bccc_dname}/}" "${_bccc_fname}";
	done > "${_bccc_tmp_fname}" || return "${?}";

	sed	-i""						\
		-e '$s/},$/}/'					\
		-e '1i\
['								\
		-e '1d'						\
		-e '$a\
]'		"${_tmp_fname}" || return "${?}";

	mv -i "${_tmp_fname}" "${_compile_commands_fname}" || return "${?}";

	if [ -e "${_tmp_fname}" ]; then
		rm -f "${_tmp_fname}";
	fi;
	trap - EXIT HUP INT TERM USR1 USR2;

	return 0;
};
# }}}
# {{{ build_clean($_dict)
build_clean() {
	make clean;
};
# }}}
# {{{ build_clean_all($_dict)
build_clean_all() {
	local _bca_dict="${1}";
	build_clean_pcre2 "${_bca_dict}" || return "${?}";
	build_clean "${_bca_dict}" || return "${?}";
	return 0;
};
# }}}
# {{{ build_clean_pcre2($_dict)
build_clean_pcre2() {
	(cd PuTTie/pcre2 || return "${?}";
	 make clean) || return "${?}";
	return 0;
};
# }}}
# {{{ build_configure($_dict)
build_configure() {
	local	_bc_dict="${1}"				\
		_bc_backend_define="" _bc_build_type=""	\
		_bc_mingw_debug_build=""		\
		_bc_version_ssh="" _bc_version_text="";
	shift 1;

	if [ -e CMakeCache.txt ]\
	|| [ -e CMakeFiles/ ]\
	|| [ -e windows/CMakeCache.txt ]\
	|| [ -e windows/CMakeFiles/ ];
	then
		return 0;
	fi;

	dict_get "${_bc_dict}" "backend_define" \$_bc_backend_define;
	dict_get "${_bc_dict}" "build_type" \$_bc_build_type;
	dict_get "${_bc_dict}" "version_ssh" \$_bc_version_ssh;
	dict_get "${_bc_dict}" "version_text" \$_bc_version_text;

	if dict_test_int "${_bc_dict}" "mingw_debug_build" -eq 1; then
		_bc_mingw_debug_build="-DWINFRIP_DEBUG_NOCONSOLE=1";
	fi;

	"${CMAKE}" . \
		-DBUILD_SSH_VERSION="${_bc_version_ssh}"			\
		-DBUILD_TEXT_VERSION="${_bc_version_text}"			\
		-DCMAKE_BUILD_TYPE="${_bc_build_type}"				\
		-DCMAKE_C_FLAGS_DEBUG="-DDEBUG -DWINFRIP_DEBUG -g3 -O0"		\
		-DCMAKE_C_FLAGS_RELEASE="-g0 -O3"				\
		-DCMAKE_TOOLCHAIN_FILE="cmake/toolchain-mingw.cmake"		\
		-DDEFAULT_STORAGE_BACKEND="${_bc_backend_define}"		\
		${_bc_mingw_debug_build}					\
		|| return "${?}";

	return 0;
};
# }}}
# {{{ build_configure_pcre2($_dict)
build_configure_pcre2() {
	local	_bcp_dict="${1}"	\
		_bcp_build_type="" _bcp_git_tag_pcre2="";
	shift 1;

	if [ -e PuTTie/pcre2/CMakeCache.txt ]\
	|| [ -e PuTTie/pcre2/CMakeFiles/ ]; then
		return 0;
	fi;

	dict_get "${_bcp_dict}" "build_type" \$_bcp_build_type;
	dict_get "${_bcp_dict}" "git_tag_pcre2" \$_bcp_git_tag_pcre2;

	(cd PuTTie/pcre2 || return "${?}";
	 git checkout "${_bcp_git_tag_pcre2}" || return "${?}";

	 "${CMAKE}"								\
		 . 								\
		-DCMAKE_BUILD_TYPE="${_bcp_build_type}"				\
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
		|| return "${?}") || return "${?}";

	return 0;
};
# }}}
# {{{ build_dbg_client($_dict, [...])
build_dbg_client() {
	local	_bdc_dict="${1}"	\
		_bdc_dbg_addr="" _bdc_dbg_exe_fname="";
	shift 1;

	dict_get "${_bdc_dict}" "dbg_addr"	\$_bdc_dbg_addr;
	dict_get "${_bdc_dict}" "dbg_exe_fname"	\$_bdc_dbg_exe_fname;

	x86_64-w64-mingw32-gdb				\
		-ex "set pagination off"		\
		-ex "target remote ${_bdc_dbg_addr}"	\
		"${_bdc_dbg_exe_fname}"			\
		"${@}" || return "${?}";

	return 0;
};
# }}}
# {{{ build_dbg_server($_dict, [...])
build_dbg_server() {
	local	_bds_dict="${1}"	\
		_bds_dbg_addr="" _bds_dbg_gdbserver_fname="" _bds_dbg_exe_fname="";
	shift 1;

	dict_get "${_bds_dict}" "dbg_addr"		\$_bds_dbg_addr;
	dict_get "${_bds_dict}" "dbg_gdbserver_fname"	\$_bds_dbg_gdbserver_fname;
	dict_get "${_bds_dict}" "dbg_exe_fname"		\$_bds_dbg_exe_fname;

	wine					\
		"${_bds_dbg_gdbserver_fname}"	\
		"${_bds_dbg_addr}"		\
		"${_bds_dbg_exe_fname}"		\
		"${@}" || return "${?}";

	return 0;
};
# }}}
# {{{ build_distclean($_dict)
build_distclean() {
	rm -fr	\
		CMakeCache.txt			\
		CMakeFiles/			\
		windows/CMakeCache.txt		\
		windows/CMakeFiles/		\
		;
};
# }}}
# {{{ build_distclean_al($_dict)
build_distclean_all() {
	local _bda_dict="${1}";
	build_distclean_pcre2 "${_bda_dict}" || return "${?}";
	build_distclean "${_bda_dict}" || return "${?}";
	return 0;
};
# }}}
# {{{ build_distclean_pcre2($_dict)
build_distclean_pcre2() {
	rm -fr	\
		PuTTie/pcre2/CMakeCache.txt	\
		PuTTie/pcre2/CMakeFiles/	\
		PuTTie/pcre2/libpcre2-16.a	\
		PuTTie/pcre2/libpcre2-16d.a	\
		;
};
# }}}
# {{{ build_make($_dict)
build_make_depends="build_configure build_make_pcre2";
build_make() {
	local	_bm_dict="${1}"	\
		_bm_build_type="" _bm_jobs_count=0 _bm_target="";
	shift 1;

	dict_get "${_bm_dict}" "build_type"	\$_bm_build_type;
	dict_get "${_bm_dict}" "jobs_count"	\$_bm_jobs_count;
	dict_get "${_bm_dict}" "target"		\$_bm_target;

	"${CMAKE}"				\
		--build .			\
		--parallel "${_bm_jobs_count}"	\
		${_bm_target:+--target "${_bm_target}"} || return "${?}";

	return 0;
};
# }}}
# {{{ build_make_pcre2($_dict)
build_make_pcre2_depends="build_configure_pcre2";
build_make_pcre2() {
	local	_bmp_dict="${1}"	\
		_bmp_build_type="" _bmp_jobs_count="";
	shift 1;

	dict_get "${_bmp_dict}" "build_type"	\$_bmp_build_type;
	dict_get "${_bmp_dict}" "jobs_count"	\$_bmp_jobs_count;

	(cd PuTTie/pcre2 || return "${?}";
	 "${CMAKE}" --build . --parallel "${_bmp_jobs_count}" || return "${?}";
	 if [ "${_bmp_build_type}" = "Debug" ]; then
		ln -fs "libpcre2-16d.a" "libpcre2-16.a" || return "${?}";
	 fi) || return "${?}";

	return 0;
};
# }}}
# {{{ build_install($_dict)
build_install_depends="build_make";
build_install() {
	local	_bi_dict="${1}"				\
		_bi_fname="" _bi_release_name=""	\
		_bi_IFS0="${IFS:- 	}";
	shift 1;

	dict_get "${_bi_dict}" "release_name" \$_bi_release_name;

	if [ -d "PuTTie/${_bi_release_name}" ]; then
		rm -fr "PuTTie/${_bi_release_name}" || return "${?}";
	fi;
	mkdir -p "PuTTie/${_bi_release_name}" || return "${?}";

	IFS="
";	for _bi_fname in $(find .				\
			-maxdepth 1				\
			-mindepth 1				\
			-iname \*.exe				\
			\( -not -iname \*test\* \)		\
			\( -not -iname bidi_gettype.exe \)	\
			-type f);
	do
		_bi_fname="${_bi_fname#./}";
		if [ "${_bi_fname}" = "putty.exe" ]; then
			cp -a "${_bi_fname}" "PuTTie/${_bi_release_name}/puttie.exe" || return "${?}";
			stat "PuTTie/${_bi_release_name}/puttie.exe" || return "${?}";
		else
			cp -a "${_bi_fname}" "PuTTie/${_bi_release_name}" || return "${?}";
			stat "PuTTie/${_bi_release_name}/${_bi_fname}" || return "${?}";
		fi;
	done; IFS="${_bi_IFS0}";

	cp -a "PuTTie/create_shortcut.exe" "PuTTie/${_bi_release_name}" || return "${?}";
	stat "PuTTie/${_bi_release_name}/create_shortcut.exe" || return "${?}";

	cp -a "PuTTie/README.md" "PuTTie/${_bi_release_name}" || return "${?}";
	stat "PuTTie/${_bi_release_name}/README.md" || return "${?}";

	ln -s "puttie.exe" "PuTTie/${_bi_release_name}/puttie-portable.exe" || return "${?}";
	stat "PuTTie/${_bi_release_name}/puttie-portable.exe" || return "${?}";

	if [ -e "PuTTie/${_bi_release_name}.zip" ]; then
		rm -f "PuTTie/${_bi_release_name}.zip" || return "${?}";
	fi;
	(cd PuTTie || return "${?}";
	 zip					\
		-r "${_bi_release_name}.zip"	\
		"${_bi_release_name}" || return "${?}") || return "${?}";
	stat "PuTTie/${_bi_release_name}.zip" || return "${?}";

	return 0;
};
# }}}
# {{{ build_publish($_dict)
build_publish_depends="build_install";
build_publish() {
	local	_bp_dict="${1}"									\
		_bp_backend="" _bp_backend_descr="" _bp_body_tmp_fname="" _bp_build_type=""	\
		_bp_git_commit="" _bp_git_commit_upstream="" _bp_git_tag_pcre2=""		\
		_bp_github_rest_assets=""  _bp_github_rest_releases="" _bp_github_token=""	\
		_bp_release_id="" _bp_release_name="" _bp_response_json="";
	shift 1;

	dict_get "${_bp_dict}" "backend"		\$_bp_backend;
	dict_get "${_bp_dict}" "build_type"		\$_bp_build_type;
	dict_get "${_bp_dict}" "git_commit"		\$_bp_git_commit;
	dict_get "${_bp_dict}" "git_commit_upstream"	\$_bp_git_commit_upstream;
	dict_get "${_bp_dict}" "git_tag_pcre2"		\$_bp_git_tag_pcre2;
	dict_get "${_bp_dict}" "github_rest_assets"	\$_bp_github_rest_assets;
	dict_get "${_bp_dict}" "github_rest_releases"	\$_bp_github_rest_releases;
	dict_get "${_bp_dict}" "github_token"		\$_bp_github_token;
	dict_get "${_bp_dict}" "release_name"		\$_bp_release_name;

	if ! [ -e "PuTTie/${_bp_release_name}.zip" ]; then
		return 1;
	fi;

	case "${_bp_backend}" in
	file)		_bp_backend_descr="file-based"; ;;
	registry)	_bp_backend_descr="Registry-based"; ;;
	ephemeral)	_bp_backend_descr="ephemeral"; ;;
	*)		return 1; ;;
	esac;

	_bp_body_tmp_fname="$(mktemp)" || return "${?}";
	trap "rm -f \"${_bp_body_tmp_fname}\" 2>/dev/null" EXIT HUP INT TERM USR1 USR2;
	cat >"${_bp_body_tmp_fname}" <<EOF
${_bp_release_name}

PuTTie ${_bp_build_type} build ${_bp_git_commit}
Defaults to ${_bp_backend_descr} global options, host CA, host key, jump list, Pageant private key list, and session storage
Select portable file backend with puttie-portable.exe.

Changes:


Upstream at ${_bp_git_commit_upstream}
pcre2 at ${_bp_git_tag_pcre2}
EOF
	"${EDITOR}" "${_bp_body_tmp_fname}" || return "${?}";

	_bp_response_json="$(curl				\
		-L						\
		-X POST						\
		-H "Accept: application/vnd.github+json"	\
		-H "Authorization: Bearer ${_bp_github_token}"	\
		-H "X-GitHub-Api-Version: 2022-11-28"		\
		"${_bp_github_rest_releases}"			\
		-d '{
			"body":'"$(json_xs -f string < "${_bp_body_tmp_fname}")"',
			"draft":false,
			"make_latest":"true",
			"name":"'"${_bp_release_name}"'",
			"prerelease":false,
			"tag_name":"'"${_bp_release_name}"'",
			"target_commitish":"master",
			"generate_release_notes":false
		}')" || return "${?}";

	printf "%s\n" "${_bp_response_json}";

	_bp_release_id="$(
		printf "%s\n" "${_bp_response_json}"							|\
		perl -wle										 \
			'local $/ = undef; use JSON qw(from_json); print(from_json(<>)->{"id"})')"	 \
				|| return "${?}";

	curl	\
		-L											\
		-X POST											\
		-H "Accept: application/vnd.github+json"						\
		-H "Authorization: Bearer ${_bp_github_token}"						\
		-H "X-GitHub-Api-Version: 2022-11-28"							\
		-H "Content-Type: application/octet-stream"						\
		"${_bp_github_rest_assets%/}/${_bp_release_id}/assets?name=${_bp_release_name}.zip"	\
		--data-binary "@PuTTie/${_bp_release_name}.zip" || return "${?}";

	rm -f "${_bp_body_tmp_fname}";
	trap - EXIT HUP INT TERM USR1 USR2;

	return 0;
};
# }}}

# {{{ buildp_check_deps([...]) {
buildp_check_deps() {
	local _bpcd_cmd_name="" _bpcd_cmd_names_missing="";

	for _bpcd_cmd_name in "${@}"; do
		if ! which "${_bpcd_cmd_name}" >/dev/null 2>&1; then
			_cmd_names_missing="${_bpcd_cmd_names_missing:+${_bpcd_cmd_names_missing} }${_bpcd_cmd_name}";
		fi;
	done;

	if [ "${_bpcd_cmd_names_missing:+1}" = 1 ]; then
		printf "Error: missing dependencies: %s\n" "${_bpcd_cmd_names_missing}" >&2;
		return 1;
	else
		return 0;
	fi;
};
# }}}
# {{{ buildp_dict_from_args($_dict, $_rshift_count)
buildp_dict_from_args() {
	local	_bpdfa_dict="${1}" _bpdfa_rshift_count="${2#\$}"	\
		_bpdfa_backend="" _bpdfa_build_type=""			\
		_bpdfa_git_branch="" _bpdfa_git_commit=""		\
		_bpdfa_ignore_deps=0					\
		_bpdfa_jobs_count=0					\
		_bpdfa_mingw_debug_build=0				\
		_bpdfa_opt="" _bpdfa_target=""				\
		OPTARG="" OPTIND=1;
	shift 2;

	_bpdfa_backend="registry";
	_bpdfa_build_type="Release";
	_bpdfa_git_branch="$(git branch --show-current)" || return "${?}";
	_bpdfa_git_commit="$(git rev-parse --short HEAD)" || return "${?}";
	_bpdfa_jobs_count=1;

	if [ "$(uname -s 2>/dev/null)" = "Linux" ]; then
		_bpdfa_jobs_count="$(awk '/^processor/' /proc/cpuinfo | wc -l)" || return "${?}";
	fi;

	while getopts B:dDhij:t: _bpdfa_opt; do
	case "${_bpdfa_opt}" in
	B)	_bpdfa_backend="${OPTARG}"; ;;
	d)	_bpdfa_build_type="Debug"; ;;
	D)	_bpdfa_build_type="Debug";
		_bpdfa_mingw_debug_build=1; ;;
	h)	buildp_usage; exit 0; ;;
	i)	_bpdfa_ignore_deps=1; ;;
	j)	_bpdfa_jobs_count="${OPTARG}"; ;;
	t)	_bpdfa_target="${OPTARG}"; ;;
	*)	buildp_usage; exit 1; ;;
	esac; done;
	shift $((${OPTIND}-1));
	eval ${_bpdfa_rshift_count}=$((${OPTIND}-1));

	dict_set "${_bpdfa_dict}" "backend"			"${_bpdfa_backend}";
	dict_set "${_bpdfa_dict}" "backend_define"		"WFS_BACKEND_$(printf "${_bpdfa_backend}" | tr a-z A-Z)" || return "${?}";
	dict_set "${_bpdfa_dict}" "build_type"			"${_bpdfa_build_type}";
	dict_set "${_bpdfa_dict}" "dbg_addr"			"localhost:1234";
	dict_set "${_bpdfa_dict}" "dbg_exe_fname"		"./putty.exe";
	dict_set "${_bpdfa_dict}" "dbg_gdbserver_fname"		"Z:/usr/share/win64/gdbserver.exe";
	dict_set "${_bpdfa_dict}" "git_branch"			"${_bpdfa_git_branch}";
	dict_set "${_bpdfa_dict}" "git_commit"			"${_bpdfa_git_commit}";
	dict_set "${_bpdfa_dict}" "git_commit_upstream"		"$(git rev-parse --short upstream/main)" || return "${?}";
	dict_set "${_bpdfa_dict}" "git_tag_pcre2"		"pcre2-10.42";
	dict_set "${_bpdfa_dict}" "github_rest_releases"	"https://api.github.com/repos/lalbornoz/PuTTie/releases";
	dict_set "${_bpdfa_dict}" "github_rest_assets"		"https://uploads.github.com/repos/lalbornoz/PuTTie/releases/";
	dict_set "${_bpdfa_dict}" "github_token"		"$(cat "PuTTie/.build.github.token")" || return "${?}";
	dict_set "${_bpdfa_dict}" "ignore_deps"			"${_bpdfa_ignore_deps}";
	dict_set "${_bpdfa_dict}" "jobs_count"			"${_bpdfa_jobs_count}";
	dict_set "${_bpdfa_dict}" "mingw_debug_build"		"${_bpdfa_mingw_debug_build}";
	dict_set "${_bpdfa_dict}" "release_name"		"PuTTie-${_bpdfa_backend}-${_bpdfa_build_type}-${_bpdfa_git_commit}"
	dict_set "${_bpdfa_dict}" "target"			"${_bpdfa_target}";
	dict_set "${_bpdfa_dict}" "version_ssh"			"";
	dict_set "${_bpdfa_dict}" "version_text"		"PuTTie ${_bpdfa_build_type} build (Git commit ${_bpdfa_git_commit} on branch ${_bpdfa_git_branch})";

	return 0;
};
# }}}
# {{{ buildp_exec($_dict, [...])
buildp_exec() {
	local	_bpe_dict="${1}"					\
		_bpe_and_found="" _bpe_and_idx=0			\
		_bpe_fn_cmd_line="" _bpe_fn_name="" _bpe_fn_rc=0	\
		_bpe_idx=0;
	shift 1;

	while [ "${#}" -ge 1 ]; do
		_bpe_fn_name="${1}"; shift 1;
		_bpe_and_found=0; _bpe_and_idx=1;

		while [ "${_bpe_and_idx}" -le "${#}" ]; do
			if eval [ \"\${${_bpe_and_idx}}\" = \"and\" ]; then
				_bpe_and_found=1; break;
			else
				: $((_bpe_and_idx+=1));
			fi;
		done;

		if [ "${_bpe_and_found}" -eq 0 ]; then
			_bpe_and_idx="$((${#} + 1))";
		fi;

		_bpe_fn_name="build_${_bpe_fn_name}";
		if command -v "${_bpe_fn_name}" >/dev/null; then
			if dict_test_int "${_bpe_dict}" "ignore_deps" -eq 0; then
				buildp_exec_depends "${_bpe_dict}" "${_bpe_fn_name}" || return "${?}";
			fi;

			_bpe_fn_cmd_line='${_bpe_fn_name} ${_bpe_dict}';
			_bpe_idx=1;
			while [ "${_bpe_idx}" -lt "${_bpe_and_idx}" ]; do
				_bpe_fn_cmd_line="${_bpe_fn_cmd_line:+${_bpe_fn_cmd_line} }\"\${${_bpe_idx}}\"";
				: $((_bpe_idx+=1));
			done;
			eval ${_bpe_fn_cmd_line}; _bpe_fn_rc="${?}";
		else
			return 1;
		fi;

		if [ "${_bpe_and_found}" -eq 1 ]; then
			shift "${_bpe_and_idx}";
		else
			shift "${#}";
		fi;

		if [ "${_bpe_fn_rc}" -ne 0 ]; then
			return "${_bpe_fn_rc}";
		fi;
	done;

	return 0;
};
# }}}
# {{{ buildp_exec_depends($_dict, $_fn_name)
buildp_exec_depends() {
	local	_bped_dict="${1}" _bped_fn_name="${2}"	\
		_bped_rc=0;

	eval set -- \${${_bped_fn_name}_depends:-};
	while [ "${#}" -gt 0 ]; do
		buildp_exec_depends "${_bped_dict}" "${1}" || return "${?}";
		if command -v "${1}" >/dev/null; then
			"${1}" "${_bped_dict}" || return "${?}";
		else
			return 1;
		fi;
		shift;
	done;

	return 0;
};
# }}}
# {{{ buildp_usage()
buildp_usage() {
	cat >&2 <<EOF
usage: ${0##*/} [-B <backend>] [-d] [-D] [-h] [-i] [-j <jobs>] [-t <target>]
      [<command> [<args>[...]] [and] [...]

      -B <backend>....: set default storage backend to either of ephemeral, file, or registry (default)
      -d..............: select Debug (vs. Release) build
      -D..............: select Debug w/o debugging console (vs. Release) build (for usage w/ dbg_server)
      -h..............: show this screen
      -i..............: ignore command dependencies
      -j <jobs>.......: set cmake(1) max. job count; defaults to processor count in /proc/cpuinfo on Linux
      -t <target>.....: build <target> instead of default target

Available commands:
clang_compile_cmds      Create compile_commands.json file
clean                   Clean PuTTie build directory
clean_all               Clean pcre2 and PuTTie build directory
clean_pcre2             Clean pcre2 build directory
configure               Configure PuTTie
configure_pcre2         Configure pcre2
dbg_client [...]        Attach to gdbserver on localhost:1234 with optional arguments to gdb
dbg_server [...]        Start gdbserver on localhost:1234 with optional arguments to putty.exe
distclean               Reset PuTTie build directory
distclean_all           Reset pcre2 and PuTTie build directory
distclean_pcre2         Reset pcre2 build directory
make                    Build PuTTie
make_pcre2              Build pcre2
install                 Create PuTTie release archive
publish                 Publish PuTTie release archive on GitHub

N.B.: When switching build types, run clean.
EOF
};
# }}}

build() {
	local	_b_dict="BUILD" _b_shift_count=0	\
		CMAKE="${CMAKE:-cmake}";

	if [ "$(uname -o 2>/dev/null)" = "Cygwin" ]; then
		export CMAKE="/usr/bin/cmake";
	fi;

	buildp_check_deps				\
		"${CMAKE}" cat cp curl find git json_xs	\
		ln make mkdir mktemp mv paste perl rm	\
		sed stat tr uname zip			\
			|| return "${?}";

	buildp_dict_from_args				\
		"${_b_dict}" \$_b_shift_count "${@}"	\
			|| return "${?}";
	shift "${_b_shift_count}";

	buildp_exec "${_b_dict}" "${@}" || return "${?}";

	return 0;
};

set -o errexit -o noglob -o nounset;
export LANG=C LC_ALL=C; build "${@}";

# vim:fdm=marker sw=8 ts=8 tw=0

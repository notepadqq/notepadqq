// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: http://codemirror.net/LICENSE

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
"use strict";

CodeMirror.defineMode('m4', function() {

  var words = {};
  function define(style, string) {
    var split = string.split(' ');
    for(var i = 0; i < split.length; i++) {
      words[split[i]] = style;
    }
  };

  // Atoms
  define('atom', 'true false');

  // Keywords
  define('keyword', 'if then do else elif while until for in esac fi fin ' +
    'fil done exit set unset export function');

  // Commands
  define('builtin', 'ab awk bash beep cat cc cd chown chmod chroot clear cp ' +
    'curl cut diff echo find gawk gcc get git grep kill killall ln ls make ' +
    'mkdir openssl mv nc node npm ping ps restart rm rmdir sed service sh ' +
    'shopt shred source sort sleep ssh start stop su sudo tee telnet top ' +
    'touch vi vim wall wc wget who write yes zsh');

  // autoconf/automake macros
  define('meta', ' ' +
    'AC_AIX AC_ALLOCA AC_ARG_ARRAY AC_ARG_ENABLE AC_ARG_PROGRAM AC_ARG_VAR ' +
    'AC_ARG_WITH AC_AUTOCONF_VERSION AC_BEFORE AC_CACHE_CHECK AC_CACHE_LOAD ' +
    'AC_CACHE_SAVE AC_CACHE_VAL AC_CANONICAL_BUILD AC_CANONICAL_HOST ' +
    'AC_CANONICAL_SYSTEM AC_CANONICAL_TARGET AC_C_BACKSLASH_A ' +
    'AC_C_BIGENDIAN AC_C_CHAR_UNSIGNED AC_C_CONST AC_C_CROSS ' +
    'AC_C_FLEXIBLE_ARRAY_MEMBER AC_CHAR_UNSIGNED AC_CHECK_ALIGNOF ' +
    'AC_CHECK_DECL AC_CHECK_DECLS AC_CHECK_DECLS_ONCE AC_CHECK_FILE ' +
    'AC_CHECK_FILES AC_CHECK_FUNC AC_CHECK_FUNCS AC_CHECK_FUNCS_ONCE ' +
    'AC_CHECK_HEADER AC_CHECK_HEADERS AC_CHECK_HEADERS_ONCE ' +
    'AC_CHECK_HEADER_STDBOOL AC_CHECKING AC_CHECK_LIB AC_CHECK_MEMBER ' +
    'AC_CHECK_MEMBERS AC_CHECK_PROG AC_CHECK_PROGS AC_CHECK_SIZEOF ' +
    'AC_CHECK_TARGET_TOOL AC_CHECK_TARGET_TOOLS AC_CHECK_TOOL ' +
    'AC_CHECK_TOOLS AC_CHECK_TYPE AC_CHECK_TYPE AC_CHECK_TYPES AC_C_INLINE ' +
    'AC_C_LONG_DOUBLE AC_COMPILE_CHECK AC_COMPILE_IFELSE AC_COMPILE_IFELSE ' +
    'AC_COMPUTE_INT AC_CONFIG_AUX_DIR AC_CONFIG_COMMANDS ' +
    'AC_CONFIG_COMMANDS_POST AC_CONFIG_COMMANDS_PRE AC_CONFIG_FILES ' +
    'AC_CONFIG_HEADER AC_CONFIG_HEADERS AC_CONFIG_ITEMS ' +
    'AC_CONFIG_LIBOBJ_DIR AC_CONFIG_LINKS AC_CONFIG_MACRO_DIR ' +
    'AC_CONFIG_SRCDIR AC_CONFIG_SUBDIRS AC_CONFIG_TESTDIR AC_CONST ' +
    'AC_COPYRIGHT AC_C_PROTOTYPES AC_C_RESTRICT AC_CROSS_CHECK ' +
    'AC_C_STRINGIZE AC_C_TYPEOF AC_C_VARARRAYS AC_C_VOLATILE AC_CYGWIN ' +
    'AC_DATAROOTDIR_CHECKED AC_DECL_SYS_SIGLIST AC_DECL_YYTEXT AC_DEFINE ' +
    'AC_DEFINE_UNQUOTED AC_DEFUN AC_DEFUN_ONCE AC_DIAGNOSE AC_DIR_HEADER ' +
    'AC_DISABLE_OPTION_CHECKING AC_DYNIX_SEQ AC_EGREP_CPP AC_EGREP_HEADER ' +
    'AC_EMXOS2 AC_ENABLE AC_ERLANG_CHECK_LIB AC_ERLANG_NEED_ERL ' +
    'AC_ERLANG_NEED_ERLC AC_ERLANG_PATH_ERL AC_ERLANG_PATH_ERLC ' +
    'AC_ERLANG_SUBST_ERTS_VER AC_ERLANG_SUBST_INSTALL_LIB_DIR ' +
    'AC_ERLANG_SUBST_INSTALL_LIB_DIR AC_ERLANG_SUBST_INSTALL_LIB_SUBDIR ' +
    'AC_ERLANG_SUBST_INSTALL_LIB_SUBDIR AC_ERLANG_SUBST_LIB_DIR ' +
    'AC_ERLANG_SUBST_ROOT_DIR AC_ERROR AC_EXEEXT AC_F77_DUMMY_MAIN ' +
    'AC_F77_FUNC AC_F77_IMPLICIT_NONE AC_F77_LIBRARY_LDFLAGS AC_F77_MAIN ' +
    'AC_F77_WRAPPERS AC_FATAL AC_FC_CHECK_BOUNDS AC_FC_DUMMY_MAIN ' +
    'AC_FC_FIXEDFORM AC_FC_FREEFORM AC_FC_FUNC AC_FC_IMPLICIT_NONE ' +
    'AC_FC_LIBRARY_LDFLAGS AC_FC_LINE_LENGTH AC_FC_MAIN ' +
    'AC_FC_MODULE_EXTENSION AC_FC_MODULE_FLAG AC_FC_MODULE_OUTPUT_FLAG ' +
    'AC_FC_PP_DEFINE AC_FC_PP_SRCEXT AC_FC_SRCEXT AC_FC_WRAPPERS AC_FIND_X ' +
    'AC_FIND_XTRA AC_FOREACH AC_FUNC_ALLOCA AC_FUNC_CHECK AC_FUNC_CHOWN ' +
    'AC_FUNC_CLOSEDIR_VOID AC_FUNC_ERROR_AT_LINE AC_FUNC_FNMATCH ' +
    'AC_FUNC_FNMATCH_GNU AC_FUNC_FORK AC_FUNC_FSEEKO AC_FUNC_GETGROUPS ' +
    'AC_FUNC_GETLOADAVG AC_FUNC_GETMNTENT AC_FUNC_GETPGRP AC_FUNC_LSTAT ' +
    'AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK AC_FUNC_MALLOC AC_FUNC_MBRTOWC ' +
    'AC_FUNC_MEMCMP AC_FUNC_MKTIME AC_FUNC_MMAP AC_FUNC_OBSTACK ' +
    'AC_FUNC_REALLOC AC_FUNC_SELECT_ARGTYPES AC_FUNC_SETPGRP ' +
    'AC_FUNC_SETVBUF_REVERSED AC_FUNC_STAT AC_FUNC_STRCOLL ' +
    'AC_FUNC_STRERROR_R AC_FUNC_STRFTIME AC_FUNC_STRNLEN AC_FUNC_STRTOD ' +
    'AC_FUNC_STRTOLD AC_FUNC_UTIME_NULL AC_FUNC_VPRINTF AC_FUNC_WAIT3 ' +
    'AC_GCC_TRADITIONAL AC_GETGROUPS_T AC_GETLOADAVG AC_GNU_SOURCE ' +
    'AC_HAVE_FUNCS AC_HAVE_HEADERS AC_HAVE_LIBRARY AC_HAVE_POUNDBANG ' +
    'AC_HEADER_ASSERT AC_HEADER_CHECK AC_HEADER_DIRENT AC_HEADER_EGREP ' +
    'AC_HEADER_MAJOR AC_HEADER_RESOLV AC_HEADER_STAT AC_HEADER_STDBOOL ' +
    'AC_HEADER_STDC AC_HEADER_SYS_WAIT AC_HEADER_TIME AC_HEADER_TIOCGWINSZ ' +
    'AC_HELP_STRING AC_INCLUDES_DEFAULT AC_INIT AC_INLINE AC_INT_16_BITS ' +
    'AC_IRIX_SUN AC_ISC_POSIX AC_LANG AC_LANG_ASSERT AC_LANG_C AC_LANG_CALL ' +
    'AC_LANG_CONFTEST AC_LANG_CPLUSPLUS AC_LANG_DEFINES_PROVIDED ' +
    'AC_LANG_FORTRAN77 AC_LANG_FUNC_LINK_TRY AC_LANG_POP AC_LANG_PROGRAM ' +
    'AC_LANG_PROGRAM AC_LANG_PUSH AC_LANG_RESTORE AC_LANG_SAVE ' +
    'AC_LANG_SOURCE AC_LANG_SOURCE AC_LANG_WERROR AC_LIBOBJ AC_LIBSOURCE ' +
    'AC_LIBSOURCES AC_LINK_FILES AC_LINK_IFELSE AC_LINK_IFELSE AC_LN_S ' +
    'AC_LONG_64_BITS AC_LONG_DOUBLE AC_LONG_FILE_NAMES AC_MAJOR_HEADER ' +
    'AC_MEMORY_H AC_MINGW32 AC_MINIX AC_MINUS_C_MINUS_O AC_MMAP AC_MODE_T ' +
    'AC_MSG_CHECKING AC_MSG_ERROR AC_MSG_FAILURE AC_MSG_NOTICE ' +
    'AC_MSG_RESULT AC_MSG_WARN AC_OBJEXT AC_OBSOLETE AC_OFF_T AC_OPENMP ' +
    'AC_OUTPUT AC_OUTPUT AC_OUTPUT_COMMANDS AC_PACKAGE_BUGREPORT ' +
    'AC_PACKAGE_NAME AC_PACKAGE_STRING AC_PACKAGE_TARNAME AC_PACKAGE_URL ' +
    'AC_PACKAGE_VERSION AC_PATH_PROG AC_PATH_PROGS ' +
    'AC_PATH_PROGS_FEATURE_CHECK AC_PATH_TARGET_TOOL AC_PATH_TOOL AC_PATH_X ' +
    'AC_PATH_XTRA AC_PID_T AC_PREFIX AC_PREFIX_DEFAULT AC_PREFIX_PROGRAM ' +
    'AC_PREPROC_IFELSE AC_PREPROC_IFELSE AC_PREREQ AC_PRESERVE_HELP_ORDER ' +
    'AC_PROG_AWK AC_PROG_CC AC_PROG_CC_C89 AC_PROG_CC_C99 AC_PROG_CC_C_O ' +
    'AC_PROG_CC_STDC AC_PROG_CPP AC_PROG_CPP_WERROR AC_PROG_CXX ' +
    'AC_PROG_CXX_C_O AC_PROG_CXXCPP AC_PROG_EGREP AC_PROG_F77 ' +
    'AC_PROG_F77_C_O AC_PROG_FC AC_PROG_FC_C_O AC_PROG_FGREP ' +
    'AC_PROG_GCC_TRADITIONAL AC_PROG_GREP AC_PROG_INSTALL AC_PROG_LEX ' +
    'AC_PROG_LN_S AC_PROG_MAKE_SET AC_PROG_MKDIR_P AC_PROG_OBJC ' +
    'AC_PROG_OBJCPP AC_PROG_OBJCXX AC_PROG_OBJCXXCPP AC_PROGRAM_CHECK ' +
    'AC_PROGRAM_EGREP AC_PROGRAM_PATH AC_PROGRAMS_CHECK AC_PROGRAMS_PATH ' +
    'AC_PROG_RANLIB AC_PROG_SED AC_PROG_YACC AC_REMOTE_TAPE ' +
    'AC_REPLACE_FNMATCH AC_REPLACE_FUNCS AC_REQUIRE AC_REQUIRE_AUX_FILE ' +
    'AC_REQUIRE_CPP AC_RESTARTABLE_SYSCALLS AC_RETSIGTYPE AC_REVISION ' +
    'AC_RSH AC_RUN_IFELSE AC_RUN_IFELSE AC_SCO_INTL AC_SEARCH_LIBS ' +
    'AC_SET_MAKE AC_SETVBUF_REVERSED AC_SIZEOF_TYPE AC_SIZE_T ' +
    'AC_STAT_MACROS_BROKEN AC_ST_BLKSIZE AC_ST_BLOCKS AC_STDC_HEADERS ' +
    'AC_STRCOLL AC_ST_RDEV AC_STRUCT_DIRENT_D_INO AC_STRUCT_DIRENT_D_TYPE ' +
    'AC_STRUCT_ST_BLKSIZE AC_STRUCT_ST_BLOCKS AC_STRUCT_ST_RDEV ' +
    'AC_STRUCT_TIMEZONE AC_STRUCT_TM AC_SUBST AC_SUBST_FILE ' +
    'AC_SYS_INTERPRETER AC_SYS_LARGEFILE AC_SYS_LONG_FILE_NAMES ' +
    'AC_SYS_POSIX_TERMIOS AC_SYS_RESTARTABLE_SYSCALLS ' +
    'AC_SYS_SIGLIST_DECLARED AC_TEST_CPP AC_TEST_PROGRAM ' +
    'AC_TIME_WITH_SYS_TIME AC_TIMEZONE AC_TRY_COMPILE AC_TRY_COMPILE ' +
    'AC_TRY_CPP AC_TRY_CPP AC_TRY_LINK AC_TRY_LINKAC_TRY_RUN ' +
    'AC_TRY_LINK_FUNC AC_TRY_RUN AC_TYPE_GETGROUPS AC_TYPE_INT16_T ' +
    'AC_TYPE_INT32_T AC_TYPE_INT64_T AC_TYPE_INT8_T AC_TYPE_INTMAX_T ' +
    'AC_TYPE_INTPTR_T AC_TYPE_LONG_DOUBLE AC_TYPE_LONG_DOUBLE_WIDER ' +
    'AC_TYPE_LONG_LONG_INT AC_TYPE_MBSTATE_T AC_TYPE_MODE_T AC_TYPE_OFF_T ' +
    'AC_TYPE_PID_T AC_TYPE_SIGNAL AC_TYPE_SIZE_T AC_TYPE_SSIZE_T ' +
    'AC_TYPE_UID_T AC_TYPE_UINT16_T AC_TYPE_UINT32_T AC_TYPE_UINT64_T ' +
    'AC_TYPE_UINT8_T AC_TYPE_UINTMAX_T AC_TYPE_UINTPTR_T ' +
    'AC_TYPE_UNSIGNED_LONG_LONG_INT AC_UID_T AC_UNISTD_H ' +
    'AC_USE_SYSTEM_EXTENSIONS AC_USG AC_UTIME_NULL ' +
    'AC_VALIDATE_CACHED_SYSTEM_TUPLE AC_VERBOSE AC_VFORK AC_VPRINTF ' +
    'AC_WAIT3 AC_WARN AC_WARNING AC_WITH AC_WORDS_BIGENDIAN AC_XENIX_DIR ' +
    'AC_YYTEXT_POINTER AH_BOTTOM AH_HEADER AH_TEMPLATE AH_TOP AH_VERBATIM ' +
    'AM_CONDITIONAL AM_COND_IF AM_CONFIG_HEADER AM_DEP_TRACK AM_GNU_GETTEXT ' +
    'AM_GNU_GETTEXT_INTL_SUBDIR AM_INIT_AUTOMAKE AM_MAINTAINER_MODE ' +
    'AM_MAKE_INCLUDE AM_MISSING_PROG AM_OUTPUT_DEPENDENCY_COMMANDS ' +
    'AM_PATH_LISPDIR AM_PATH_PYTHON AM_PROG_AR AM_PROG_AS AM_PROG_CC_C_O ' +
    'AM_PROG_GCJ AM_PROG_INSTALL_STRIP AM_PROG_LEX AM_PROG_MKDIR_P ' +
    'AM_PROG_UPC AM_PROG_VALAC AM_SANITY_CHECK AM_SET_DEPDIR ' +
    'AM_SILENT_RULES AM_SUBST_NOTMAKE AM_WITH_DMALLOC AS_BOURNE_COMPATIBLE ' +
    'AS_BOX AS_CASE AS_DIRNAME AS_ECHO AS_ECHO_N AS_ESCAPE AS_EXECUTABLE_P ' +
    'AS_EXIT AS_HELP_STRING AS_IF AS_INIT AS_INIT_GENERATED ' +
    'AS_LINENO_PREPARE AS_LITERAL_IF AS_LITERAL_WORD_IF AS_ME_PREPARE ' +
    'AS_MESSAGE_FD AS_MESSAGE_LOG_FD AS_MKDIR_P AS_ORIGINAL_STDIN_FD ' +
    'AS_SET_CATFILE AS_SET_STATUS AS_SHELL_SANITIZE AS_TMPDIR AS_TR_CPP ' +
    'AS_TR_SH AS_UNSET AS_VAR_APPEND AS_VAR_ARITH AS_VAR_COPY AS_VAR_IF ' +
    'AS_VAR_POPDEF AS_VAR_PUSHDEF AS_VAR_SET AS_VAR_SET_IF AS_VAR_TEST_SET ' +
    'AS_VERSION_COMPARE AT_ARG_OPTION AT_ARG_OPTION_ARG AT_BANNER ' +
    'AT_CAPTURE_FILE AT_CHECK AT_CHECK_EUNIT AT_CHECK_UNQUOTED AT_CLEANUP ' +
    'AT_COLOR_TESTS AT_COPYRIGHT AT_DATA AT_FAIL_IF AT_INIT AT_KEYWORDS ' +
    'AT_PACKAGE_BUGREPORT AT_PACKAGE_NAME AT_PACKAGE_STRING ' +
    'AT_PACKAGE_TARNAME AT_PACKAGE_URL AT_PACKAGE_VERSION AT_SETUP ' +
    'AT_SKIP_IF AT_TESTED AT_XFAIL_IF AU_ALIAS AU_DEFUN LT_INIT ' +
    'ac_cv_alignof_type-or-expr ac_cv_c_const ac_cv_c_int16_t ' +
    'ac_cv_c_int32_t ac_cv_c_int64_t ac_cv_c_int8_t ac_cv_c_restrict ' +
    'ac_cv_c_uint16_t ac_cv_c_uint32_t ac_cv_c_uint64_t ac_cv_c_uint8_t ' +
    'ac_cv_f77_compiler_gnu ac_cv_f77_dummy_main ac_cv_f77_implicit_none ' +
    'ac_cv_f77_libs ac_cv_f77_main ac_cv_f77_mangling ac_cv_fc_check_bounds ' +
    'ac_cv_fc_compiler_gnu ac_cv_fc_dummy_main ac_cv_fc_fixedform ' +
    'ac_cv_fc_freeform ac_cv_fc_implicit_none ac_cv_fc_libs ' +
    'ac_cv_fc_line_length ac_cv_fc_main ac_cv_fc_mangling ' +
    'ac_cv_fc_module_ext ac_cv_fc_module_flag ac_cv_fc_module_output_flag ' +
    'ac_cv_fc_pp_define ac_cv_fc_pp_srcext_ext ac_cv_fc_srcext_ext ' +
    'ac_cv_file_file ac_cv_func_chown_works ac_cv_func_closedir_void ' +
    'ac_cv_func_fnmatch_gnu ac_cv_func_fnmatch_works ac_cv_func_function ' +
    'ac_cv_func_getgroups_works ac_cv_func_getpgrp_void ' +
    'ac_cv_func_lstat_dereferences_slashed_symlink ' +
    'ac_cv_func_lstat_empty_string_bug ac_cv_func_malloc_0_nonnull ' +
    'ac_cv_func_mbrtowc ac_cv_func_memcmp_working ' +
    'ac_cv_func_mmap_fixed_mapped ac_cv_func_obstack ac_cv_func_pow ' +
    'ac_cv_func_realloc_0_nonnull ac_cv_func_setpgrp_void ' +
    'ac_cv_func_stat_empty_string_bug ac_cv_func_strcoll_works ' +
    'ac_cv_func_strerror_r_char_p ac_cv_func_strnlen_working ' +
    'ac_cv_func_strtod ac_cv_func_strtold ac_cv_func_utime_null ' +
    'ac_cv_func_working_mktime ac_cv_have_decl_symbol ' +
    'ac_cv_header_header-file ac_cv_header_stdbool_h ac_cv_header_stdc ' +
    'ac_cv_header_sys_wait_h ac_cv_header_time ac_cv_lib_error_at_line ' +
    'ac_cv_lib_library_function ac_cv_member_aggregate_member ' +
    'ac_cv_member_struct_stat_st_blocks ac_cv_path_install ac_cv_path_mkdir ' +
    'ac_cv_path_SED ac_cv_path_variable ac_cv_prog_AWK ac_cv_prog_cc_c89 ' +
    'ac_cv_prog_cc_c99 ac_cv_prog_cc_compiler_c_o ac_cv_prog_cc_stdc ' +
    'ac_cv_prog_c_openmp ac_cv_prog_cxx_openmp ac_cv_prog_EGREP ' +
    'ac_cv_prog_f77_c_o ac_cv_prog_f77_g ac_cv_prog_f77_openmp ' +
    'ac_cv_prog_f77_v ac_cv_prog_fc_c_o ac_cv_prog_fc_g ' +
    'ac_cv_prog_fc_openmp ac_cv_prog_fc_v ac_cv_prog_FGREP ac_cv_prog_GREP ' +
    'ac_cv_prog_LEX ac_cv_prog_variable ac_cv_prog_YACC ' +
    'ac_cv_search_function ac_cv_search_getmntent ac_cv_sizeof_type-or-expr ' +
    'ac_cv_sys_posix_termios ac_cv_type_getgroups ac_cv_type_long_double ' +
    'ac_cv_type_long_double_wider ac_cv_type_long_long_int ' +
    'ac_cv_type_mbstate_t ac_cv_type_mode_t ac_cv_type_off_t ' +
    'ac_cv_type_pid_t ac_cv_type_size_t ac_cv_type_ssize_t ac_cv_type_type ' +
    'ac_cv_type_uid_t ac_cv_type_unsigned_long_long_int __file__ __line__ ' +
    '__oline__ dnl m4_append m4_append_uniq m4_append_uniq_w m4_apply ' +
    'm4_argn m4_assert m4_bmatch m4_bpatsubst m4_bpatsubsts m4_bregexp ' +
    'm4_builtin m4_car m4_case m4_cdr m4_changecom m4_changequote m4_chomp ' +
    'm4_chomp_all m4_cleardivert m4_cmp m4_combine m4_cond m4_copy ' +
    'm4_copy_force m4_count m4_curry m4_debugfile m4_debugmode m4_decr ' +
    'm4_default m4_default_nblank m4_default_nblank_quoted ' +
    'm4_default_quoted m4_define m4_define_default m4_defn m4_divert ' +
    'm4_divert_once m4_divert_pop m4_divert_push m4_divert_text m4_divnum ' +
    'm4_do m4_dquote m4_dquote_elt m4_dumpdef m4_dumpdefs m4_echo ' +
    'm4_errprint m4_errprintn m4_escape m4_esyscmd m4_esyscmd_s m4_eval ' +
    'm4_exit m4_expand m4_fatal m4_flatten m4_for m4_foreach m4_foreach_w ' +
    'm4_format m4_if m4_ifblank m4_ifdef m4_ifnblank m4_ifndef m4_ifset ' +
    'm4_ifval m4_ifvaln m4_ignore m4_include m4_incr m4_index m4_indir ' +
    'm4_init m4_join m4_joinall m4_len m4_list_cmp m4_location m4_make_list ' +
    'm4_maketemp m4_map m4_mapall m4_mapall_sep m4_map_args ' +
    'm4_map_args_pair m4_map_args_sep m4_map_args_w m4_map_sep m4_max ' +
    'm4_min m4_mkstemp m4_n m4_newline m4_normalize m4_pattern_allow ' +
    'm4_pattern_forbid m4_popdef m4_pushdef m4_quote m4_re_escape m4_rename ' +
    'm4_rename_force m4_reverse m4_set_add m4_set_add_all m4_set_contains ' +
    'm4_set_contents m4_set_delete m4_set_difference m4_set_dump ' +
    'm4_set_empty m4_set_foreach m4_set_intersection m4_set_list ' +
    'm4_set_listc m4_set_map m4_set_map_sep m4_set_remove m4_set_size ' +
    'm4_set_union m4_shift m4_shift2 m4_shift3 m4_shiftn m4_sign ' +
    'm4_sinclude m4_split m4_stack_foreach m4_stack_foreach_lifo ' +
    'm4_stack_foreach_sep m4_stack_foreach_sep_lifo m4_strip m4_substr ' +
    'm4_syscmd m4_sysval m4_text_box m4_text_wrap m4_tolower m4_toupper ' +
    'm4_traceoff m4_traceon m4_translit m4_undefine m4_undivert m4_unquote ' +
    'm4_version_compare m4_version_prereq m4_warn m4_wrap m4_wrap_lifo ' +
    'PKG_CHECK_MODULES ' +
    ' ');

  function tokenBase(stream, state) {
    if (stream.eatSpace()) return null;

    var sol = stream.sol();
    var ch = stream.next();

    if (ch === '\\') {
      stream.next();
      return null;
    }
    if (ch === '[' || ch === ']') {
      return 'string';
    }
    if (ch === '\'' || ch === '"' || ch === '`') {
      state.tokens.unshift(tokenString(ch));
      return tokenize(stream, state);
    }
    if (ch === '#') {
//      if (sol && stream.eat('!')) {
//        stream.skipToEnd();
//        return 'meta'; // 'comment'?
//      }
      stream.skipToEnd();
      return 'comment';
    }
    // dnl Comment.
    if (ch === 'd') {
      if (sol && stream.eat('n') && stream.eat('l')) {
        stream.skipToEnd();
        return 'comment';
      }
    }
    if (ch === '$') {
      state.tokens.unshift(tokenDollar);
      return tokenize(stream, state);
    }
    if (ch === '+' || ch === '=') {
      return 'operator';
    }
    if (ch === '-') {
      stream.eat('-');
      stream.eatWhile(/\w/);
      return 'attribute';
    }
    if (/\d/.test(ch)) {
      stream.eatWhile(/\d/);
      if(stream.eol() || !/\w/.test(stream.peek())) {
        return 'number';
      }
    }
    stream.eatWhile(/[\w-]/);
    var cur = stream.current();
    if (stream.peek() === '=' && /\w+/.test(cur)) return 'def';
    return words.hasOwnProperty(cur) ? words[cur] : null;
  }

  function tokenString(quote) {
    return function(stream, state) {
      var next, end = false, escaped = false;
      while ((next = stream.next()) != null) {
        if (next === quote && !escaped) {
          end = true;
          break;
        }
        if (next === '$' && !escaped && quote !== '\'') {
          escaped = true;
          stream.backUp(1);
          state.tokens.unshift(tokenDollar);
          break;
        }
        escaped = !escaped && next === '\\';
      }
      if (end || !escaped) {
        state.tokens.shift();
      }
      return (quote === '`' || quote === ')' ? 'quote' : 'string');
    };
  };

  var tokenDollar = function(stream, state) {
    if (state.tokens.length > 1) stream.eat('$');
    var ch = stream.next(), hungry = /\w/;
    if (ch === '{') hungry = /[^}]/;
    if (ch === '(') {
      state.tokens[0] = tokenString(')');
      return tokenize(stream, state);
    }
    if (!/\d/.test(ch)) {
      stream.eatWhile(hungry);
      stream.eat('}');
    }
    state.tokens.shift();
    return 'def';
  };

  function tokenize(stream, state) {
    return (state.tokens[0] || tokenBase) (stream, state);
  };

  return {
    startState: function() {return {tokens:[]};},
    token: function(stream, state) {
      return tokenize(stream, state);
    },
    lineComment: '#',
    fold: "brace"
  };
});

CodeMirror.defineMIME('application/x-m4', 'm4');

});

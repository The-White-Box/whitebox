# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# CLANG compiler configuration.
#
# Derived from https://github.com/facebook/folly/blob/master/CMake/FollyCompilerUnix.cmake
# which is licensed under the Apache License, Version 2.0 (the "License").  See
# the License for the specific language governing permissions andlimitations
# under the License.

include(wb_clang_tidy_configuration)

option(WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS           "Define _GLIBCXX_ASSERTIONS macro to enable extra error checking in the form of precondition assertions, such as bounds checking in strings and null pointer checks when dereferencing smart pointers." ON)
option(WB_CLANG_DEFINE__GLIBCXX_SANITIZE_VECTOR      "Define _GLIBCXX_SANITIZE_VECTOR macro to annotate std::vector operations so that AddressSanitizer can detect invalid accesses to the unused capacity of a std::vector." ON)
option(WB_CLANG_ENABLE_ALL_WARNINGS                  "If enabled, pass -Wall to the Clang compiler.  See https://clang.llvm.org/docs/UsersManual.html#enabling-all-diagnostics" ON)
option(WB_CLANG_ENABLE_EXCEPTIONS                    "Enable exceptions." ON)
option(WB_CLANG_ENABLE_FAST_MATH                     "Enable fast-math mode.  This option lets the compiler make aggressive, potentially-lossy assumptions about floating-point math." OFF)
option(WB_CLANG_ENABLE_LOOPS_UNROLLING               "If enabled, use -funroll-loops for Release builds." OFF)
option(WB_CLANG_THREAT_COMPILER_WARNINGS_AS_ERRORS   "If enabled, pass -Werror to the Clang compiler." ON)

wb_define_strings_option(WB_CLANG_DEFINE__FORTIFY_SOURCE
  "Define _FORTIFY_SOURCE macro to a positive integer to control which source fortification is enabled."
  "2" "1" "")

wb_define_strings_option(WB_CLANG_DEFINE__LIBCPP_DEBUG
  "Define _LIBCPP_DEBUG macro to enable extra error checking in libc++ to detect incorrect usage of the standard library. libc++ should be build with LIBCXX_ENABLE_DEBUG_MODE_SUPPORT."
  "1" "0" "")

wb_define_strings_option(WB_CLANG_DEFINE__POSIX_C_SOURCE
  "Define _POSIX_C_SOURCE macro to a positive integer to control which POSIX functionality is made available. The greater the value of this macro, the more functionality is made available."
  "200809L" "200112L" "199506L" "")

wb_define_strings_option(WB_CLANG_ENABLE_CET_PROTECTION
  "Enables Control-flow Enforcement Technology (CET) protection, which defends your program from certain attacks that exploit vulnerabilities. This option offers preliminary support for CET."
  "none" "full" "branch" "return")

wb_define_strings_option(WB_CLANG_LINKER_TYPE
    "Replaces default ld linker with one from predefined set."
    "lld" "gold" "ld")

wb_define_strings_option(WB_CLANG_ENABLE_LTO
  "If enabled, use Link Time Optimization for Release builds."
  "-flto=thin" "-flto" "-flto=full" "")

wb_define_strings_option(WB_CLANG_LANGUAGE_VERSION
  "This determines which version of C++ to compile as via Clang."
  "c++17" "c++20")

wb_define_strings_option(WB_CLANG_MINIMUM_CPU_ARCHITECTURE
  "This tells the compiler to choose minimum instruction set for the specified architecture."
  "" "native" "x86-64")

wb_define_strings_option(WB_CLANG_DEBUG_OPTIMIZATION_LEVEL
  "This tells the compiler to choose optimization level in Debug build."
  "-Og" "-O0" "")

wb_define_strings_option(WB_CLANG_RELEASE_OPTIMIZATION_LEVEL
  "This tells the compiler to choose optimization level in Release build."
  "-O2" "-O1" "-O0" "-O3" "-Os" "-Ofast")

wb_define_strings_option(WB_CLANG_STACK_PROTECTOR_LEVEL
  "This tells the compiler to force the addition of a stack canary that checks for stack smashing."
  "-strong" "-all" "")

# Apply the option set for WhiteBox to the specified target.
function(wb_apply_compile_options_to_target THE_TARGET)
  # First determine clang-tidy is present.  If present, we should use Clang-compatible flags only, or clang-tidy will
  # complain about unknown flags.
  wb_apply_clang_tidy_options_to_target(APPLY_CLANG_TIDY ${THE_TARGET} ${WB_CLANG_LANGUAGE_VERSION})

  target_compile_options(${THE_TARGET}
    PRIVATE
      # Increased reliability of backtraces.
      -fasynchronous-unwind-tables
      # Enable fast math.
      $<$<BOOL:${WB_CLANG_ENABLE_FAST_MATH}>:-ffast-math>
      # Enable table-based thread cancellation.
      -fexceptions
      # String and character constants in UTF-8 during execution.
      -fexec-charset=utf-8
      # Sources should be in UTF-8.
      -finput-charset=utf-8
      # Full ASLR for executables.
      $<$<STREQUAL:$<TARGET_PROPERTY:${THE_TARGET},TYPE>,EXECUTABLE>:
        -fPIE
      >
      # No text relocations for shared libraries.
      $<$<STREQUAL:$<TARGET_PROPERTY:${THE_TARGET},TYPE>,SHARED_LIBRARY>:
        -fPIC
      >
      # Let the type char be signed, like signed char.
      -fsigned-char
      # Assume strict aliasing rules applied.
      -fstrict-aliasing
      # Do not export symbols by default, only when explicitly marked.
      -fvisibility=hidden
      # All inlined class member functions to have hidden visibility.
      -fvisibility-inlines-hidden
      # Produce debugging information.
      -g
      # Enable or disable exceptions.
      $<$<NOT:$<BOOL:${WB_CLANG_ENABLE_EXCEPTIONS}>>:-fno-exceptions>
      # Avoid temporary files, speeding up builds.
      -pipe
      # Enables support for the Control-Flow Enforcement Technology (CET).
      -fcf-protection=${WB_CLANG_ENABLE_CET_PROTECTION}
      # Force the addition of a stack canary that checks for stack smashing.
      -fstack-protector${WB_CLANG_STACK_PROTECTOR_LEVEL}
      # Minimum architecture (instruction sets) to use when generating code.
      $<$<BOOL:${WB_CLANG_MINIMUM_CPU_ARCHITECTURE}>:-march=${WB_CLANG_MINIMUM_CPU_ARCHITECTURE}>

      ## Warnings

      # Enable almost all warnings.
      $<$<BOOL:${WB_CLANG_ENABLE_ALL_WARNINGS}>:-Wall>
      # Enable extra warnings.
      -Wextra
      # Threat warnings as errors.
      $<$<BOOL:${WB_CLANG_THREAT_COMPILER_WARNINGS_AS_ERRORS}>:-Werror>
      # Warn about code affected by ABI changes.  This includes code that may
      # not be compatible with the vendor-neutral C++ ABI as well as the psABI
      # for the particular target.
      -Wabi
      # Warn about calls to allocation functions decorated with attribute
      # alloc_size that specify zero bytes.  This is implementation defined
      # behavior.
      ### -Walloc-zero
      # Warn about declarations using the alias and similar attributes whose
      # target is incompatible with the type of the alias.
      # At 2 level -Wattribute-alias also diagnoses cases where the attributes
      # of the alias declaration are more restrictive than the attributes
      # applied to its target.
      ### -Wattribute-alias=2
      # Warn whenever a pointer is cast such that the required alignment of the
      # target is increased.  For example, warn if a char * is cast to an int *
      # on machines where integers can only be accessed at two- or four-byte
      # boundaries.
      -Wcast-align
      # Warn whenever a pointer is cast so as to remove a type qualifier from
      # the target type.  For example, warn if a const char * is cast to an
      # ordinary char *.
      -Wcast-qual
      # Warn for implicit conversions that may alter a value.
      -Wconversion
      # Warn when macros __TIME__, __DATE__ or __TIMESTAMP__ are encountered as
      # they might prevent bit-wise-identical reproducible compilations.
      -Wdate-time
      # Warn if a requested optimization pass is disabled.  This warning does
      # not generally indicate that there is anything wrong with your code; it
      # merely indicates that GCCs optimizers are unable to handle the code
      # effectively.  Often, the problem is that your code is too big or too
      # complex.
      -Wdisabled-optimization
      # Give a warning when a value of type float is implicitly promoted to
      # double.
      -Wdouble-promotion
      # Warn when an if-else has identical branches.
      ### -Wduplicated-branches
      # Warn about duplicated conditions in an if-else-if chain.
      ### -Wduplicated-cond
      # Warn when a value of enumerated type is implicitly converted to a
      # different enumerated type.
      -Wenum-conversion
      # Warn when a switch case falls through.  -Wimplicit-fallthrough=5 doesnt
      # recognize any comments as fallthrough comments, only attributes disable
      # the warning.
      -Wimplicit-fallthrough
      # Warn if floating-point values are used in equality comparisons.
      #
      # The idea behind this is that sometimes it is convenient (for the
      # programmer) to consider floating-point values as approximations to
      # infinitely precise real numbers.  If you are doing this, then you need
      # to compute (by analyzing the code, or in some other way) the maximum or
      # likely maximum error that the computation introduces, and allow for it
      # when performing comparisons (and when producing output, but thats a
      # different problem).  In particular, instead of testing for equality, you
      # should check to see whether the two values have ranges that overlap; and
      # this is done with the relational operators, so equality comparisons are
      # probably mistaken.
      -Wfloat-equal
      # Check calls to printf and scanf, etc., to make sure that the arguments
      # supplied have types appropriate to the format string specified, and that
      # the conversions specified in the format string make sense.
      #
      # Also enable additional format checks.  Currently equivalent to:
      # -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k.
      -Wformat=2
      # Warn about calls to formatted input/output functions such as sprintf and
      # vsprintf that might overflow the destination buffer.
      #
      # Level 1 of -Wformat-overflow enabled by -Wformat employs a conservative
      # approach that warns only about calls that most likely overflow the
      # buffer.
      # Level 2 warns also about calls that might overflow the destination
      # buffer given an argument of sufficient length or magnitude.
      ### -Wformat-overflow=2
      # If -Wformat is specified, also warn if the format string is not a string
      # literal and so cannot be checked, unless the format function takes its
      # format arguments as a va_list.
      -Wformat-nonliteral
      # If -Wformat is specified, also warn about uses of format functions that
      # represent possible security problems.
      -Wformat-security
      # If -Wformat is specified, also warn if the format string requires an
      # unsigned argument and the argument is signed and vice versa.
      ### -Wformat-signedness
      # Warn about calls to formatted input/output functions such as snprintf
      # and vsnprintf that might result in output truncation.
      #
      # Level 1 of -Wformat-truncation enabled by -Wformat employs a
      # conservative approach that warns only about calls to bounded functions
      # whose return value is unused and that will most likely result in output
      # truncation.
      # Level 2 warns also about calls to bounded functions whose return value
      # is used and that might result in truncation given an argument of
      # sufficient length or magnitude.
      ### -Wformat-truncation=2
      # If -Wformat is specified, also warn about strftime formats that may
      # yield only a two-digit year.
      -Wformat-y2k
      # Warn if a precompiled header (see Precompiled Headers) is found in the
      # search path but cannot be used.
      -Winvalid-pch
      # Warn about suspicious uses of logical operators in expressions.  This
      # includes using logical operators in contexts where a bit-wise operator
      # is likely to be expected.  Also warns when the operands of a logical
      # operator are the same.
      ### -Wlogical-op
      # Warn if a structure is given the packed attribute, but the packed
      # attribute has no effect on the layout or size of the structure.  Such
      # structures may be mis-aligned for little benefit.
      -Wpacked
      # Warn about anything that depends on the size of a function type or of
      # void.  GNU C assigns these types a size of 1, for convenience in
      # calculations with void * pointers and pointers to functions.  In C++,
      # warn also when an arithmetic operation involves NULL.
      -Wpointer-arith
      # Warn if a user-supplied include directory does not exist.
      -Wmissing-include-dirs
      # Warn if missing noreturn attribute.
      -Wmissing-noreturn
      # Warn when the compiler detects paths that dereferences a null pointer.
      -Wnull-dereference
      # Warn if padding is included in a structure, either to align an element
      # of the structure or to align the whole structure.
      -Wpadded
      # Warn about C-style casts.  This will force any undefined type puns to
      # use reinterpret_cast, in general reinterpret_cast should be a flag for
      # closer code review.  It is also easiser to search your code base for
      # reinterpret_cast to perform an audit.
      -Wold-style-cast
      # Warn about string constants that are longer than the minimum maximum
      # length specified in the C standard.
      -Woverlength-strings
      # Warn when the compiler detects that an argument passed to a restrict or
      # __restrict qualified parameter alias with another parameter.
      ### -Wrestrict
      # Warn for implicit conversions that may change the sign of an integer
      # value, like assigning a signed integer expression to an unsigned integer
      # variable.
      -Wsign-conversion
      # Warn if declared name shadows the same name in some outer level.
      -Wshadow
      # Control warnings about left shift overflows.
      -Wshift-overflow
      # Warn whenever a switch statement has an index of enumerated type and
      # lacks a case for one or more of the named codes of that enumeration.
      # case labels outside the enumeration range also provoke warnings when
      # this option is used.
      # The only difference between -Wswitch and this option is that this
      # option gives a warning about an omitted enumeration code even if there
      # is a default label.
      -Wswitch-enum
      # Warn whenever a switch statement does not have a default case.
      -Wswitch-default
      # This option is only active when -fstack-protector is active.  It warns
      # about functions that are not protected against stack smashing.
      -Wstack-protector
      # It warns about code that might break the strict aliasing rules that the
      # compiler is using for optimization.
      #
      # Level 1: Most aggressive, quick, least accurate.  Possibly useful when
      # higher levels do not warn but -fstrict-aliasing still breaks the code,
      # as it has very few false negatives.  However, it has many false
      # positives.  Warns for all pointer conversions between possibly
      # incompatible types, even if never dereferenced.  Runs in the front end
      # only.
      # Level 2: Aggressive, quick, not too precise. May still have many false
      # positives (not as many as level 1 though), and few false negatives (but
      # possibly more than level 1).  Unlike level 1, it only warns when an
      # address is taken.  Warns about incomplete types.  Runs in the front end
      # only.
      -Wstrict-aliasing=1
      # Warn for cases where adding an attribute may be beneficial.
      ### -Wsuggest-attribute=pure
      ### -Wsuggest-attribute=cold
      ### -Wsuggest-attribute=const
      ### -Wsuggest-attribute=malloc
      ### -Wsuggest-attribute=noreturn
      # Warn about trampolines generated for pointers to nested functions.  For
      # most targets, it is made up of code and thus requires the stack to be
      # made executable in order for the program to work properly.
      ### -Wtrampolines
      # Warn if an undefined identifier is evaluated in an #if directive.  Such
      # identifiers are replaced with zero.
      -Wundef
      # Warn on unused functions vars, etc.
      -Wunused
      # Warn about macros defined in the main file that are unused.  A macro is
      # used if it is expanded or tested for existence at least once.  The
      # preprocessor also warns if the macro has not been used at the time it is
      # redefined or undefined.
      -Wunused-macros
      # Warn if the loop cannot be optimized because the compiler cannot assume
      # anything on the bounds of the loop indices.
      ### -Wunsafe-loop-optimizations
      # Warn when an expression is cast to its own type within a C++ program.
      ### -Wuseless-cast

      # Warn if vector operation is not implemented via SIMD capabilities of the
      # architecture.  Mainly useful for the performance tuning.
      ### -Wvector-operation-performance

      # Build in the requested version of C++.
      -std=${WB_CLANG_LANGUAGE_VERSION}

      ## Debug configuration
      $<$<CONFIG:DEBUG>:
        $<$<BOOL:${WB_CLANG_DEBUG_OPTIMIZATION_LEVEL}>:${WB_CLANG_DEBUG_OPTIMIZATION_LEVEL}>
      >

      ## Non-debug configuration.
      $<$<NOT:$<CONFIG:DEBUG>>:
        # Try max optimization.  Be careful, this can lead to pessimization!
        $<$<BOOL:${WB_CLANG_RELEASE_OPTIMIZATION_LEVEL}>:${WB_CLANG_RELEASE_OPTIMIZATION_LEVEL}>
        # Enable link time optimization.
        $<$<BOOL:${WB_CLANG_ENABLE_LTO}>:${WB_CLANG_ENABLE_LTO}>
        # Enable loop unrolling.
        $<$<BOOL:${WB_CLANG_ENABLE_LOOPS_UNROLLING}>:-funroll-loops>
      >
  )

  target_compile_definitions(${THE_TARGET}
    PUBLIC
      ## Debug configuration.
      $<$<CONFIG:DEBUG>:
        # The annotations must be present on all vector operations or none, so
        # this macro must be defined to the same value for all translation units
        # that create, destroy or modify vectors.
        $<$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_SANITIZE_VECTOR}>:
          _GLIBCXX_SANITIZE_VECTOR
        >
      >

    PRIVATE
      # Mostly obsolete enabler of thread-safe function versions in stdlib.
      _REENTRANT
      # Made available functions from the POSIX standard.
      $<$<BOOL:${WB_CLANG_DEFINE__POSIX_C_SOURCE}>:
        _POSIX_C_SOURCE=${WB_CLANG_DEFINE__POSIX_C_SOURCE}
      >

      ## Security.
      # Compile + run-time buffer overflow detection.
      $<$<BOOL:${WB_CLANG_DEFINE__FORTIFY_SOURCE}>:
        # TODO: ASAN doesn't like this! Disable if ASAN is present.
        _FORTIFY_SOURCE=${WB_CLANG_DEFINE__FORTIFY_SOURCE}
      >

      ## Debug configuration.
      $<$<CONFIG:DEBUG>:
        # Run-time bounds checking for C++ strings/containers.
        $<$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS}>:_GLIBCXX_ASSERTIONS>
        # Catch libstdc++ usage errors by enabling debug mode.  When defined,
        # _GLIBCXX_ASSERTIONS is defined automatically
        $<$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS}>:_GLIBCXX_DEBUG>
        # When defined makes the debug mode extremely picky by making the use of
        # libstdc++ extensions and libstdc++-specific behavior into errors.
        $<$<BOOL:${WB_CLANG_DEFINE__GLIBCXX_ASSERTIONS}>:_GLIBCXX_DEBUG_PEDANTIC>
        # Enables special debugging checks meant to detect incorrect usage of the libc++ standard library.
        $<$<NOT:$<STREQUAL:"${WB_CLANG_DEFINE__LIBCPP_DEBUG}","">>:_LIBCPP_DEBUG=${WB_CLANG_DEFINE__LIBCPP_DEBUG}>
      >
  )

  target_link_options(${THE_TARGET}
    PRIVATE
      # Detect and reject underlinking.
      -Wl,-z,defs
      # Marks some section read only, which prevents some GOT overwrite attacks.
      -Wl,-z,relro
      # All symbols are resolved at load time.  Combined with the previous flag,
      # this prevents more GOT overwrite attacks.
      -Wl,-z,now

      # Full ASLR for executables.
      $<$<STREQUAL:$<TARGET_PROPERTY:${THE_TARGET},TYPE>,EXECUTABLE>:
        -pie
      >
      # No text relocations for shared libraries.
      $<$<STREQUAL:$<TARGET_PROPERTY:${THE_TARGET},TYPE>,SHARED_LIBRARY>:
        -shared
      >
      # -Wl,-z,cet-report=error
      # Use faster than default linker.
      -fuse-ld=${WB_CLANG_LINKER_TYPE}
  )

  wb_dump_target_property(${THE_TARGET} COMPILE_OPTIONS     "cxx compiler   flags")
  wb_dump_target_property(${THE_TARGET} COMPILE_DEFINITIONS "cxx compiler defines")
  wb_dump_target_property(${THE_TARGET} LINK_OPTIONS        "cxx linker     flags")

  if (APPLY_CLANG_TIDY)
    wb_dump_target_property(${THE_TARGET} CXX_CLANG_TIDY    "cxx clang-tidy flags")
  else()
    message(STATUS "${THE_TARGET} cxx clang-tidy flags: not applied on Clang.")
  endif()

  message("--")
endfunction()
# Copyright (c) 2021 The WhiteBox Authors.  All rights reserved.
# Use of this source code is governed by a 3-Clause BSD license that can be
# found in the LICENSE file.
#
# MSVC compiler configuration.
#
# Derived from https://github.com/facebook/folly/blob/master/CMake/FollyCompilerMSVC.cmake
# which is licensed under the Apache License, Version 2.0 (the "License").  See
# the License for the specific language governing permissions and limitations
# under the License.

include(wb_clang_tidy_configuration)

# Some additional configuration options.
option(WB_MSVC_CREATE_HOTPATCHABLE_IMAGE                        "If enabled, compiler prepares an image for hot patching." OFF)
option(WB_MSVC_ENABLE_ALL_WARNINGS                              "If enabled, pass /Wall to the compiler." ON)
option(WB_MSVC_ENABLE_ADDITIONAL_SECURITY_CHECKS                "If enabled, use recommended Security Development Lifecycle (SDL) checks.  These checks change security-relevant warnings into errors, and set additional secure code-generation features.." ON)
option(WB_MSVC_ENABLE_COROUTINE_SUPPORT                         "If enabled, enable compiler support for coroutines." ON)
option(WB_MSVC_ENABLE_DEBUG_INLINING                            "If enabled, enable inlining in the debug configuration.  This allows /Zc:inline to be far more effective." OFF)
option(WB_MSVC_ENABLE_FAST_LINK                                 "If enabled, pass /DEBUG:FASTLINK to the linker.  This makes linking faster, but the gtest integration for Visual Studio can't currently handle the .pdbs generated." OFF)
option(WB_MSVC_ENABLE_GUARD_FOR_CONTROL_FLOW                    "If enabled, compiler/linker generate Control Flow Guard security checks." ON)
option(WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION                 "If enabled, compiler/linker generate a sorted list of the relative virtual addresses (RVA) of all the valid exception handling continuation targets for a binary.  It's used during runtime for NtContinue and SetThreadContext instruction pointer validation." ON)
option(WB_MSVC_ENABLE_INTRINSICS                                "If enabled, replaces some function calls with intrinsic or otherwise special forms.  Using the true intrinsic forms implies loss of IEEE exception handling, and loss of _matherr and errno functionality; the latter implies loss of ANSI conformance." ON)
option(WB_MSVC_ENABLE_LEAN_AND_MEAN_WINDOWS                     "If enabled, define WIN32_LEAN_AND_MEAN to include a smaller subset of Windows.h" ON)
option(WB_MSVC_ENABLE_LTCG                                      "If enabled, use Link Time Code Generation for Release builds." ON)
option(WB_MSVC_ENABLE_PARALLEL_BUILD                            "If enabled, build multiple source files in parallel." ON)
option(WB_MSVC_ENABLE_RTTI                                      "If enabled, adds code to check object types at run time (dynamic_cast, typeid)." ON)
option(WB_MSVC_THREAT_STATIC_CODE_ANALYSIS_WARNINGS_AS_ERRORS   "If enabled, threat complex static analysis warnings as errors." ON)
option(WB_MSVC_THREAT_COMPILER_WARNINGS_AS_ERRORS               "If enabled, pass /WX to the compiler. Compiler will threat warnings as errors." ON)
option(WB_MSVC_USE_STATIC_RUNTIME                               "If enabled, build against the static, rather than the dynamic, runtime." OFF)
option(WB_MSVC_USE_UTF16_WINAPI_INSTEAD_OF_ANSI                 "If enabled, uses *W (UTF-16) WinAPI instead of *A ANSI." OFF)
option(WB_MSVC_USE_SECURE_CRT_OVERLOAD_STANDARD_NAMES           "If enabled, uses secure tempalte overloads to ex. replace the strcpy call to strcpy_s, which prevents buffer overruns.  See https://docs.microsoft.com/en-us/cpp/c-runtime-library/secure-template-overloads" ON)

wb_define_strings_option(WB_MSVC_STATIC_CODE_ANALYSIS_MODE
  "Specifies complex source code analysis mode.  Plugins require presetup.  See https://docs.microsoft.com/en-us/cpp/build/reference/analyze-code-analysis."
  "" "/analyze" "/analyze:only" "/analyze:plugin EspXEngine.dll")

wb_define_strings_option(WB_MSVC_DEBUG_RUNTIME_ERROR_CHECKS
  "Used to enable and disable the run-time error checks feature in DEBUG, in conjunction with the runtime_checks pragma."
  "csu" "c" "cs" "cu" "s" "su")

wb_define_strings_option(WB_MSVC_EXCEPTION_HANDLING_MODEL
  "Specifies the exception handling model support generated by the compiler."
  "s" "sr" "sc" "scr" "a" "ar" "c" "r")

wb_define_strings_option(WB_MSVC_FAVORED_CPU_ARCHITECTURE
  "This tells the compiler to generate code optimized to run best on the specified architecture."
  "blend" "AMD64" "ATOM" "INTEL64")

wb_define_strings_option(WB_MSVC_FLOATING_POINT_BEHAVIOR
  "Specifies how the compiler treats floating-point expressions, optimizations, and exceptions"
  "precise" "strict" "fast")

wb_define_strings_option(WB_MSVC_FLOATING_POINT_EXCEPTIONS_BEHAVIOR
  "Specifies how any unmasked floating-point exceptions are raised at the exact point at which they occur, and that no additional floating-point exceptions are raised. WB_MSVC_FLOATING_POINT_BEHAVIOR:strict option enables except, and WB_MSVC_FLOATING_POINT_BEHAVIOR:precise does not.  The except option is not compatible with WB_MSVC_FLOATING_POINT_BEHAVIOR:fast."
  "except-" "except")

wb_define_strings_option(WB_MSVC_CXX_LANGUAGE_VERSION
  "This determines which version of C++ to compile as."
  "c++17" "c++latest")

wb_define_strings_option(WB_MSVC_MINIMUM_CPU_ARCHITECTURE
  "This tells the compiler to choose minimum instruction set for the specified architecture."
  "" "AVX" "AVX2" "AVX512" "ARMv7VE" "VFPv4")


############################################################
# We need to adjust a couple of the default option sets.
############################################################

# If the static runtime is requested, we have to overwrite some of CMake's
# defaults.
if (WB_MSVC_USE_STATIC_RUNTIME)
  foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

# The Ninja generator doesn't de-dup the exception mode flag, so remove the
# default flag so that MSVC doesn't warn about it on every single file.
if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
  foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if (${flag_var} MATCHES "/EHsc")
      string(REGEX REPLACE "/EHsc" "" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

# The Ninja generator adds /GR (enable runtime type info) to CMake args.  Remove
# it here, so we can override with our behavior.
foreach(flag_var
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
  if (${flag_var} MATCHES "/GR")
    string(REGEX REPLACE "/GR" "" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

# In order for /Zc:inline, which speeds up the build significantly, to work we
# need to remove the /Ob0 parameter that CMake adds by default, because that
# would normally disable all inlining.
foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
  if (${flag_var} MATCHES "/Ob0")
    string(REGEX REPLACE "/Ob0" "" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

# At least Ninja adds /RTC1 runtime check in debug mode.  We are going to
# override with our one later, so replace with empty.
foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
  if (${flag_var} MATCHES "/RTC1")
    string(REGEX REPLACE "/RTC1" "" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

# At least Ninja doesn't remove the /W3 flag when we add /W4|/Wall one, which
# leads to compilation warnings.  Remove /W3 entirely, as /W4|/Wall be used.
foreach(flag_var
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
  if (${flag_var} MATCHES "/W3")
    string(REGEX REPLACE "/W3" "" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

# When building with Ninja, or with /MP enabled, there is the potential
# for multiple processes to need to lock the same pdb file.
# The /FS option (which is implicitly enabled by /MP) is widely believed
# to be the solution for this, but even with /FS enabled the problem can
# still randomly occur.
# https://stackoverflow.com/a/58020501/149111 suggests that /Z7 should be
# used; rather than placing the debug info into a .pdb file it embeds it
# into the object files in a similar way to gcc/clang which should reduce
# contention and potentially make the build faster... but at the cost of
# larger object files
foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
  if (${flag_var} MATCHES "/Zi")
    string(REGEX REPLACE "/Zi" "/Z7" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

# Apply the option set for WhiteBox to the specified target.
function(wb_apply_compile_options_to_target THE_TARGET)
  # First determine clang-tidy is present.  If present, we should use Clang-compatible flags only, or clang-tidy will
  # complain about unknown flags.
  # wb_apply_clang_tidy_options_to_target(APPLY_CLANG_TIDY ${THE_TARGET} ${WB_MSVC_CXX_LANGUAGE_VERSION})

  # The general options passed:
  target_compile_options(${THE_TARGET}
    PUBLIC
      # The /await compiler option enables compiler support for C++ coroutines
      # and the keywords co_await, co_yield, and co_return.
      $<$<BOOL:${WB_MSVC_ENABLE_COROUTINE_SUPPORT}>:/await>
      # Specifies the exception handling model support generated by the
      # compiler.  Arguments specify whether to apply catch(...) syntax to both
      # structured and standard C++ exceptions, whether extern "C" code is
      # assumed to throw exceptions, and whether to optimize away certain
      # noexcept checks.
      /EH${WB_MSVC_EXCEPTION_HANDLING_MODEL}
      # Results in more efficient code for an .exe file for accessing thread-
      # local storage (TLS) variables.  Using /GA for a DLL can result in bad
      # code generation.
      $<$<STREQUAL:$<TARGET_PROPERTY:${THE_TARGET},TYPE>,EXECUTABLE>:
        /GA
      >
      # Enables the compiler to create a single copy of identical strings in the
      # program image and in memory during execution.
      #
      # String pooling makes pointers to same buffers point to the same memory:
      # const char *s = "This is a character buffer";
      # const char *t = "This is a character buffer";
      # assert(s == t);
      /GF
      # Enable Run-Time Type Information.  You usually need the /GR option when
      # your code uses dynamic_cast Operator or typeid.
      $<$<BOOL:${WB_MSVC_ENABLE_RTTI}>:/GR>
      # Detects some buffer overruns that overwrite a function's return address,
      # exception handler address, or certain types of parameters.
      /GS
      # Supports fiber safety for data allocated using static thread-local
      # storage, that is, data allocated with __declspec(thread).
      #
      # /GT prevents such optimizations as caching the address of the TLS array,
      # or optimize it as a common subexpression across a function call.
      #
      # Set this if you use static TLS and fibers.
      /GT
      # Causes the compiler to analyze control flow for indirect call targets at
      # compile time, and then to insert code to verify the targets at runtime.
      #
      # The /guard:cf option must be passed to both the compiler and linker to
      # build code that uses the CFG exploit mitigation technique.
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_CONTROL_FLOW}>:/guard:cf>
      # Causes the compiler to generate a sorted list of the relative virtual
      # addresses (RVA) of all the valid exception handling continuation targets
      # for a binary.  It's used during runtime for NtContinue and
      # SetThreadContext instruction pointer validation.
      #
      # The /guard:ehcont option must be passed to both the compiler and linker
      # to generate EH continuation target RVAs for a binary.
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
      # Causes the compiler to package global data in individual COMDAT sections
      # for optimization.
      /Gw
      # When /hotpatch is used in a compilation, the compiler ensures that first
      # instruction of each function is at least two bytes, which is required
      # for hot patching.
      #
      # To complete the preparation for making an image hotpatchable, after you
      # use /hotpatch to compile, you must use /FUNCTIONPADMIN to link.
      $<$<BOOL:${WB_MSVC_CREATE_HOTPATCHABLE_IMAGE}>:/hotpatch>
      # Replaces some function calls with intrinsic or otherwise special forms
      # of the function that help your application run faster.  /Oi is only a
      # request to the compiler to replace some function calls with intrinsics;
      # the compiler may call the function (and not replace the function call
      # with an intrinsic) if it will result in better performance.
      #
      # Using the true intrinsic forms implies loss of IEEE exception handling,
      # and loss of _matherr and errno functionality; the latter implies loss of
      # ANSI conformance.
      $<$<BOOL:${WB_MSVC_ENABLE_INTRINSICS}>:/Oi>

      ## C++ standard conformance.
      # Selects strict volatile semantics as defined by the ISO-standard C++
      # language.  Acquire/release semantics are not guaranteed on volatile
      # accesses.
      /volatile:iso
      # Enable the __cplusplus macro to report the supported standard.
      /Zc:__cplusplus
      # Conform to the C++ standard and allow external linkage for constexpr
      # variables.
      /Zc:externConstexpr
      # Disallow temporaries from binding to non-const lvalue references.
      /Zc:referenceBinding
      # Enforce the standard rules for explicit type conversion.
      /Zc:rvalueCast
      # Enable implicit noexcept specifications where required, such as
      # destructors.
      /Zc:implicitNoexcept
      # Have the compiler eliminate unreferenced COMDAT functions and data
      # before emitting the object file.
      /Zc:inline
      # Don't allow conversion from a string literal to mutable characters.
      /Zc:strictStrings
      # Enable thread-safe function-local statics initialization.
      /Zc:threadSafeInit
      # Assume operator new throws on failure.
      /Zc:throwingNew
      # Be mean, don't allow bad non-standard stuff (C++/CLI, __declspec, etc.
      # are all left intact).
      /permissive-

      # Enables recommended Security Development Lifecycle (SDL) checks.
      $<$<BOOL:${WB_MSVC_ENABLE_ADDITIONAL_SECURITY_CHECKS}>:/sdl>
      /std:${WB_MSVC_CXX_LANGUAGE_VERSION} # Build in the requested version of C++

      # Set source character set and the execution character set as UTF-8.
      /utf-8

    PRIVATE
      /bigobj # Support objects with > 65k sections. Needed due to templates.
      # Architecture to prefer when generating code.
      /favor:${WB_MSVC_FAVORED_CPU_ARCHITECTURE}
      # Minimum architecture (instruction sets) to use when generating code.
      $<$<BOOL:${WB_MSVC_MINIMUM_CPU_ARCHITECTURE}>:/arch:${WB_MSVC_MINIMUM_CPU_ARCHITECTURE}>
      # Specify floating-point behavior.
      ######## /fp:${WB_MSVC_FLOATING_POINT_BEHAVIOR}
      # Specify floating-point exceptions behavior.
      /fp:${WB_MSVC_FLOATING_POINT_EXCEPTIONS_BEHAVIOR}

      # Enable all warnings if requested or set 4 (highest) level for warnings.
      $<IF:$<BOOL:${WB_MSVC_ENABLE_ALL_WARNINGS}>,/Wall,W4>
      # Enable all warnings if requested.
      $<$<BOOL:${WB_MSVC_ENABLE_ALL_WARNINGS}>:/Wall>
      # Treats all warnings as errors if requested.
      $<$<BOOL:${WB_MSVC_THREAT_COMPILER_WARNINGS_AS_ERRORS}>:/WX>
      # Enable multi-processor compilation if requested.
      $<$<BOOL:${WB_MSVC_ENABLE_PARALLEL_BUILD}>:/MP>
      # Enable static analysis if requested.
      $<$<BOOL:${WB_MSVC_STATIC_CODE_ANALYSIS_MODE}>:${WB_MSVC_STATIC_CODE_ANALYSIS_MODE}>
      # Threat static analyser warnings as errors.
      $<$<NOT:$<BOOL:${WB_MSVC_THREAT_STATIC_CODE_ANALYSIS_WARNINGS_AS_ERRORS}>>:/analyze:WX->

      ## Debug builds.
      $<$<CONFIG:DEBUG>:
        /Gy- # Disable function level linking.

        # /RTCc reports when a value is assigned to a smaller data type and
        # results in a data loss.  Rejects code that conforms to the standard.
        # You can define _ALLOW_RTCc_IN_STL to silence the warning and use the
        # /RTCc option.
        # /RTCs enables stack frame run-time error checking.
        # /RTCu reports when a variable is used without having been initialized.
        /RTC${WB_MSVC_DEBUG_RUNTIME_ERROR_CHECKS}

        # Add /Ob2 if allowing inlining in debug mode.
        $<$<BOOL:${WB_MSVC_ENABLE_DEBUG_INLINING}>:/Ob2>
      >

      ## Non-debug builds.
      $<$<NOT:$<CONFIG:DEBUG>>:
        /Gy   # Enable function level linking. (-ffunction-sections)
        /Qpar # Enable parallel code generation.
        /Ot   # Favor fast code.

        $<$<BOOL:${WB_MSVC_ENABLE_LTCG}>:/GL> # Enable link time code generation.
      >
  )

  target_compile_options(${THE_TARGET}
    PUBLIC
      ## Warnings to disable:
      # Please, check _STL_EXTRA_DISABLED_WARNINGS can be applied first for
      # system header warnings.
      # C4514: 'function' : unreferenced inline function has been removed.
      # C4710: 'function': function not inlined.
      # C5045: Compiler will insert Spectre mitigation for memory load if
      #        /Qspectre switch specified.
      $<$<BOOL:${WB_MSVC_ENABLE_ALL_WARNINGS}>:/wd4514;/wd4710;/wd5045>

      ## Warnings to treat as errors:
      # Mixed use of struct and class on same type names.
      /we4099
      # Unknown escape sequence. This is usually caused by incorrect escaping.
      /we4129
      # 'operator/operation' : unsafe conversion from 'type of expression' to
      # 'type required'.
      /we4191
      # 'identifier' : conversion from 'type1' to 'type2', possible loss of
      # data.
      /we4242
      # 'function' : member function does not override any base class virtual
      # member function.
      /we4263
      # 'virtual_function' : no override available for virtual member function
      # from base 'class'; function is hidden.
      /we4264
      # 'class' : class has virtual functions, but destructor is not virtual.
      /we4265
      # 'function' : no override available for virtual member function from base
      # 'type'; function is hidden.
      /we4266
      # 'conversion' : truncation from 'type 1' to 'type 2'.
      /we4302
      # Character cannot be represented in current charset.  This is remidied by
      # prefixing string with "u8".
      /we4566
      # Conversion from 'type1 ' to 'type_2' is sign-extended.  This may cause
      # unexpected runtime behavior.
      /we4826
      # Wide string literal cast to 'LPSTR'.
      /we4905
      # String literal cast to 'LPWSTR'.
      /we4906
      # Illegal copy-initialization; more than one user-defined conversion has
      # been implicitly applied.
      /we4928

    PRIVATE
      ## Warnings disabled for /analyze
      $<$<BOOL:${WB_MSVC_STATIC_CODE_ANALYSIS_MODE}>:
      >
  )

  # And the extra defines:
  target_compile_definitions(${THE_TARGET}
    PUBLIC
      $<$<NOT:$<BOOL:${WB_MSVC_ENABLE_ADDITIONAL_SECURITY_CHECKS}>>:
        # Don't deprecate posix names of functions.
        _CRT_NONSTDC_NO_WARNINGS
        # Don't deprecate the non _s versions of various standard library
        # functions, because safety is for chumps.
        _CRT_SECURE_NO_WARNINGS
        # Don't deprecate the non _s versions of various standard library
        # functions, because safety is for chumps.
        _SCL_SECURE_NO_WARNINGS
      >

      # A type with an extended alignment in VS 15.8 or later.
      _ENABLE_EXTENDED_ALIGNED_STORAGE

      $<$<BOOL:${WB_MSVC_ENABLE_ADDITIONAL_SECURITY_CHECKS}>:
        __STDC_WANT_SECURE_LIB__  # Need secure stdc lib.
      >

      # Enable automatic replacement of insecure CRT functions with secure
      # templates.
      $<$<BOOL:WB_MSVC_USE_SECURE_CRT_OVERLOAD_STANDARD_NAMES>:
        _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
        _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1
      >

      ## Disable warnings only inside STL.
      # 4514 Unreferenced inline function has been removed.
      # 4710 Function not inlined.
      # 4820 Padding added after data member.
      _UCRT_EXTRA_DISABLED_WARNINGS=4514\ 4710\ 4820

      # UTF-8 TCHARS and APIs are default.
      $<$<BOOL:${WB_MSVC_USE_UTF16_WINAPI_INSTEAD_OF_ANSI}>:
        _UNICODE
        UNICODE
      >

      # Don't include most of Windows.h
      $<$<BOOL:${WB_MSVC_ENABLE_LEAN_AND_MEAN_WINDOWS}>:WIN32_LEAN_AND_MEAN>

      $<$<CONFIG:DEBUG>:
        _ALLOW_RTCc_IN_STL  # Allow to build STL with /RTCc
      >
  )

  # Ignore a warning about an object file not defining any symbols,
  # these are known, and we don't care.
  set_property(TARGET ${THE_TARGET} APPEND_STRING PROPERTY STATIC_LIBRARY_FLAGS " /ignore:4221")

  target_link_options(${THE_TARGET}
    PRIVATE
      # Mark an executable image as compatible with Control-flow Enforcement
      # Technology (CET) Shadow Stack.
      /CETCOMPAT
      # Prepares an image for hotpatching.
      $<$<BOOL:${WB_MSVC_CREATE_HOTPATCHABLE_IMAGE}>:/FUNCTIONPADMIN>
      # Causes the linker to analyze control flow for indirect call targets at
      # compile time, and then to insert code to verify the targets at runtime.
      #
      # The /guard:cf option must be passed to both the compiler and linker to
      # build code that uses the CFG exploit mitigation technique.
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_CONTROL_FLOW}>:/guard:cf>
      # Causes the compiler to generate a sorted list of the relative virtual
      # addresses (RVA) of all the valid exception handling continuation targets
      # for a binary.  It's used during runtime for NtContinue and
      # SetThreadContext instruction pointer validation.
      #
      # The /guard:ehcont option must be passed to both the compiler and linker
      # to generate EH continuation target RVAs for a binary.
      $<$<BOOL:${WB_MSVC_ENABLE_GUARD_FOR_EH_CONTINUATION}>:/guard:ehcont>
      # Enable address space layout randomization.
      /DYNAMICBASE
      # Support high-entropy 64-bit address space layout randomization (ASLR).
      /HIGHENTROPYVA
      # Compatibility with Windows Data Execution Prevention feature.
      /NXCOMPAT
      # Treats linker warnings as errors.
      /WX

      ## Debug builds.
      $<$<CONFIG:DEBUG>:
        /OPT:NOREF,NOICF  # No (unreferenced data delete + same COMDAT folding).
        /INCREMENTAL      # Do incremental linking.
        # Generate a partial PDB file that simply references the original object
        # and library files.
        $<$<BOOL:${WB_MSVC_ENABLE_FAST_LINK}>:/DEBUG:FASTLINK>
      >

      ## Non-debug builds.
      $<$<NOT:$<CONFIG:DEBUG>>:
        # Add /GL to the compiler, and /LTCG to the linker if link time code
        # generation is enabled.
        $<$<BOOL:${WB_MSVC_ENABLE_LTCG}>:/LTCG>
        # Incremental linking is not supported with LTCG.
        $<$<BOOL:${WB_MSVC_ENABLE_LTCG}>:/INCREMENTAL:NO>
         # Remove unreferenced functions and data + identical COMDAT folding.
        /OPT:REF,ICF
      >
  )

  wb_dump_target_property(${THE_TARGET} COMPILE_OPTIONS     "cxx compiler   flags")
  wb_dump_target_property(${THE_TARGET} COMPILE_DEFINITIONS "cxx compiler defines")
  wb_dump_target_property(${THE_TARGET} LINK_OPTIONS        "cxx linker     flags")
  
  if (APPLY_CLANG_TIDY)
    wb_dump_target_property(${THE_TARGET} CXX_CLANG_TIDY    "cxx clang-tidy flags")
  else()
    message(STATUS "${THE_TARGET} cxx clang-tidy flags: not applied on MSVC.")
  endif()

  message("--")
endfunction()
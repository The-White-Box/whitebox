// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.
//
// This file adds defines about the platform we're currently building on.
//  Operating System:
//    WB_OS_WIN / WB_OS_MACOS / WB_OS_LINUX / WB_OS_POSIX (MACOS or LINUX)
//    is set by the build system
//  Compiler:
//    WB_COMPILER_MSVC / WB_COMPILER_GCC / WB_COMPILER_CLANG
//  Processor:
//    WB_ARCH_CPU_X86_64 / WB_ARCH_CPU_64_BITS

#ifndef WB_BUILD_BUILD_CONFIG_H_
#define WB_BUILD_BUILD_CONFIG_H_

// A set of macroses to use for platform detection.
#if defined(ANDROID)
// Android OS.
#define WB_OS_ANDROID 1
#elif defined(__APPLE__)
// only include TargetConditions after testing ANDROID as some android builds
// on mac don't have this header available and it's not needed unless the target
// is really mac/ios.
#include <TargetConditionals.h>
// Mac OS.
#define WB_OS_MACOS 1
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
// iOS.
#define WB_OS_IOS 1
#endif  // defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#elif defined(__linux__)
// Linux based OS.
#define WB_OS_LINUX 1
// Include a system header to pull in features.h for glibc/uclibc macros.
#include <unistd.h>
#if defined(__GLIBC__) && !defined(__UCLIBC__)
// We really are using glibc, not uClibc pretending to be glibc.
#define WB_LIBC_GLIBC 1
#endif
#elif defined(_WIN32)
// Windows OS.
#define WB_OS_WIN 1
#elif defined(__FreeBSD__)
// FreeBSD OS.
#define WB_OS_FREEBSD 1
#elif defined(__NetBSD__)
// NetBSD OS.
#define WB_OS_NETBSD 1
#elif defined(__OpenBSD__)
// OpenBSD OS.
#define WB_OS_OPENBSD 1
#elif defined(__sun)
// Solaris OS.
#define WB_OS_SOLARIS 1
#else
#error "Please add support for your platform."
#endif
// NOTE: Adding a new port?  Please follow
// https://github.com/The-White-Box/whitebox/blob/master/docs/new_port_policy.md

// For access to standard BSD features, use WB_OS_BSD instead of a
// more specific macro.
#if defined(WB_OS_FREEBSD) || defined(WB_OS_NETBSD) || defined(WB_OS_OPENBSD)
// *BSD OS.
#define WB_OS_BSD 1
#endif

#if defined(WB_OS_AIX) || defined(WB_OS_ANDROID) || defined(WB_OS_FREEBSD) || \
    defined(WB_OS_LINUX) || defined(WB_OS_MACOS) || defined(WB_OS_NETBSD) ||  \
    defined(WB_OS_OPENBSD) || defined(WB_OS_QNX) || defined(WB_OS_SOLARIS)
// For access to standard POSIXish features, use WB_OS_POSIX instead of a
// more specific macro.
#define WB_OS_POSIX 1
#endif

#if defined(WB_OS_WIN) || defined(WB_OS_POSIX)
// Use mi-malloc.
#define WB_MI_MALLOC 1
#endif

// Compiler detection.
#if defined(__clang__)
// Clang.
#define WB_COMPILER_CLANG 1
#elif defined(__GNUC__)
// GCC.
#define WB_COMPILER_GCC 1
#elif defined(_MSC_VER)
// MSVC.
#define WB_COMPILER_MSVC 1
#else
#error "Please add support for your platform."
#endif

// Processor architecture detection.  For more info on what's defined, see:
//   https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//   https://www.agner.org/optimize/calling_conventions.pdf
// or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64)
#define WB_ARCH_CPU_X86_FAMILY 1
#define WB_ARCH_CPU_X86_64 1
#define WB_ARCH_CPU_64_BITS 1
#define WB_ARCH_CPU_LITTLE_ENDIAN 1
#elif (defined(__PPC64__) || defined(__PPC__)) && defined(__BIG_ENDIAN__)
#define WB_ARCH_CPU_PPC64_FAMILY 1
#define WB_ARCH_CPU_PPC64 1
#define WB_ARCH_CPU_64_BITS 1
#define WB_ARCH_CPU_BIG_ENDIAN 1
#elif defined(__PPC64__)
#define WB_ARCH_CPU_PPC64_FAMILY 1
#define WB_ARCH_CPU_PPC64 1
#define WB_ARCH_CPU_64_BITS 1
#define WB_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__)
#define WB_ARCH_CPU_ARM_FAMILY 1
#define WB_ARCH_CPU_ARM64 1
#define WB_ARCH_CPU_64_BITS 1
#define WB_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEL__) && defined(__LP64__)
#define WB_ARCH_CPU_MIPS_FAMILY 1
#define WB_ARCH_CPU_MIPS64EL 1
#define WB_ARCH_CPU_64_BITS 1
#define WB_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEB__) && defined(__LP64__)
#define WB_ARCH_CPU_MIPS_FAMILY 1
#define WB_ARCH_CPU_MIPS64 1
#define WB_ARCH_CPU_64_BITS 1
#define WB_ARCH_CPU_BIG_ENDIAN 1
#else
#error "Please add support for your CPU architecture."
#endif

// Type detection for wchar_t.
#if defined(WB_OS_WIN)
#define WB_WCHAR_T_IS_UTF16
#elif defined(WB_OS_POSIX) &&                                   \
    (defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)) && \
    defined(__WCHAR_MAX__) &&                                   \
    (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#define WB_WCHAR_T_IS_UTF32
#elif defined(WB_OS_POSIX) &&                                   \
    (defined(WB_COMPILER_GCC) || defined(WB_COMPILER_CLANG)) && \
    defined(__WCHAR_MAX__) &&                                   \
    (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
// On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
// compile in this mode.  This is intended for other projects using base who
// manage their own dependencies and make sure short wchar works for them.
#define WB_WCHAR_T_IS_UTF16
#else
#error "Please add support for wchar_t type on your platform."
#endif

#endif  // !WB_BUILD_BUILD_CONFIG_H_
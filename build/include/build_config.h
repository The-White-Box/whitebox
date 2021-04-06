// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a 3-Clause BSD license that can be
// found in the LICENSE file.

// This file adds defines about the platform we're currently building on.
//  Operating System:
//    WHITEBOX_OS_WIN / WHITEBOX_OS_MACOSX / WHITEBOX_OS_LINUX /
//    WHITEBOX_OS_POSIX (MACOSX or LINUX) / WHITEBOX_OS_NACL (NACL_SFI or
//    NACL_NONSFI) / WHITEBOX_OS_NACL_SFI / WHITEBOX_OS_NACL_NONSFI OS_CHROMEOS
//    is set by the build system
//  Compiler:
//    WHITEBOX_COMPILER_MSVC / WHITEBOX_COMPILER_GCC
//  Processor:
//    WHITEBOX_ARCH_CPU_X86 / WHITEBOX_ARCH_CPU_X86_64 /
//    WHITEBOX_ARCH_CPU_X86_FAMILY (X86 or X86_64) WHITEBOX_ARCH_CPU_32_BITS /
//    WHITEBOX_ARCH_CPU_64_BITS

#ifndef WHITEBOX_BUILD_INCLUDE_BUILD_CONFIG_H_
#define WHITEBOX_BUILD_INCLUDE_BUILD_CONFIG_H_

// A set of macros to use for platform detection.
#if defined(__native_client__)
// __native_client__ must be first, so that other OS_ defines are not set.
#define WHITEBOX_OS_NACL 1
// WHITEBOX_OS_NACL comes in two sandboxing technology flavors, SFI or Non-SFI.
// PNaCl toolchain defines __native_client_nonsfi__ macro in Non-SFI build
// mode, while it does not in SFI build mode.
#if defined(__native_client_nonsfi__)
#define WHITEBOX_OS_NACL_NONSFI
#else
#define WHITEBOX_OS_NACL_SFI
#endif
#elif defined(ANDROID)
#define WHITEBOX_OS_ANDROID 1
#elif defined(__APPLE__)
// only include TargetConditions after testing ANDROID as some android builds
// on mac don't have this header available and it's not needed unless the target
// is really mac/ios.
#include <TargetConditionals.h>
#define WHITEBOX_OS_MACOSX 1
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define WHITEBOX_OS_IOS 1
#endif  // defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#elif defined(__linux__)
#define WHITEBOX_OS_LINUX 1
// include a system header to pull in features.h for glibc/uclibc macros.
#include <unistd.h>
#if defined(__GLIBC__) && !defined(__UCLIBC__)
// we really are using glibc, not uClibc pretending to be glibc
#define WHITEBOX_LIBC_GLIBC 1
#endif
#elif defined(_WIN32)
#define WHITEBOX_OS_WIN 1
#elif defined(__FreeBSD__)
#define WHITEBOX_OS_FREEBSD 1
#elif defined(__NetBSD__)
#define WHITEBOX_OS_NETBSD 1
#elif defined(__OpenBSD__)
#define WHITEBOX_OS_OPENBSD 1
#elif defined(__sun)
#define WHITEBOX_OS_SOLARIS 1
#elif defined(__QNXNTO__)
#define WHITEBOX_OS_QNX 1
#elif defined(_AIX)
#define WHITEBOX_OS_AIX 1
#else
#error Please add support for your platform in build/build_config.h
#endif
// NOTE: Adding a new port? Please follow
// https://chromium.googlesource.com/chromium/src/+/master/docs/new_port_policy.md

#if defined(USE_OPENSSL_CERTS) && defined(USE_NSS_CERTS)
#error Cannot use both OpenSSL and NSS for certificates
#endif

// For access to standard BSD features, use WHITEBOX_OS_BSD instead of a
// more specific macro.
#if defined(WHITEBOX_OS_FREEBSD) || defined(WHITEBOX_OS_NETBSD) || \
    defined(WHITEBOX_OS_OPENBSD)
#define WHITEBOX_OS_BSD 1
#endif

// For access to standard POSIXish features, use WHITEBOX_OS_POSIX instead of a
// more specific macro.
#if defined(WHITEBOX_OS_AIX) || defined(WHITEBOX_OS_ANDROID) ||    \
    defined(WHITEBOX_OS_FREEBSD) || defined(WHITEBOX_OS_LINUX) ||  \
    defined(WHITEBOX_OS_MACOSX) || defined(WHITEBOX_OS_NACL) ||    \
    defined(WHITEBOX_OS_NETBSD) || defined(WHITEBOX_OS_OPENBSD) || \
    defined(WHITEBOX_OS_QNX) || defined(WHITEBOX_OS_SOLARIS)
#define WHITEBOX_OS_POSIX 1
#endif

// Use tcmalloc
#if (defined(WHITEBOX_OS_WIN) || defined(WHITEBOX_OS_LINUX) || \
     defined(WHITEBOX_OS_ANDROID)) &&                          \
    !defined(NO_TCMALLOC)
#define WHITEBOX_USE_TCMALLOC 1
#endif

// Compiler detection.
#if defined(__GNUC__)
#define WHITEBOX_COMPILER_GCC 1
#elif defined(_MSC_VER)
#define WHITEBOX_COMPILER_MSVC 1
#elif defined(__clang__)
#define WHITEBOX_COMPILER_CLANG 1
#else
#error Please add support for your compiler in build/build_config.h
#endif

// Processor architecture detection. For more info on what's defined, see:
//   https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//   https://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_AMD64)
#define WHITEBOX_ARCH_CPU_X86_FAMILY 1
#define WHITEBOX_ARCH_CPU_X86_64 1
#define WHITEBOX_ARCH_CPU_64_BITS 1
#define WHITEBOX_ARCH_CPU_LITTLE_ENDIAN 1
#elif (defined(__PPC64__) || defined(__PPC__)) && defined(__BIG_ENDIAN__)
#define WHITEBOX_ARCH_CPU_PPC64_FAMILY 1
#define WHITEBOX_ARCH_CPU_PPC64 1
#define WHITEBOX_ARCH_CPU_64_BITS 1
#define WHITEBOX_ARCH_CPU_BIG_ENDIAN 1
#elif defined(__PPC64__)
#define WHITEBOX_ARCH_CPU_PPC64_FAMILY 1
#define WHITEBOX_ARCH_CPU_PPC64 1
#define WHITEBOX_ARCH_CPU_64_BITS 1
#define WHITEBOX_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__)
#define WHITEBOX_ARCH_CPU_ARM_FAMILY 1
#define WHITEBOX_ARCH_CPU_ARM64 1
#define WHITEBOX_ARCH_CPU_64_BITS 1
#define WHITEBOX_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEL__) && defined(__LP64__)
#define WHITEBOX_ARCH_CPU_MIPS_FAMILY 1
#define WHITEBOX_ARCH_CPU_MIPS64EL 1
#define WHITEBOX_ARCH_CPU_64_BITS 1
#define WHITEBOX_ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEB__) && defined(__LP64__)
#define WHITEBOX_ARCH_CPU_MIPS_FAMILY 1
#define WHITEBOX_ARCH_CPU_MIPS64 1
#define WHITEBOX_ARCH_CPU_64_BITS 1
#define WHITEBOX_ARCH_CPU_BIG_ENDIAN 1
#else
#error Please add support for your architecture in build/build_config.h
#endif

// Type detection for wchar_t.
#if defined(WHITEBOX_OS_WIN)
#define WHITEBOX_WCHAR_T_IS_UTF16
#elif defined(WHITEBOX_OS_POSIX) && defined(WHITEBOX_COMPILER_GCC) && \
    defined(__WCHAR_MAX__) &&                                         \
    (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#define WHITEBOX_WCHAR_T_IS_UTF32
#elif defined(WHITEBOX_OS_POSIX) && defined(WHITEBOX_COMPILER_GCC) && \
    defined(__WCHAR_MAX__) &&                                         \
    (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
// On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
// compile in this mode (in particular, Chrome doesn't). This is intended for
// other projects using base who manage their own dependencies and make sure
// short wchar works for them.
#define WHITEBOX_WCHAR_T_IS_UTF16
#else
#error Please add support for your wchar_t in build/build_config.h
#endif

#endif  // !WHITEBOX_BUILD_INCLUDE_BUILD_CONFIG_H_
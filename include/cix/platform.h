// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once


// known compilers
// only one of those will be redefined to (1) by the detection logic below
#define CIX_COMPILER_CLANG  0
#define CIX_COMPILER_GCC    0
#define CIX_COMPILER_INTEL  0
#define CIX_COMPILER_MSVC   0


// compiler detection
// CAUTION: test order matters since intel's compiler also defines __GNUC__ and
// _MSC_VER macros
#if defined(__INTEL_COMPILER)
    #ifndef __GNUC_PATCHLEVEL__
        #define __GNUC_PATCHLEVEL__  0
    #endif
    #undef CIX_COMPILER_INTEL
    #define CIX_COMPILER_INTEL    1
    #define CIX_COMPILER_NAME     "intel"
    #define CIX_COMPILER_VERSION  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)  // MMmmPP
    #define CIX_FUNCTION          __func__
    #define CIX_FILE              __BASE_FILE__
    #define CIX_LINE              __LINE__
#elif defined(__clang__)
    #undef CIX_COMPILER_CLANG
    #define CIX_COMPILER_CLANG    1
    #define CIX_COMPILER_NAME     "clang"
    #define CIX_COMPILER_VERSION  (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)  // MMmmPP
    #define CIX_FUNCTION          __func__
    #define CIX_FILE              __BASE_FILE__
    #define CIX_LINE              __LINE__
#elif defined(__GNUC__)
    #ifndef __GNUC_PATCHLEVEL__
        #define __GNUC_PATCHLEVEL__  0
    #endif
    #undef CIX_COMPILER_GCC
    #define CIX_COMPILER_GCC      1
    #define CIX_COMPILER_NAME     "gcc"
    #define CIX_COMPILER_VERSION  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)  // MMmmPP
    #define CIX_FUNCTION          __func__  // __PRETTY_FUNCTION__
    #define CIX_FILE              __BASE_FILE__
    #define CIX_LINE              __LINE__
#elif defined(_MSC_VER)
    // * MSVC++ 17.0 _MSC_VER >= 1930 (Visual Studio 2022)
    // * MSVC++ 14.2 _MSC_VER >= 1920 (Visual Studio 2019)
    // * MSVC++ 14.1 _MSC_VER >= 1910 (Visual Studio 2017)
    // * MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
    // * MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
    // * MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
    // * MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
    // * MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
    // * MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
    // * MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
    // * MSVC++ 7.0  _MSC_VER == 1300
    // * MSVC++ 6.0  _MSC_VER == 1200
    // * MSVC++ 5.0  _MSC_VER == 1100
    #undef CIX_COMPILER_MSVC
    #define CIX_COMPILER_MSVC     1
    #define CIX_COMPILER_NAME     "msvc"
    #define CIX_COMPILER_VERSION  _MSC_VER
    #define CIX_FUNCTION          __func__
    #define CIX_FILE              __FILE__
    #define CIX_LINE              __LINE__
#else
    #error unknown compiler
#endif



// known platforms
// one or multiple of those values will be redefined to (1) by the detection
// logic below

// unix common groups
#define CIX_PLATFORM_UNIX   0
#define CIX_PLATFORM_POSIX  0
#define CIX_PLATFORM_LINUX  0

// linux distros group
#define CIX_PLATFORM_ANDROID  0

// bsd group
#define CIX_PLATFORM_DRAGONFLY  0
#define CIX_PLATFORM_FREEBSD    0
#define CIX_PLATFORM_NETBSD     0
#define CIX_PLATFORM_OPENBSD    0

// windows group
#define CIX_PLATFORM_WINDOWS  0
#define CIX_PLATFORM_CYGWIN   0
#define CIX_PLATFORM_MINGW    0

// apple group
#define CIX_PLATFORM_MACOS          0
#define CIX_PLATFORM_IOS            0
#define CIX_PLATFORM_IOS_SIMULATOR  0

// other unix-like group
#define CIX_PLATFORM_HPUX     0
#define CIX_PLATFORM_SOLARIS  0
#define CIX_PLATFORM_AIX      0


// platform detection
// CAUTION: test order matters
#if defined(__MINGW32__)  // || defined(__MINGW64__)
    #undef CIX_PLATFORM_MINGW
    #define CIX_PLATFORM_MINGW  1
    #define CIX_PLATFORM_NAME   "mingw"
#elif defined(_WIN32)
    #undef CIX_PLATFORM_WINDOWS
    #define CIX_PLATFORM_WINDOWS  1
    #define CIX_PLATFORM_NAME   "windows"
#elif defined(__CYGWIN__)
    #undef CIX_PLATFORM_CYGWIN
    #define CIX_PLATFORM_CYGWIN  1
    #define CIX_PLATFORM_NAME    "cygwin"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        #undef CIX_PLATFORM_IOS_SIMULATOR
        #define CIX_PLATFORM_IOS_SIMULATOR  1
        #define CIX_PLATFORM_NAME           "iossim"
    #elif TARGET_OS_IPHONE
        #undef CIX_PLATFORM_IOS
        #define CIX_PLATFORM_IOS   1
        #define CIX_PLATFORM_NAME  "ios"
    #elif TARGET_OS_MAC
        #undef CIX_PLATFORM_MACOS
        #define CIX_PLATFORM_MACOS  1
        #define CIX_PLATFORM_NAME   "macos"
    #else
        #error unknown apple platform
    #endif
#elif defined(__ANDROID__)
    #undef CIX_PLATFORM_ANDROID
    #define CIX_PLATFORM_ANDROID  1
    #define CIX_PLATFORM_NAME     "android"
#elif defined(__sun) && defined(__SVR4)
    #undef CIX_PLATFORM_SOLARIS
    #define CIX_PLATFORM_SOLARIS  1
    #define CIX_PLATFORM_NAME     "solaris"
#elif defined(__hpux)
    #undef CIX_PLATFORM_HPUX
    #define CIX_PLATFORM_HPUX  1
    #define CIX_PLATFORM_NAME  "hp-ux"
#elif defined(__aix)
    #undef CIX_PLATFORM_AIX
    #define CIX_PLATFORM_AIX   1
    #define CIX_PLATFORM_NAME  "aix"
#elif defined(__DragonFly__)
    #undef CIX_PLATFORM_DRAGONFLY
    #define CIX_PLATFORM_DRAGONFLY  1
    #define CIX_PLATFORM_NAME       "dragonfly"
#elif defined(__FreeBSD__)
    #undef CIX_PLATFORM_FREEBSD
    #define CIX_PLATFORM_FREEBSD  1
    #define CIX_PLATFORM_NAME     "freebsd"
#elif defined(__OpenBSD__)
    #undef CIX_PLATFORM_OPENBSD
    #define CIX_PLATFORM_OPENBSD  1
    #define CIX_PLATFORM_NAME     "openbsd"
#elif defined(__NetBSD__)
    #undef CIX_PLATFORM_NETBSD
    #define CIX_PLATFORM_NETBSD  1
    #define CIX_PLATFORM_NAME    "netbsd"
#elif defined(__linux__)
    #undef CIX_PLATFORM_LINUX
    #define CIX_PLATFORM_LINUX  1
    #define CIX_PLATFORM_NAME   "linux"
#else
    #error unknown platform
#endif

#if defined(__linux__)
    #undef CIX_PLATFORM_LINUX
    #define CIX_PLATFORM_LINUX  1
#endif

#if defined(__unix__)
    #undef CIX_PLATFORM_UNIX
    #define CIX_PLATFORM_UNIX  1
#endif

#if defined(_POSIX_VERSION)  // || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
    #undef CIX_PLATFORM_POSIX
    #define CIX_PLATFORM_POSIX  1
#endif



// supported endianness
// * only one of those will be redefined to (1) by the detection logic below
// * std::endian is C++20
// * exotic endianness not supported
// * see also <cix/endian.h>
#define CIX_ENDIAN_LITTLE  0
#define CIX_ENDIAN_BIG     0


// compile-time endianness detection
// detection logic from rapidjson
// CAUTION: any modification here must be reported to endian.h

// detect with GCC 4.6's macro
#ifdef __BYTE_ORDER__
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #undef CIX_ENDIAN_LITTLE
        #define CIX_ENDIAN_LITTLE  1
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #undef CIX_ENDIAN_BIG
        #define CIX_ENDIAN_BIG  1
    #else
        #error unknown endianness
    #endif  // __BYTE_ORDER__

// detect with GLIBC's endian.h
#elif defined(__GLIBC__)
    #include <endian.h>
    #if (__BYTE_ORDER == __LITTLE_ENDIAN)
        #undef CIX_ENDIAN_LITTLE
        #define CIX_ENDIAN_LITTLE  1
    #elif (__BYTE_ORDER == __BIG_ENDIAN)
        #undef CIX_ENDIAN_BIG
        #define CIX_ENDIAN_BIG  1
    #else
        #error unknown endianness
    #endif  // __GLIBC__

// detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
    #undef CIX_ENDIAN_LITTLE
    #define CIX_ENDIAN_LITTLE  1
#elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
    #undef CIX_ENDIAN_BIG
    #define CIX_ENDIAN_BIG  1

// detect with architecture macros
#elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
    #undef CIX_ENDIAN_BIG
    #define CIX_ENDIAN_BIG  1
#elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
    #undef CIX_ENDIAN_LITTLE
    #define CIX_ENDIAN_LITTLE  1
#elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
    #undef CIX_ENDIAN_LITTLE
    #define CIX_ENDIAN_LITTLE  1
// #elif defined(CIX_DOXYGEN_RUNNING)
//     #undef CIX_ENDIAN_LITTLE
//     #define CIX_ENDIAN_LITTLE  1
#else
    #error unknown endianness
#endif

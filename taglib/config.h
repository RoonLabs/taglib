/* config.h. Upstream TagLib generates this file with cmake.  Roon doesn't use cmake,
   so this must be kept up to date by hand. */

#ifndef TAGLIB_CONFIG_H
#define TAGLIB_CONFIG_H

#ifdef __APPLE__
#  include <libkern/OSAtomic.h>
#  define TAGLIB_ATOMIC_MAC
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#  define NOMINMAX
#  include <windows.h>
#  define TAGLIB_ATOMIC_WIN
#elif defined (__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 401)    \
      && (defined(__i386__) || defined(__i486__) || defined(__i586__) || \
          defined(__i686__) || defined(__x86_64) || defined(__ia64)) \
      && !defined(__INTEL_COMPILER)
#  define TAGLIB_ATOMIC_GCC
#elif defined(__ia64) && defined(__INTEL_COMPILER)
#  include <ia64intrin.h>
#  define TAGLIB_ATOMIC_GCC
#elif defined(__GNUC__) && defined(__ARMEL__)
#  define TAGLIB_ATOMIC_GCC
#else
#error Roon will fail without threadsafe taglib!
#endif

#endif

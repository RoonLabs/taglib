#ifndef INCLUDED_ROON_TAGLIB_UTILS_H
#define INCLUDED_ROON_TAGLIB_UTILS_H

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "taglib_export.h"
#include <tfile.h>
#include <tbytevector.h>

using namespace TagLib;

TAGLIB_EXPORT
ByteVector taglib_make_signature(File *f, unsigned long long offset, unsigned long long length);

TAGLIB_EXPORT
ByteVector taglib_make_signature(const ByteVector &bv);

// #ifdef __cplusplus
// }
// #endif

#endif

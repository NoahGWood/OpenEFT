#pragma once

extern "C" {
#include <wsq.h>
#include <util.h>
#include <ihead.h>
}

// Kill NBIS garbage macros
#ifdef BYTE
#undef BYTE
#endif

#ifdef WORD
#undef WORD
#endif

#ifdef DWORD
#undef DWORD
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif
namespace OpenEFT
{
    
} // namespace OpenEFT

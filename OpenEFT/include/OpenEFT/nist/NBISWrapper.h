// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
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

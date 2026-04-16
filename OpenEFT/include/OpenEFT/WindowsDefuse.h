// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
/**
 * @file WindowsDefuse.h
 * @author NoahGWood (@NoahGWood)
 * @brief This file exists because windows is macro hell
 * @version 0.1
 * @date 2026-04-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifdef _WIN32

// Force include windows.h first (so we can clean it)
#include <windows.h>

// Now defuse the landmines
#ifdef ERROR
#undef ERROR
#endif

#ifdef IGNORE
#undef IGNORE
#endif

#ifdef OPTIONAL
#undef OPTIONAL
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#endif
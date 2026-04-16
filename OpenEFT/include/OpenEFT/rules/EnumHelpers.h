// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <string>
#include <string_view>
#include <algorithm>
#include <OpenEFT/WindowsDefuse.h>

// ===== helpers =====

#define ENUM_CASE(NAME, X) case NAME::X: return #X;
#define ENUM_IF(NAME, X) if (tmp == #X) return NAME::X;

#define EXPAND(x) x

#define FOR_EACH_1(W, N, X) W(N, X)
#define FOR_EACH_2(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_1(W, N, __VA_ARGS__))
#define FOR_EACH_3(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_2(W, N, __VA_ARGS__))
#define FOR_EACH_4(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_3(W, N, __VA_ARGS__))
#define FOR_EACH_5(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_4(W, N, __VA_ARGS__))
#define FOR_EACH_6(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_5(W, N, __VA_ARGS__))
#define FOR_EACH_7(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_6(W, N, __VA_ARGS__))
#define FOR_EACH_8(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_7(W, N, __VA_ARGS__))
#define FOR_EACH_9(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_8(W, N, __VA_ARGS__))
#define FOR_EACH_10(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_9(W, N, __VA_ARGS__))
#define FOR_EACH_11(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_10(W, N, __VA_ARGS__))
#define FOR_EACH_12(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_11(W, N, __VA_ARGS__))
#define FOR_EACH_13(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_12(W, N, __VA_ARGS__))
#define FOR_EACH_14(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_13(W, N, __VA_ARGS__))
#define FOR_EACH_15(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_14(W, N, __VA_ARGS__))
#define FOR_EACH_16(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_15(W, N, __VA_ARGS__))
#define FOR_EACH_17(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_16(W, N, __VA_ARGS__))
#define FOR_EACH_18(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_17(W, N, __VA_ARGS__))
#define FOR_EACH_19(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_18(W, N, __VA_ARGS__))
#define FOR_EACH_20(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_19(W, N, __VA_ARGS__))
#define FOR_EACH_21(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_20(W, N, __VA_ARGS__))
#define FOR_EACH_22(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_21(W, N, __VA_ARGS__))
#define FOR_EACH_23(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_22(W, N, __VA_ARGS__))
#define FOR_EACH_24(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_23(W, N, __VA_ARGS__))
#define FOR_EACH_25(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_24(W, N, __VA_ARGS__))
#define FOR_EACH_26(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_25(W, N, __VA_ARGS__))
#define FOR_EACH_27(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_26(W, N, __VA_ARGS__))
#define FOR_EACH_28(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_27(W, N, __VA_ARGS__))
#define FOR_EACH_29(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_28(W, N, __VA_ARGS__))
#define FOR_EACH_30(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_29(W, N, __VA_ARGS__))
#define FOR_EACH_31(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_30(W, N, __VA_ARGS__))
#define FOR_EACH_32(W, N, X, ...) W(N, X) EXPAND(FOR_EACH_31(W, N, __VA_ARGS__))

#define GET_FOR_EACH_MACRO( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
    _31, _32, NAME, ...) NAME

#define FOR_EACH(action, NAME, ...) \
    EXPAND(GET_FOR_EACH_MACRO(__VA_ARGS__, \
        FOR_EACH_32, FOR_EACH_31, FOR_EACH_30, FOR_EACH_29, FOR_EACH_28, FOR_EACH_27, FOR_EACH_26, FOR_EACH_25, \
        FOR_EACH_24, FOR_EACH_23, FOR_EACH_22, FOR_EACH_21, FOR_EACH_20, FOR_EACH_19, FOR_EACH_18, FOR_EACH_17, \
        FOR_EACH_16, FOR_EACH_15, FOR_EACH_14, FOR_EACH_13, FOR_EACH_12, FOR_EACH_11, FOR_EACH_10, FOR_EACH_9, \
        FOR_EACH_8, FOR_EACH_7, FOR_EACH_6, FOR_EACH_5, FOR_EACH_4, FOR_EACH_3, FOR_EACH_2, FOR_EACH_1) \
        (action, NAME, __VA_ARGS__))

// ===== main macro =====

#define DECLARE_ENUM(NAME, UNDERLYING, ...)                          \
    enum class NAME : UNDERLYING { __VA_ARGS__ };                    \
                                                                    \
    inline std::string_view ToString(NAME v) {                       \
        switch (v) {                                                 \
            FOR_EACH(ENUM_CASE, NAME, __VA_ARGS__)                   \
            default: return "UNKNOWN";                               \
        }                                                            \
    }                                                                \
                                                                    \
    inline NAME FromString_##NAME(std::string_view s) {              \
        std::string tmp(s);                                          \
        std::transform(tmp.begin(), tmp.end(), tmp.begin(),          \
                       [](unsigned char c){ return std::toupper(c); }); \
        FOR_EACH(ENUM_IF, NAME, __VA_ARGS__)                         \
        return NAME::UNKNOWN;                                        \
    }
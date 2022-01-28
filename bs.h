/*
   Brief: Basic stuff
   Author: Edgar Araújo <edgararaj@gmail.com>
   Copyright 2021
*/

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <limits>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#undef UNICODE
#include <windows.h>
#include <windowsx.h>

typedef unsigned int uint;
typedef size_t   size;
typedef wchar_t  wchar;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

template <typename T>
i64 MaxNum()
{
	return std::numeric_limits<T>::max();
}

template <typename T>
i64 MinNum()
{
	return std::numeric_limits<T>::lowest();
}

template <typename T>
T SafeTrunc(auto Value)
{
	assert(Value <= MaxNum<T>() && Value >= MinNum<T>());
	return (T)Value;
}

template <typename T>
T Clamp(T Value, i64 Min, i64 Max)
{
	if (Value < Min)
		return (T)Min;
	else if (Value > Max)
		return (T)Max;
	else
		return Value;
}

template <typename T>
T Abs(T Value)
{
	return Value < 0 ? -Value : Value;
}

#define KB(x)  ((x)   * 1024ull)
#define MB(x)  (KB(x) * 1024ull)
#define GB(x)  (MB(x) * 1024ull)
#define TB(x)  (GB(x) * 1024ull)

#define ARR_COUNT(x) (sizeof(x)/sizeof((x)[0]))

const wchar* FindTitle(const wchar Path[MAX_PATH])
{
	const wchar* Start = 0;
	for (auto Char = Path; *Char; Char++)
	{
		if (*Char == '\\' || *Char == '/')
			Start = Char + 1;
	}

	return Start ? Start : Path;
}

wchar* DelTitle(wchar Path[MAX_PATH])
{
	wchar* LastSlash = 0;
	for (auto Char = Path; *Char; Char++)
	{
		if (*Char == '\\' || *Char == '/')
			LastSlash = Char;
	}
	if (LastSlash)
		LastSlash[1] = 0;

	return Path;
}

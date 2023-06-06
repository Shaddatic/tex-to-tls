#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <wchar.h>
#include <stdbool.h>

typedef signed __int32	sint32;
typedef signed __int16	sint16;
typedef signed __int8	sint8;

typedef unsigned __int8 byte;

enum TexFileType
{
	FT_ERR,
	FT_PAK,
	FT_PVM,
	FT_UPVM,
	FT_GVM,
	FT_PVMX,
	FT_XVM,
};

enum TexFileMode
{
	FM_NO_NAMES,
	FM_USE_NAMES,
	FM_USE_VOID
};

byte*	LoadTexFile(wchar_t* fn, size_t* rSize, int* rType);

void	Pause();

void	GetFileName(const wchar_t* pWidePath, char* pBuf);
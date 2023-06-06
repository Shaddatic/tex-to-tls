#include <tex-to-tls.h>

#include <stdio.h>
#include <stdlib.h>

static int
GetFileType(char* fbuf)
{
	static const char pak_test[] = { (char)0x01, 'p', 'a', 'k', '\0' };
	static const char pvm_test[] = { (char)0xFF, 'P', 'V', 'M', 'H', '\0' };

	if (strstr(fbuf, pak_test) == fbuf)
		return FT_PAK;

	if (strstr(fbuf, pvm_test) == fbuf)
		return FT_PVM;

	if (strstr(fbuf, "PVMH") == fbuf)
		return FT_UPVM;

	if (strstr(fbuf, "GVMH") == fbuf)
		return FT_GVM;

	if (strstr(fbuf, "PVMX") == fbuf)
		return FT_PVMX;

	if (strstr(fbuf, "XVMH") == fbuf)
		return FT_XVM;

	return FT_ERR;
}

byte*
LoadTexFile(wchar_t* fn, size_t* retSize, int* retType)
{
	FILE* f = _wfopen(fn, L"r");

	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	byte* fbuf = malloc(fsize);

	if (!fbuf)
	{
		puts("Error:\tmalloc() failed! How?!");
		Pause();
		exit(EXIT_FAILURE);
	}

	fread(fbuf, fsize, 1, f);

	fclose(f);

	int	type = GetFileType(fbuf);

	if (type == FT_ERR)
	{
		puts("Error:\tCouldn't determine file type!");
		Pause();
		exit(EXIT_FAILURE);
	}

	if (retSize)
		*retSize = fsize;

	if (retType)
		*retType = type;

	return fbuf;
}
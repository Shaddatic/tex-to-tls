/*
*   tex-to-tls : Texture to TexList Structure file
* 
*   -Shaddatic
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>

typedef signed __int32	sint32;
typedef signed __int16	sint16;
typedef signed __int8	sint8;

void Pause()
{
	system("pause >nul");
}

void PrintAndPause(const char* prnt)
{
	puts(prnt);
	Pause();
}

char* orcStringConvertTo8(wchar_t* str)
{
	int newlength = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	if (newlength <= 0)
		return nullptr;
	char* newstr = (char*)malloc(newlength);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, newstr, newlength, nullptr, nullptr);
	return newstr;
}

char* orcStringInString(char* str, const char* substr, int strsize)
{
	char* result = nullptr;

	char* firsthit = nullptr;

	for (auto i = 0, j = 0; i < strsize; ++i)
	{
		if (str[i] == substr[j])
		{
			if (!j)
				firsthit = &str[i];

			if (substr[++j] != '\0')
				continue;

			result = firsthit;
			break;
		}
		else
		{
			j = 0;
		}
	}

	return result;
}

const char* voidcaststr = "";

#define FPUTS_TEXNAME(F, NAME)		fprintf(F, "\tTEXN(%s\"%s\"),\n", voidcaststr, NAME)
#define PRINT_TEXNAME(INDEX, NAME)	printf("Texture %i\t==\t\"%s\"\n", INDEX, NAME)

int MakeTLS_PAK(FILE* f, char* fbuf, int fsize, bool names)
{
	int texnum = *((sint32*)&fbuf[0x25]) - 1;

	if (!names)
		return texnum;

	int rhead = 0x41;

	for (auto i = 0; i < texnum; ++i)
	{
		char* nameend = orcStringInString(&fbuf[rhead], ".dds", fsize - rhead);

		if (!nameend)
		{
			puts("Expected .dds not found, possibly corrupt file!");
			continue;
		}

		*nameend = '\0';

		char* namestart;

		for (auto j = -1; ; --j)
		{
			if (nameend[j] == '\\')
			{
				namestart = &nameend[j + 1];
				break;
			}
		}

		FPUTS_TEXNAME(f, namestart);

		PRINT_TEXNAME(i, namestart);

		rhead = (int)(nameend - fbuf) + 5;

		nameend = orcStringInString(&fbuf[rhead], ".dds", fsize - rhead); // PAK files have 2 copies of the name, this is a crude way to skip past it

		if (!nameend)
			puts("Expected .dds not found, possibly corrupt file!");

		rhead = (int)(nameend - fbuf) + 5;
	}

	return texnum;
}

int MakeTLS_PRS(char* fbuf)
{
	int texnum;

	if (fbuf[0x0B] == 0x01)
		texnum = fbuf[0x0C];
	else
		texnum = *((__int16*)&fbuf[0x0C]);

	return texnum;
}

int MakeTLS_UPVM(FILE* f, char* fbuf, int fsize, bool names)
{
	int texnum = *((sint16*)&fbuf[0x0A]);

	if (!names)
		return texnum;

	int rhead = 0x0E;

	for (auto i = 0; i < texnum; ++i)
	{
		fbuf[rhead + 28] = '\0';	// UPVM names might be terminated by 0x01 in some cases; this corrects that

		FPUTS_TEXNAME(f, &fbuf[rhead]);

		PRINT_TEXNAME(i, &fbuf[rhead]);

		rhead += 38;
	}

	return texnum;
}

int MakeTLS_GVM(FILE* f, char* fbuf, int fsize, bool names)
{
	int texnum = *((sint16*)&fbuf[0x0B]);

	if (!names)
		return texnum;

	int rhead = 0x0E;

	for (auto i = 0; i < texnum; ++i)
	{
		fbuf[rhead + 28] = '\0';	// GVM names might be terminated by 0xFF in some cases; this corrects that

		FPUTS_TEXNAME(f, &fbuf[rhead]);

		PRINT_TEXNAME(i, &fbuf[rhead]);

		rhead += 38;
	}

	return texnum;
}

int MakeTLS_PVMX(FILE* f, char* fbuf, int fsize, bool names)
{
	int texnum = 0;

	char* rhead = fbuf;

	while (true)
	{
		rhead = orcStringInString(rhead, ".png", fsize - (rhead - fbuf));

		if (!rhead)
			return texnum;

		*rhead = '\0';

		if (!names)
			continue;

		char* rhead2 = rhead;

		while (true)
		{
			if (*(--rhead2) == 0x02)
				break;
		}

		++rhead2;

		FPUTS_TEXNAME(f, rhead2);

		PRINT_TEXNAME(texnum, rhead2);

		++texnum;
	}
}

int MakeTLS_XVM(char* fbuf)
{
	return *((sint32*)&fbuf[0x08]);
}

enum eTextureFileType
{
	FT_ERR,
	FT_PAK,
	FT_PVM,
	FT_UPVM,
	FT_GVM,
	FT_PVMX,
	FT_XVM,
};

int GetFileType(char* fbuf)
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

[[nodiscard("Dynamic memory allocated")]]
char* GetTexname(wchar_t* fn)
{
	wchar_t wtexname[260] = {};
	int wrhead = 0;

	int rhead = (int)wcslen(fn);	// Head to end

	while (true)	// Reverse to last folder
	{
		if (fn[--rhead] == L'\\')
			break;
	}

	while (true)	// Copy file name into wtexname
	{
		if (fn[++rhead] != L'\0')
			wtexname[wrhead++] = fn[rhead];
		else
			break;
	}

	while (true)	// Snip off file extension from wtexname
	{
		if (!wrhead)	// Something went wrong, throw null error
			return nullptr;

		if (wtexname[--wrhead] == L'.')
		{
			wtexname[wrhead] = '\0';
			break;
		}
	}

	return orcStringConvertTo8(wtexname);	// Convert to UTF-8, or basically ASCII in our case
}

int LoadEntireFile(wchar_t* fn, char** pfbuf)
{
	FILE* f = _wfopen(fn, L"r");

	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char* fbuf = (char*)malloc(fsize);

	if (!fbuf)
	{
		fclose(f);
		*pfbuf = nullptr;
		return 0;
	}

	fread(fbuf, fsize, 1, f);

	fclose(f);

	*pfbuf = fbuf;

	return fsize;
}

int wmain(int argc, wchar_t** argv)
{
	if (argc < 2)
	{
		PrintAndPause("Error:\tDrag a texture file (.pak, .prs, .pvm, .gvm, .pvmx, .xvm) onto this .exe to convert!");
		return 1;
	}

#define EXIT(PRINT, FBUF) if constexpr (FBUF) free(fbuf); PrintAndPause(PRINT); return 1;

	wchar_t* fn = argv[1];

	char* fbuf;
	int fsize = LoadEntireFile(fn, &fbuf);

	if (!fbuf)
	{
		EXIT("Error:\tFile couldn't be loaded!", false);
	}

	int type = GetFileType(fbuf);

	if (type == FT_ERR)
	{
		EXIT("Error:\tCouldn't determine file type!", true);
	}

	char* texname = GetTexname(fn);

	if (!texname)
	{
		EXIT("Error:\tFile type found, but file extension not found!", true);
	}

	int rhead = wcslen(fn);

	while (fn[rhead] != L'.') { --rhead; }

	fn[++rhead] = L't';	// Just swap the extension around to a .tls haha, 
	fn[++rhead] = L'l';	// shouldn't be an issue as long as the input file is actually a texture file
	fn[++rhead] = L's';
	fn[++rhead] = L'\0';

	static const char* typenames[] = { "ERR", "-- PAK Mode --", "-- PVM Mode (Texture name extraction not supported, try an uncompressed PVM) -- ", "-- Uncompressed PVM Mode --", "-- GVM Mode --", "-- PVMX Mode --", "-- XVM Mode (Texture name extraction not supported) -- " };

	printf("\n%s\n\n0:\tNo texture names\n1:\tInclude texture names\n2:\tInclude texture names & cast to (void*)\n\n", typenames[type]);

	int mode;

	while (!scanf("%i", &mode));

	if (mode == 2)
		voidcaststr = "(void*)";

	FILE* f = _wfopen(argv[1], L"w");

	fprintf(f, "TEXTURE_START\n\nTEXTURENAME\ttexture_%s[]\nSTART\n", texname);

	int texnum = 0;

	switch (type) {
	case FT_PAK:
		texnum = MakeTLS_PAK(f, fbuf, fsize, mode);
		break;
	case FT_PVM:
		texnum = MakeTLS_PRS(fbuf);
		mode = 0;
		break;
	case FT_UPVM:
		texnum = MakeTLS_UPVM(f, fbuf, fsize, mode);
		break;
	case FT_GVM:
		texnum = MakeTLS_GVM(f, fbuf, fsize, mode);
		break;
	case FT_PVMX:
		texnum = MakeTLS_PVMX(f, fbuf, fsize, mode);
		break;
	case FT_XVM:
		texnum = MakeTLS_XVM(fbuf);
		mode = 0;
		break;
	}

	if (!mode)	// Rather than have each file type do this exact thing, I just put it here
	{
		for (auto i = 0; i < texnum; ++i)
		{
			fprintf(f, "\tTEXN(NULL),\n");
		}
	}

	fprintf(f, "END\n\nTEXTURELIST\ttexlist_%s[]\nSTART\nTextureList\ttexture_%s,\nTextureNum\t%i,\nEND\n\nTEXTURE_END\n", texname, texname, texnum);

	fclose(f);

	free(fbuf);

	printf("\n%s.tls successfully created with %i %s textures!\n\nPress 'Return' to exit\n", texname, texnum, mode ? "named" : "unnamed");
	Pause();

	return 0;
}

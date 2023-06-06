/*
*   tex-to-tls : Texture to TexList Structure file
* 
*   -Shaddatic
*/

#include <tex-to-tls.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int UserMode;

static char FileName[260];

static char* 
StringInString(const char* str, const char* substr, size_t strsize)
{
	const char* result = NULL;

	const char* firsthit = NULL;

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

	return (char*) result;
}

static void
PrintTexnameEntry(const char* texName)
{
	if (texName && UserMode)
	{
		if (UserMode == FM_USE_VOID)
			printf("\tTEXN ( (void*)\"%s\" ),\n", texName);
		else
			printf("\tTEXN ( \"%s\" ),\n", texName);
	}
	else
	{
		printf("\tTEXN ( NULL ),\n");
	}
}

static void
PrintTexnamelessList(const int nbTex)
{
	for (int i = 0; i < nbTex; ++i)
	{
		PrintTexnameEntry(NULL);
	}
}

static char*
GetNextPakTexture(const char* pReadStart, size_t maxRead)
{
	char* result = StringInString(pReadStart, ".dds", maxRead);

	if (!result)
	{
		puts("Error:\tExpected .dds not found, possibly corrupt file!");
		Pause();
		exit(EXIT_FAILURE);
	}

	return result;
}

static char*
GetPakTextureName(const char* pNameEnd)
{
	const char* result;

	for (int j = -1; ; --j)
	{
		if (pNameEnd[j] == '\\')
		{
			result = &pNameEnd[j + 1];
			break;
		}
	}

	return (char*) result;
}

static int 
PrintTLS_PAK(byte* fBuffer, size_t fSize)
{
	int nbtex = *((sint32*) &fBuffer[0x25]) - 1;

	int rhead = 0x41;

	for (int i = 0; i < nbtex; ++i)
	{
		char* nameend = GetNextPakTexture(&fBuffer[rhead], fSize - (size_t)rhead);

		*nameend = '\0';

		PrintTexnameEntry(GetPakTextureName(nameend));

		rhead = (int)(nameend - fBuffer) + 5;

		/* PAK files have 2 copies of the name, this is a crude way to skip past the copy */

		nameend = GetNextPakTexture(&fBuffer[rhead], (int)fSize - rhead); 

		rhead = (int)(nameend - fBuffer) + 5;
	}

	return nbtex;
}

static int 
PrintTLS_PRS(byte* fBuffer)
{
	int nbtex;

	if (fBuffer[0x0B] == 0x01)
		nbtex = fBuffer[0x0C];
	else
		nbtex = *((sint16*) &fBuffer[0x0C]);

	PrintTexnamelessList(nbtex);

	return nbtex;
}

static int 
PrintTLS_UPVM(byte* fBuffer, size_t fSize)
{
	int nbtex = *((sint16*) &fBuffer[0x0A]);

	int rhead = 0x0E;

	for (int i = 0; i < nbtex; ++i)
	{
		fBuffer[rhead + 28] = '\0';	// UPVM names might be terminated by 0x01 in some cases; this corrects that

		PrintTexnameEntry(&fBuffer[rhead]);

		rhead += 38;
	}

	return nbtex;
}

static int 
PrintTLS_GVM(byte* fBuffer, size_t fSize)
{
	int nbtex = *((sint16*) &fBuffer[0x0B]);

	int rhead = 0x0E;

	for (int i = 0; i < nbtex; ++i)
	{
		fBuffer[rhead + 28] = '\0';	// GVM names might be terminated by 0xFF in some cases; this corrects that

		PrintTexnameEntry(&fBuffer[rhead]);

		rhead += 38;
	}

	return nbtex;
}

static int 
PrintTLS_PVMX(byte* fBuffer, size_t fSize)
{
	int nbtex = 0;

	char* rhead = fBuffer;

	while (1)
	{
		rhead = StringInString(rhead, ".png", fSize - (rhead - fBuffer));

		if (!rhead)
			return nbtex;

		*rhead = '\0';

		char* rhead2 = rhead;

		while (1)
		{
			if (*(--rhead2) == 0x02)
				break;
		}

		++rhead2;

		PrintTexnameEntry(rhead2);

		++nbtex;
	}
}

static int 
PrintTLS_XVM(byte* fBuffer)
{
	int nbtex = *((sint32*) &fBuffer[0x08]);

	PrintTexnamelessList(nbtex);

	return nbtex;
}

static int
GetUserMode()
{
	printf(
		"0:\tNo texture names\t(recommended)\n"
		"1:\tExtract texture names\n"
		"2:\tExtract texture names & add '(void*)'\n\n"
	);

	int umode;

	while (!scanf("%i", &umode));

	printf("\n\n");

	return umode;
}

static void
PrintTLS(int fType, byte* fBuffer, size_t fSize)
{
	printf(
		"TEXTURENAME\t"	"texture_%s[]\n"
		"START\n",

		FileName
	);

	int nbtex = 0;

	switch (fType) {
	case FT_PAK:
		nbtex = PrintTLS_PAK(fBuffer, fSize);
		break;
	case FT_PVM:
		nbtex = PrintTLS_PRS(fBuffer);
		break;
	case FT_UPVM:
		nbtex = PrintTLS_UPVM(fBuffer, fSize);
		break;
	case FT_GVM:
		nbtex = PrintTLS_GVM(fBuffer, fSize);
		break;
	case FT_PVMX:
		nbtex = PrintTLS_PVMX(fBuffer, fSize);
		break;
	case FT_XVM:
		nbtex = PrintTLS_XVM(fBuffer);
		break;
	}

	printf(
		"END\n\n"

		"TEXTURELIST\t"	"texlist_%s[]\n"
		"START\n"
		"TextureList\t"	"texture_%s,\n"
		"TextureNum\t"	"%i,\n"
		"END\n\n",

		FileName,
		FileName,
		nbtex
	);
}

static const char* FileTypeList[] = {
	"-- PAK Mode --",
	"-- PVM Mode (Texture name extraction not supported, try an uncompressed PVM) -- ",
	"-- Uncompressed PVM Mode --",
	"-- GVM Mode --",
	"-- PVMX Mode --",
	"-- XVM Mode (Texture name extraction not supported) -- "
};

int 
wmain(int argc, wchar_t** argv)
{
	if (argc < 2)
	{
		puts("Error:\tDrag & drop a texture file (.pak, .prs, .pvm, .gvm, .pvmx, .xvm) onto this .exe to convert!");
		Pause();
		return 1;
	}

	GetFileName(argv[1], FileName);

	/* Load given texture file and get attributes */

	size_t texbufsize;
	int type;

	byte* texbuffer = LoadTexFile(argv[1], &texbufsize, &type);

	/* Print and get user info */

	printf("%s\n\n", FileTypeList[type - 1]);

	UserMode = GetUserMode();

	/* Print .tls to stdout */

	printf("TEXTURE_START\n\n");

	PrintTLS(type, texbuffer, texbufsize);

	printf("TEXTURE_END\n\n\n");

	/* Print complete, pause for copying */

	printf("Creation of .tls data complete!\n");

	Pause();

	return 0;
}

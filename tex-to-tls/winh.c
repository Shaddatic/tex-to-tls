#include <tex-to-tls.h>

#include <windows.h>

void Pause()
{
	system("pause >nul");
}

void
GetFileName(const wchar_t* pWidePath, char* pBuf)
{
	char localbuf[260];

	int size = WideCharToMultiByte(CP_UTF8, 0, pWidePath, -1, (LPSTR)localbuf, 260, NULL, NULL);

	int sizecpy = size;

	int snipped = 0;

	while (sizecpy--)
	{
		if (localbuf[sizecpy] == '.' && !snipped)
		{
			localbuf[sizecpy] = '\0';
			snipped = 1;
		}
		else if (localbuf[sizecpy] == '\\')
			break;
	}

	char* filename = &localbuf[sizecpy + 1];

	strcpy(pBuf, filename);
}
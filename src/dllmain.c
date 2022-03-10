#include "helpers.h"
#include <stdio.h>
#include <windows.h>

#define ARRAY_SIZE 255

wchar_t olds[ARRAY_SIZE][ARRAY_SIZE];
wchar_t news[ARRAY_SIZE][ARRAY_SIZE];

HOOK (void, __stdcall, DivaDrawTextW, 0x140198380, void *param, uint32_t flags,
	  const wchar_t **text)
{
	for (int i = 0; i < ARRAY_SIZE; i++)
		{
			if (wcscmp (*text, olds[i]) == 0)
				{
					const wchar_t *ptrs[2];
					ptrs[0] = news[i];
					ptrs[1] = (wchar_t *)((uint64_t)ptrs[0]
										  + wcslen (news[i]) * 2);
					originalDivaDrawTextW (param, flags, &ptrs[0]);
					return;
				}
		}
	originalDivaDrawTextW (param, flags, text);
}

BOOL WINAPI
DllMain (HMODULE mod, DWORD cause, void *ctx)
{
	if (cause != DLL_PROCESS_ATTACH)
		return 1;

	INSTALL_HOOK (DivaDrawTextW);

	WIN32_FIND_DATAA fd;
	HANDLE file = FindFirstFileA (configPath ("translations\\*.toml"), &fd);
	if (file == 0)
		return 1;

	do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			char filepath[MAX_PATH];
			strcpy (filepath, configPath ("translations\\"));
			strcat (filepath, fd.cFileName);
			toml_table_t *translationConfig = openConfig (filepath);

			if (!readConfigBool (translationConfig, "enabled", false))
				{
					toml_free (translationConfig);
					continue;
				}

			toml_array_t *translationArray
				= toml_array_in (translationConfig, "translation");
			for (int i = 0;; i++)
				{
					toml_table_t *translation
						= toml_table_at (translationArray, i);
					if (!translation)
						break;
					toml_datum_t old = toml_string_in (translation, "old");
					toml_datum_t new = toml_string_in (translation, "new");
					if (!old.ok || !new.ok)
						continue;

					MultiByteToWideChar (CP_UTF8, 0, old.u.s, -1, olds[i],
										 ARRAY_SIZE);
					MultiByteToWideChar (CP_UTF8, 0, new.u.s, -1, news[i],
										 ARRAY_SIZE);

					free (old.u.s);
					free (new.u.s);
				}

			toml_free (translationConfig);
		}
	while (FindNextFileA (file, &fd));

	return 1;
}

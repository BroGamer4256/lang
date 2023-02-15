#include "helpers.h"
#include "sigscan.h"
#include <stdio.h>
#include <windows.h>

int num = 0;

wchar_t **olds;
wchar_t **news;

SIG_SCAN (sigDrawText, 0x1402C583D, "\xE8\x00\x00\x00\x00\xE9\x00\x00\x00\x00\x4D\x8B\xC1", "x????x????xxx");

HOOK (void, __stdcall, DivaDrawTextW, 0x1402C7400, void *param, uint32_t flags, const wchar_t **text) {
	for (int i = 0; i < num; i++) {
		if (wcscmp (*text, olds[i]) == 0) {
			const wchar_t *ptrs[2];
			ptrs[0] = news[i];
			ptrs[1] = (wchar_t *)((uint64_t)ptrs[0] + wcslen (news[i]) * 2);
			originalDivaDrawTextW (param, flags, &ptrs[0]);
			return;
		}
	}
	originalDivaDrawTextW (param, flags, text);
}

u32
readUnalignedU32 (u64 memory) {
	u8 *p = (u8 *)memory;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

void
init () {
	freopen ("CONOUT$", "w", stdout);

	u64 drawTextLoc    = (u64)sigDrawText ();
	whereDivaDrawTextW = (void *)(readUnalignedU32 (drawTextLoc + 1) + 5 + drawTextLoc);
	INSTALL_HOOK (DivaDrawTextW);

	WIN32_FIND_DATAW fd;
	HANDLE file = FindFirstFileW (L"translations\\*.toml", &fd);
	if (file == 0) return;

	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		wchar_t filepath[MAX_PATH];
		wcscpy (filepath, L"translations\\");
		wcscat (filepath, fd.cFileName);
		toml_table_t *translationConfig = openConfigW (filepath);
		if (!translationConfig) continue;

		if (!readConfigBool (translationConfig, "enabled", false)) {
			toml_free (translationConfig);
			continue;
		}

		toml_array_t *translationArray = toml_array_in (translationConfig, "translation");
		for (int i = 0;; i++) {
			toml_table_t *translation = toml_table_at (translationArray, i);
			if (!translation) break;
			toml_datum_t old = toml_string_in (translation, "old");
			toml_datum_t new = toml_string_in (translation, "new");
			if (!old.ok || !new.ok) continue;
			num++;

			free (old.u.s);
			free (new.u.s);
		}
		toml_free (translationConfig);
	} while (FindNextFileW (file, &fd));

	olds = calloc (num, sizeof (wchar_t *));
	news = calloc (num, sizeof (wchar_t *));
	num  = 0;

	file = FindFirstFileW (L"translations\\*.toml", &fd);
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

		wchar_t filepath[MAX_PATH];
		wcscpy (filepath, L"translations\\");
		wcscat (filepath, fd.cFileName);
		toml_table_t *translationConfig = openConfigW (filepath);

		if (!translationConfig) continue;

		if (!readConfigBool (translationConfig, "enabled", false)) {
			toml_free (translationConfig);
			continue;
		}

		toml_array_t *translationArray = toml_array_in (translationConfig, "translation");
		for (int i = 0;; i++) {
			toml_table_t *translation = toml_table_at (translationArray, i);
			if (!translation) break;
			toml_datum_t old = toml_string_in (translation, "old");
			toml_datum_t new = toml_string_in (translation, "new");
			if (!old.ok || !new.ok) continue;

			int sizeOld = MultiByteToWideChar (CP_UTF8, 0, old.u.s, -1, 0, 0);
			int sizeNew = MultiByteToWideChar (CP_UTF8, 0, new.u.s, -1, 0, 0);

			olds[num] = calloc (sizeOld, sizeof (wchar_t));
			news[num] = calloc (sizeNew, sizeof (wchar_t));

			MultiByteToWideChar (CP_UTF8, 0, old.u.s, -1, olds[num], sizeOld);
			MultiByteToWideChar (CP_UTF8, 0, new.u.s, -1, news[num], sizeNew);

			num++;

			free (old.u.s);
			free (new.u.s);
		}
		toml_free (translationConfig);
	} while (FindNextFileW (file, &fd));
}

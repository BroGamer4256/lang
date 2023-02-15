#include "helpers.h"
#include "sigscan.h"
#include <processenv.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <windows.h>

std::unordered_map<std::wstring, std::wstring> translations = {};

SIG_SCAN (sigDrawText, 0x1402C583D, "\xE8\x00\x00\x00\x00\xE9\x00\x00\x00\x00\x4D\x8B\xC1", "x????x????xxx");

HOOK (void, __stdcall, DivaDrawTextW, 0x1402C7400, void *param, uint32_t flags, const wchar_t **text) {
	if (translations.contains (std::wstring (*text))) {
		const wchar_t *ptrs[2];
		ptrs[0] = translations[std::wstring (*text)].c_str ();
		ptrs[1] = (wchar_t *)((u64)ptrs[0] + wcslen (ptrs[0]) * 2);
		originalDivaDrawTextW (param, flags, ptrs);
		return;
	}
	originalDivaDrawTextW (param, flags, text);
}

u32
readUnalignedU32 (u64 memory) {
	u8 *p = (u8 *)memory;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

extern "C" void
init () {
	freopen ("CONOUT$", "w", stdout);

	u64 drawTextLoc    = (u64)sigDrawText ();
	whereDivaDrawTextW = (void *)(readUnalignedU32 (drawTextLoc + 1) + 5 + drawTextLoc);
	INSTALL_HOOK (DivaDrawTextW);

	SetCurrentDirectory (exeDirectory ());

	WIN32_FIND_DATAW dirData;
	HANDLE modDir = FindFirstFileW (L"mods\\*", &dirData);
	if (modDir == 0) return;

	do {
		wchar_t dirPath[MAX_PATH];
		wcscpy (dirPath, L"mods\\");
		wcscat (dirPath, dirData.cFileName);
		wcscat (dirPath, L"\\translations\\");
		if (!PathFileExistsW (dirPath)) continue;
		wcscat (dirPath, L"*.toml");

		WIN32_FIND_DATAW fd;
		HANDLE file = FindFirstFileW (dirPath, &fd);
		if (file == 0) continue;

		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

			wchar_t filepath[MAX_PATH];
			wcscpy (filepath, L"mods\\");
			wcscat (filepath, dirData.cFileName);
			wcscat (filepath, L"\\translations\\");
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
				toml_datum_t oldTranslation = toml_string_in (translation, "old");
				toml_datum_t newTranslation = toml_string_in (translation, "new");
				if (!oldTranslation.ok || !newTranslation.ok) continue;

				int sizeOld = MultiByteToWideChar (CP_UTF8, 0, oldTranslation.u.s, -1, 0, 0);
				int sizeNew = MultiByteToWideChar (CP_UTF8, 0, newTranslation.u.s, -1, 0, 0);

				wchar_t *oldTranslationW = (wchar_t *)calloc (sizeOld, sizeof (wchar_t));
				wchar_t *newTranslationW = (wchar_t *)calloc (sizeNew, sizeof (wchar_t));

				MultiByteToWideChar (CP_UTF8, 0, oldTranslation.u.s, -1, oldTranslationW, sizeOld);
				MultiByteToWideChar (CP_UTF8, 0, newTranslation.u.s, -1, newTranslationW, sizeNew);

				translations[std::wstring (oldTranslationW)] = std::wstring (newTranslationW);

				free (oldTranslation.u.s);
				free (newTranslation.u.s);
			}
			toml_free (translationConfig);
		} while (FindNextFileW (file, &fd));
	} while (FindNextFileW (modDir, &dirData));
}

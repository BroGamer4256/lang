#include "helpers.h"
#include "sigscan.h"
#include <processenv.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <windows.h>

enum class State : i32 {
	STARTUP     = 0,
	ADVERTISE   = 1,
	GAME        = 2,
	DATA_TEST   = 3,
	TEST_MODE   = 4,
	APP_ERROR   = 5,
	CS_MENU     = 6,
	CUSTOMIZE   = 7,
	GALLERY     = 8,
	MENU_SWITCH = 9,
	GAME_SWITCH = 10,
	TSHIRT_EDIT = 11,
	MAX         = 12,
};

enum class SubState : i32 {
	DATA_INITIALIZE           = 0,
	SYSTEM_STARTUP            = 1,
	LOGO                      = 2,
	TITLE                     = 3,
	CONCEAL                   = 4,
	PV_SEL                    = 5,
	PLAYLIST_SEL              = 6,
	GAME                      = 7,
	DATA_TEST_MAIN            = 8,
	DATA_TEST_MISC            = 9,
	DATA_TEST_OBJ             = 10,
	DATA_TEST_STG             = 11,
	DATA_TEST_MOT             = 12,
	DATA_TEST_COLLISION       = 13,
	DATA_TEST_SPR             = 14,
	DATA_TEST_AET             = 15,
	DATA_TEST_AUTH3D          = 16,
	DATA_TEST_CHR             = 17,
	DATA_TEST_ITEM            = 18,
	DATA_TEST_PERF            = 19,
	DATA_TEST_PVSCRIPT        = 20,
	DATA_TEST_PRINT           = 21,
	DATA_TEST_CARD            = 22,
	DATA_TEST_OPD             = 23,
	DATA_TEST_SLIDER          = 24,
	DATA_TEST_GLITTER         = 25,
	DATA_TEST_GRAPHICS        = 26,
	DATA_TEST_COLLECTION_CARD = 27,
	DATA_TEST_PAD             = 28,
	TEST_MODE                 = 29,
	APP_ERROR                 = 30,
	UNK_31                    = 31,
	CS_MENU                   = 32,
	CS_COMMERCE               = 33,
	CS_OPTION_MENU            = 34,
	CS_UNK_TUTORIAL_35        = 35,
	CS_CUSTOMIZE_SEL          = 36,
	CS_UNK_TUTORIAL_37        = 37,
	CS_GALLERY                = 38,
	UNK_39                    = 39,
	UNK_40                    = 40,
	UNK_41                    = 41,
	MENU_SWITCH               = 42,
	UNK_43                    = 43,
	OPTION_MENU_SWITCH        = 44,
	UNK_45                    = 45,
	UNK_46                    = 46,
	MAX                       = 47,
};

enum class Menu {
	NONE,
	PV_SEL,
	MODULE_SEL,
	GAME,
};

std::unordered_map<std::wstring, std::pair<std::wstring, Menu>> translations = {};

i32
readUnalignedI32 (u64 memory) {
	u8 *p = (u8 *)memory;
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

void *
readReferenceSig (void *sig) {
	return (void *)(readUnalignedI32 ((u64)sig + 1) + 5 + (u64)sig);
}

SIG_SCAN (sigDrawText, 0x1402C583D, "\xE8\x00\x00\x00\x00\xE9\x00\x00\x00\x00\x4D\x8B\xC1", "x????x????xxx");
SIG_SCAN (sigGetState, 0x1402C4A46, "\xE8\x00\x00\x00\x00\x48\x8B\xD8\x80\x78\x10\x00", "x????xxxxxxx");
FUNCTION_PTR (i32 *, __stdcall, GetState, readReferenceSig (sigGetState ()));

Menu
getCurrentMenu () {
	i32 *state               = GetState ();
	State currentState       = (State)state[0];
	SubState currentSubState = (SubState)state[7];

	if (currentState == State::CUSTOMIZE) return Menu::MODULE_SEL;
	if (currentSubState == SubState::PV_SEL) return Menu::PV_SEL;
	if (currentSubState == SubState::GAME) return Menu::GAME;
	return Menu::NONE;
}

Menu
strToMenu (char *str) {
	if (strcmp (str, "PV_SEL") == 0) return Menu::PV_SEL;
	if (strcmp (str, "MODULE_SEL") == 0) return Menu::MODULE_SEL;
	if (strcmp (str, "GAME") == 0) return Menu::GAME;
	return Menu::NONE;
}

HOOK (void, __stdcall, DivaDrawTextW, readReferenceSig (sigDrawText ()), void *param, uint32_t flags, const wchar_t **text) {
	auto it = translations.find (std::wstring (*text));
	if (it != translations.end () && (it->second.second == Menu::NONE || it->second.second == getCurrentMenu ())) {
		const wchar_t *ptrs[2];
		ptrs[0] = it->second.first.c_str ();
		ptrs[1] = (wchar_t *)((u64)ptrs[0] + wcslen (ptrs[0]) * 2);
		originalDivaDrawTextW (param, flags, ptrs);
		return;
	}
	originalDivaDrawTextW (param, flags, text);
}

extern "C" void
init () {
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
			if (!translationArray) continue;
			for (int i = 0;; i++) {
				toml_table_t *translation = toml_table_at (translationArray, i);
				if (!translation) break;
				toml_datum_t oldTranslation = toml_string_in (translation, "old");
				toml_datum_t newTranslation = toml_string_in (translation, "new");
				if (!oldTranslation.ok || !newTranslation.ok) continue;

				Menu menuNeeded   = Menu::NONE;
				toml_datum_t menu = toml_string_in (translation, "menu");
				if (menu.ok) {
					menuNeeded = strToMenu (menu.u.s);
					free (menu.u.s);
				}

				int sizeOld = MultiByteToWideChar (CP_UTF8, 0, oldTranslation.u.s, -1, 0, 0);
				int sizeNew = MultiByteToWideChar (CP_UTF8, 0, newTranslation.u.s, -1, 0, 0);

				wchar_t *oldTranslationW = (wchar_t *)calloc (sizeOld, sizeof (wchar_t));
				wchar_t *newTranslationW = (wchar_t *)calloc (sizeNew, sizeof (wchar_t));

				MultiByteToWideChar (CP_UTF8, 0, oldTranslation.u.s, -1, oldTranslationW, sizeOld);
				MultiByteToWideChar (CP_UTF8, 0, newTranslation.u.s, -1, newTranslationW, sizeNew);

				translations[std::wstring (oldTranslationW)] = std::pair (std::wstring (newTranslationW), menuNeeded);

				free (oldTranslationW);
				free (newTranslationW);
				free (oldTranslation.u.s);
				free (newTranslation.u.s);
			}
			toml_free (translationConfig);
		} while (FindNextFileW (file, &fd));
	} while (FindNextFileW (modDir, &dirData));
}

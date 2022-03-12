#include "helpers.h"
#include <stdio.h>
#include <windows.h>

#define CURRENT_GAME_SUB_STATE_ADDRESS 0x140EDA82C

enum SubGameState : uint32_t
{
	SUB_DATA_INITIALIZE,
	SUB_SYSTEM_STARTUP,
	SUB_SYSTEM_STARTUP_ERROR,
	SUB_WARNING,
	SUB_LOGO,
	SUB_RATING,
	SUB_DEMO,
	SUB_TITLE,
	SUB_RANKING,
	SUB_SCORE_RANKING,
	SUB_CM,
	SUB_PHOTO_MODE_DEMO,
	SUB_SELECTOR,
	SUB_GAME_MAIN,
	SUB_GAME_SEL,
	SUB_STAGE_RESULT,
	SUB_SCREEN_SHOT_SEL,
	SUB_SCREEN_SHOT_RESULT,
	SUB_GAME_OVER,
	SUB_DATA_TEST_MAIN,
	SUB_DATA_TEST_MISC,
	SUB_DATA_TEST_OBJ,
	SUB_DATA_TEST_STG,
	SUB_DATA_TEST_MOT,
	SUB_DATA_TEST_COLLISION,
	SUB_DATA_TEST_SPR,
	SUB_DATA_TEST_AET,
	SUB_DATA_TEST_AUTH_3D,
	SUB_DATA_TEST_CHR,
	SUB_DATA_TEST_ITEM,
	SUB_DATA_TEST_PERF,
	SUB_DATA_TEST_PVSCRIPT,
	SUB_DATA_TEST_PRINT,
	SUB_DATA_TEST_CARD,
	SUB_DATA_TEST_OPD,
	SUB_DATA_TEST_SLIDER,
	SUB_DATA_TEST_GLITTER,
	SUB_DATA_TEST_GRAPHICS,
	SUB_DATA_TEST_COLLECTION_CARD,
	SUB_TEST_MODE_MAIN,
	SUB_APP_ERROR,
	SUB_MAX,
};

struct
{
	const char *string;
	enum SubGameState state;
} ConfigSubGameStates[] = {
	{ "SUB_DATA_INITIALIZE", SUB_DATA_INITIALIZE },
	{ "SUB_SYSTEM_STARTUP", SUB_SYSTEM_STARTUP },
	{ "SUB_SYSTEM_STARTUP_ERROR", SUB_SYSTEM_STARTUP_ERROR },
	{ "SUB_WARNING", SUB_WARNING },
	{ "SUB_LOGO", SUB_LOGO },
	{ "SUB_RATING", SUB_RATING },
	{ "SUB_DEMO", SUB_DEMO },
	{ "SUB_TITLE", SUB_TITLE },
	{ "SUB_RANKING", SUB_RANKING },
	{ "SUB_SCORE_RANKING", SUB_SCORE_RANKING },
	{ "SUB_CM", SUB_CM },
	{ "SUB_PHOTO_MODE_DEMO", SUB_PHOTO_MODE_DEMO },
	{ "SUB_SELECTOR", SUB_SELECTOR },
	{ "SUB_GAME_MAIN", SUB_GAME_MAIN },
	{ "SUB_GAME_SEL", SUB_GAME_SEL },
	{ "SUB_STAGE_RESULT", SUB_STAGE_RESULT },
	{ "SUB_SCREEN_SHOT_SEL", SUB_SCREEN_SHOT_SEL },
	{ "SUB_SCREEN_SHOT_RESULT", SUB_SCREEN_SHOT_RESULT },
	{ "SUB_GAME_OVER", SUB_GAME_OVER },
	{ "SUB_DATA_TEST_MAIN", SUB_DATA_TEST_MAIN },
	{ "SUB_DATA_TEST_MISC", SUB_DATA_TEST_MISC },
	{ "SUB_DATA_TEST_OBJ", SUB_DATA_TEST_OBJ },
	{ "SUB_DATA_TEST_STG", SUB_DATA_TEST_STG },
	{ "SUB_DATA_TEST_MOT", SUB_DATA_TEST_MOT },
	{ "SUB_DATA_TEST_COLLISION", SUB_DATA_TEST_COLLISION },
	{ "SUB_DATA_TEST_SPR", SUB_DATA_TEST_SPR },
	{ "SUB_DATA_TEST_AET", SUB_DATA_TEST_AET },
	{ "SUB_DATA_TEST_AUTH_3D", SUB_DATA_TEST_AUTH_3D },
	{ "SUB_DATA_TEST_CHR", SUB_DATA_TEST_CHR },
	{ "SUB_DATA_TEST_ITEM", SUB_DATA_TEST_ITEM },
	{ "SUB_DATA_TEST_PERF", SUB_DATA_TEST_PERF },
	{ "SUB_DATA_TEST_PVSCRIPT", SUB_DATA_TEST_PVSCRIPT },
	{ "SUB_DATA_TEST_PRINT", SUB_DATA_TEST_PRINT },
	{ "SUB_DATA_TEST_CARD", SUB_DATA_TEST_CARD },
	{ "SUB_DATA_TEST_OPD", SUB_DATA_TEST_OPD },
	{ "SUB_DATA_TEST_SLIDER", SUB_DATA_TEST_SLIDER },
	{ "SUB_DATA_TEST_GLITTER", SUB_DATA_TEST_GLITTER },
	{ "SUB_DATA_TEST_GRAPHICS", SUB_DATA_TEST_GRAPHICS },
	{ "SUB_DATA_TEST_COLLECTION_CARD", SUB_DATA_TEST_COLLECTION_CARD },
	{ "SUB_TEST_MODE_MAIN", SUB_TEST_MODE_MAIN },
	{ "SUB_APP_ERROR", SUB_APP_ERROR },
};

enum SubGameState
strToSubGameState (const char *str)
{
	for (int i = 0; i < COUNTOFARR (ConfigSubGameStates); i++)
		if (strcmp (str, ConfigSubGameStates[i].string) == 0)
			return ConfigSubGameStates[i].state;
	return SUB_MAX;
}

int num = 0;

wchar_t **olds;
wchar_t **news;
enum SubGameState *states;

HOOK (void, __stdcall, DivaDrawTextW, 0x140198380, void *param, uint32_t flags,
	  const wchar_t **text)
{
	for (int i = 0; i < num; i++)
		{
			if (states[i] != SUB_MAX)
				{
					if (states[i] == SUB_GAME_SEL || states[i] == SUB_SELECTOR)
						{
							if (*(enum SubGameState *)
										CURRENT_GAME_SUB_STATE_ADDRESS
									!= SUB_GAME_SEL
								&& *(enum SubGameState *)
										   CURRENT_GAME_SUB_STATE_ADDRESS
									   != SUB_SELECTOR)
								continue;
						}
					else if (*(enum SubGameState *)
								 CURRENT_GAME_SUB_STATE_ADDRESS
							 != states[i])
						{
							continue;
						}
				}

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
	if (cause == DLL_PROCESS_DETACH)
		{
			for (int i = 0; i < num; i++)
				{
					free (olds[i]);
					free (news[i]);
				}
			free (olds);
			free (news);
		}
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
					num++;

					free (old.u.s);
					free (new.u.s);
				}
			toml_free (translationConfig);
		}
	while (FindNextFileA (file, &fd));

	olds = calloc (num, sizeof (wchar_t *));
	news = calloc (num, sizeof (wchar_t *));
	states = calloc (num, sizeof (enum SubGameState));
	num = 0;

	file = FindFirstFileA (configPath ("translations\\*.toml"), &fd);
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
					toml_datum_t state = toml_string_in (translation, "state");
					if (!old.ok || !new.ok)
						continue;

					int sizeOld
						= MultiByteToWideChar (CP_UTF8, 0, old.u.s, -1, 0, 0);
					int sizeNew
						= MultiByteToWideChar (CP_UTF8, 0, new.u.s, -1, 0, 0);

					olds[num] = calloc (sizeOld, sizeof (wchar_t));
					news[num] = calloc (sizeNew, sizeof (wchar_t));

					MultiByteToWideChar (CP_UTF8, 0, old.u.s, -1, olds[num],
										 sizeOld);
					MultiByteToWideChar (CP_UTF8, 0, new.u.s, -1, news[num],
										 sizeNew);

					if (state.ok)
						states[num] = strToSubGameState (state.u.s);
					else
						states[num] = SUB_MAX;

					num++;

					free (old.u.s);
					free (new.u.s);
					if (state.ok)
						free (state.u.s);
				}
			toml_free (translationConfig);
		}
	while (FindNextFileA (file, &fd));

	return 1;
}

#include "helpers.h"
#include <stdio.h>
#include <time.h>
#include <windows.h>

void owoDrawTextW (void *param, uint32_t flags, const wchar_t **text);

struct linked_list {
	const wchar_t *original;
	wchar_t *owoified;
	struct linked_list *next;
};

int num = 0;

wchar_t **olds;
wchar_t **news;
struct linked_list *translated;

HOOK (void, __stdcall, DivaDrawTextW, 0x140198380, void *param, uint32_t flags,
	  const wchar_t **text) {
	for (int i = 0; i < num; i++) {
		if (wcscmp (*text, olds[i]) == 0) {
			const wchar_t *ptrs[2];
			ptrs[0] = news[i];
			ptrs[1] = (wchar_t *)((uint64_t)ptrs[0] + wcslen (news[i]) * 2);
			owoDrawTextW (param, flags, &ptrs[0]);
			return;
		}
	}
	originalDivaDrawTextW (param, flags, text);

	/*
	wchar_t buf[256] = { 0 };
	swprintf (buf, 256, L"[lang] %ls\n\0", *text);
	HANDLE consoleHandle = GetStdHandle (STD_OUTPUT_HANDLE);
	WriteConsoleW (consoleHandle, buf, 256, NULL, NULL);
	*/
}
wchar_t *faces[] = { L"owo", L"UwU", L">w<", L"^w^" };

wchar_t *
owoTranslate (const wchar_t *text) {
	int len = 0;
	for (int i = 0; i < wcslen (text); i++) {
		len++;
		switch (text[i]) {
		case L'!':
			len += 4;
			break;
		case L'n':
		case L'N':
			if (text[i + 1] == L'a' || text[i + 1] == L'e'
				|| text[i + 1] == L'i' || text[i + 1] == L'o'
				|| text[i + 1] == L'u' || text[i + 1] == L'A'
				|| text[i + 1] == L'E' || text[i + 1] == L'I'
				|| text[i + 1] == L'O' || text[i + 1] == L'U')
				len++;
			break;
		default:
			break;
		}
	}
	srand (time (0));
	wchar_t *owoified = calloc (len + 1, sizeof (wchar_t));
	int owoI = 0;
	for (int i = 0; i < wcslen (text); i++) {
		switch (text[i]) {
		case L'!':;
			wchar_t *face = faces[rand () % COUNTOFARR (faces)];
			owoified[owoI] = L' ';
			owoified[owoI + 1] = face[0];
			owoified[owoI + 2] = face[1];
			owoified[owoI + 3] = face[2];
			owoified[owoI + 4] = L' ';
			owoI += 4;
			break;
		case L'n':
		case L'N':
			if (text[i + 1] == L'a' || text[i + 1] == L'e'
				|| text[i + 1] == L'i' || text[i + 1] == L'o'
				|| text[i + 1] == L'u' || text[i + 1] == L'A'
				|| text[i + 1] == L'E' || text[i + 1] == L'I'
				|| text[i + 1] == L'O' || text[i + 1] == L'U') {
				owoified[owoI] = text[i];
				owoified[owoI + 1] = L'y';
				owoI++;
			} else {
				owoified[owoI] = text[i];
			}
			break;
		default:
			owoified[owoI] = text[i];
			break;
		}
		owoI++;
	}
	owoified[owoI] = 0;

	return owoified;
}

struct linked_list *
lookup (struct linked_list *list, const wchar_t *text) {
	if (list == 0 || list->next == 0) {
		return 0;
	}
	if (list->original == 0 || wcscmp (list->original, text) != 0) {
		return lookup (list->next, text);
	}
	return list;
}

struct linked_list *
last (struct linked_list *list) {
	if (list == 0) {
		return 0;
	}
	struct linked_list *last = list;
	while (last->next != 0) {
		last = last->next;
	}
	return last;
}

void
add (struct linked_list *list, struct linked_list *new) {
	struct linked_list *last_list = last (list);
	if (last_list == 0) {
		list = new;
		return;
	}
	last_list->next = new;
}

void
owoDrawTextW (void *param, uint32_t flags, const wchar_t **text) {
	struct linked_list *owoified = lookup (translated, *text);
	if (owoified == 0) {
		wchar_t *newText = owoTranslate (*text);
		owoified = malloc (sizeof (struct linked_list));
		owoified->original = *text;
		owoified->owoified = newText;
		add (translated, owoified);
	}
	const wchar_t *ptrs[2];
	ptrs[0] = owoified->owoified;
	ptrs[1] = (wchar_t *)((uint64_t)ptrs[0] + wcslen (owoified->owoified) * 2);

	originalDivaDrawTextW (param, flags, &ptrs[0]);
}

BOOL WINAPI
DllMain (HMODULE mod, DWORD cause, void *ctx) {
	if (cause == DLL_PROCESS_DETACH) {
		for (int i = 0; i < num; i++) {
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

	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		char filepath[MAX_PATH];
		strcpy (filepath, configPath ("translations\\"));
		strcat (filepath, fd.cFileName);
		toml_table_t *translationConfig = openConfig (filepath);

		if (!readConfigBool (translationConfig, "enabled", false)) {
			toml_free (translationConfig);
			continue;
		}

		toml_array_t *translationArray
			= toml_array_in (translationConfig, "translation");
		for (int i = 0;; i++) {
			toml_table_t *translation = toml_table_at (translationArray, i);
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
	} while (FindNextFileA (file, &fd));

	olds = calloc (num, sizeof (wchar_t *));
	news = calloc (num, sizeof (wchar_t *));
	num = 0;

	file = FindFirstFileA (configPath ("translations\\*.toml"), &fd);
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		char filepath[MAX_PATH];
		strcpy (filepath, configPath ("translations\\"));
		strcat (filepath, fd.cFileName);
		toml_table_t *translationConfig = openConfig (filepath);

		if (!translationConfig)
			continue;

		if (!readConfigBool (translationConfig, "enabled", false)) {
			toml_free (translationConfig);
			continue;
		}

		toml_array_t *translationArray
			= toml_array_in (translationConfig, "translation");
		for (int i = 0;; i++) {
			toml_table_t *translation = toml_table_at (translationArray, i);
			if (!translation)
				break;
			toml_datum_t old = toml_string_in (translation, "old");
			toml_datum_t new = toml_string_in (translation, "new");
			if (!old.ok || !new.ok)
				continue;

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
	} while (FindNextFileA (file, &fd));

	return 1;
}

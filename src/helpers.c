#include "helpers.h"
#include <windows.h>

char *
exeDirectory () {
	static char buffer[MAX_PATH];
	GetModuleFileNameA (NULL, buffer, MAX_PATH);
	*strrchr (buffer, '\\') = 0;
	return buffer;
}

toml_table_t *
openConfig (const char *configFilePath) {
	FILE *file = fopen (configFilePath, "r");
	if (!file) {
		printf ("Error at openConfig (%s): cannot open file\n", configFilePath);
		return 0;
	}
	char errorbuf[200];
	toml_table_t *config = toml_parse_file (file, errorbuf, 200);
	fclose (file);

	if (!config) {
		printf ("Error at openConfig (%s): %s\n", configFilePath, errorbuf);
		return 0;
	}

	return config;
}

toml_table_t *
openConfigW (const wchar_t *configFilePath) {
	FILE *file = _wfopen (configFilePath, L"r");
	if (!file) {
		wprintf (L"Error at openConfig (%ls): cannot open file\n", configFilePath);
		return 0;
	}
	char errorbuf[200];
	toml_table_t *config = toml_parse_file (file, errorbuf, 200);
	fclose (file);

	if (!config) {
		wprintf (L"Error at openConfig (%ls): %s\n", configFilePath, errorbuf);
		return 0;
	}

	return config;
}

toml_table_t *
openConfigSection (toml_table_t *config, const char *sectionName) {
	toml_table_t *section = toml_table_in (config, sectionName);
	if (!section) {
		printf ("Error at openConfigSection (%s): cannot find section\n", sectionName);
		return 0;
	}

	return section;
}

bool
readConfigBool (toml_table_t *table, const char *key, bool notFoundValue) {
	toml_datum_t data = toml_bool_in (table, key);
	if (!data.ok) return notFoundValue;

	return (bool)data.u.b;
}

int64_t
readConfigInt (toml_table_t *table, const char *key, int64_t notFoundValue) {
	toml_datum_t data = toml_int_in (table, key);
	if (!data.ok) return notFoundValue;

	return data.u.i;
}

char *
readConfigString (toml_table_t *table, const char *key, char *notFoundValue) {
	toml_datum_t data = toml_string_in (table, key);
	if (!data.ok) return notFoundValue;

	return data.u.s;
}

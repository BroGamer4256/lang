#pragma once
#include <MinHook.h>
#include <stdbool.h>
#include <stdint.h>
#include <toml.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define FUNCTION_PTR(returnType, callingConvention, function, location, ...) returnType (callingConvention *function) (__VA_ARGS__) = (returnType (callingConvention *) (__VA_ARGS__)) (location)

#define PROC_ADDRESS(libraryName, procName) GetProcAddress (LoadLibrary (TEXT (libraryName)), procName)

#define HOOK(returnType, callingConvention, functionName, location, ...)                                                                                                                               \
	typedef returnType callingConvention (*functionName) (__VA_ARGS__);                                                                                                                                \
	functionName original##functionName = NULL;                                                                                                                                                        \
	void *where##functionName           = (void *)location;                                                                                                                                            \
	returnType callingConvention implOf##functionName (__VA_ARGS__)

#define INSTALL_HOOK(functionName)                                                                                                                                                                     \
	{                                                                                                                                                                                                  \
		MH_Initialize ();                                                                                                                                                                              \
		MH_CreateHook ((void *)where##functionName, (void *)implOf##functionName, (void **)(&original##functionName));                                                                                 \
		MH_EnableHook ((void *)where##functionName);                                                                                                                                                   \
	}

#define READ_MEMORY(location, type) *(type *)location

#define WRITE_MEMORY(location, type, ...)                                                                                                                                                              \
	{                                                                                                                                                                                                  \
		const type data[] = {__VA_ARGS__};                                                                                                                                                             \
		DWORD oldProtect;                                                                                                                                                                              \
		VirtualProtect ((void *)(location), sizeof (data), PAGE_EXECUTE_READWRITE, &oldProtect);                                                                                                       \
		memcpy ((void *)(location), data, sizeof (data));                                                                                                                                              \
		VirtualProtect ((void *)(location), sizeof (data), oldProtect, &oldProtect);                                                                                                                   \
	}

#define WRITE_MEMORY_STRING(location, data, length)                                                                                                                                                    \
	{                                                                                                                                                                                                  \
		DWORD oldProtect;                                                                                                                                                                              \
		VirtualProtect ((void *)(location), length, PAGE_EXECUTE_READWRITE, &oldProtect);                                                                                                              \
		memcpy ((void *)(location), data, length);                                                                                                                                                     \
		VirtualProtect ((void *)(location), length, oldProtect, &oldProtect);                                                                                                                          \
	}

#define WRITE_NOP(location, count)                                                                                                                                                                     \
	{                                                                                                                                                                                                  \
		DWORD oldProtect;                                                                                                                                                                              \
		VirtualProtect ((void *)(location), (size_t)(count), PAGE_EXECUTE_READWRITE, &oldProtect);                                                                                                     \
		for (size_t i = 0; i < (size_t)(count); i++)                                                                                                                                                   \
			*((uint8_t *)(location) + i) = 0x90;                                                                                                                                                       \
		VirtualProtect ((void *)(location), (size_t)(count), oldProtect, &oldProtect);                                                                                                                 \
	}

#define WRITE_NULL(location, count)                                                                                                                                                                    \
	{                                                                                                                                                                                                  \
		DWORD oldProtect;                                                                                                                                                                              \
		VirtualProtect ((void *)(location), (size_t)(count), PAGE_EXECUTE_READWRITE, &oldProtect);                                                                                                     \
		for (size_t i = 0; i < (size_t)(count); i++)                                                                                                                                                   \
			*((uint8_t *)(location) + i) = 0x00;                                                                                                                                                       \
		VirtualProtect ((void *)(location), (size_t)(count), oldProtect, &oldProtect);                                                                                                                 \
	}

#define COUNTOFARR(arr) sizeof (arr) / sizeof (arr[0])

#ifdef __cplusplus
extern "C" {
#endif
char *exeDirectory ();
toml_table_t *openConfig (const char *configFilePath);
toml_table_t *openConfigW (const wchar_t *configFilePath);
toml_table_t *openConfigSection (toml_table_t *config, const char *sectionName);
bool readConfigBool (toml_table_t *table, const char *key, bool notFoundValue);
int64_t readConfigInt (toml_table_t *table, const char *key, int64_t notFoundValue);
char *readConfigString (toml_table_t *table, const char *key, char *notFoundValue);
#ifdef __cplusplus
}
#endif

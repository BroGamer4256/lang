#include "sigscan.h"
#include "helpers.h"
#include <psapi.h>

MODULEINFO moduleInfo;

const MODULEINFO
getModuleInfo () {
	if (moduleInfo.SizeOfImage) return moduleInfo;

	ZeroMemory (&moduleInfo, sizeof (MODULEINFO));
	GetModuleInformation (GetCurrentProcess (), GetModuleHandle (0), &moduleInfo, sizeof (MODULEINFO));

	return moduleInfo;
}

void *
sigScanMemory (const char *signature, const char *mask, void *memory, size_t memorySize) {
	const size_t sigSize = strlen (signature);

	for (size_t i = 0; i < memorySize; i++) {
		char *currMemory = (char *)memory + i;

		size_t j;
		for (j = 0; j < sigSize; j++)
			if (mask[j] != '?' && signature[j] != currMemory[j]) break;

		if (j == sigSize) return currMemory;
	}

	return 0;
}

void *
sigScan (const char *signature, const char *mask, void *hint) {
	const size_t sigSize  = strlen (mask);
	const MODULEINFO info = getModuleInfo ();
	if ((hint >= info.lpBaseOfDll) && ((char *)hint + sigSize <= (char *)info.lpBaseOfDll + info.SizeOfImage)) {
		void *result = sigScanMemory (signature, mask, hint, sigSize);

		if (result) return result;
	}
	return sigScanMemory (signature, mask, info.lpBaseOfDll, info.SizeOfImage);
}

bool sigValid = true;

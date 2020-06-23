#pragma once
#include "windows.h"
#include <vector>

class cMemUtil
{
public:
	bool PatchAdr(LPVOID dst, LPVOID src, size_t len);
	bool PlaceHook(void * hookSpot, void * ourFunct, int len);
	PBYTE TrampHook(PBYTE src, PBYTE dst, unsigned int len);
	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<unsigned int> offsets);
	uint8_t* FindPattern(uint32_t dwAddress, size_t dwLen, uint8_t* bMask, char* szMask);
	uintptr_t ReadPtr(uintptr_t adr);
	DWORD FindPattern(DWORD address, DWORD size, PBYTE pattern, PCHAR mask);
};

extern cMemUtil MemUtil;
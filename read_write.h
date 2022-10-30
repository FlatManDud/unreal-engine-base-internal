#pragma once
#include <Windows.h>


bool IsValidPointer(uintptr_t address) {
	if (!IsBadWritePtr((LPVOID)address, (UINT_PTR)8)) return TRUE;
	else return false;
}

template<typename ReadT>
ReadT read(uintptr_t address, const ReadT& def = ReadT()) {
	if (IsValidPointer(address)) {
		return *(ReadT*)address;
	}
}

template<typename WriteT>
WriteT write(uintptr_t address, WriteT value, const WriteT& def = WriteT()) {
	if (IsValidPointer(address)) {
		*(WriteT*)((PBYTE)address) = value;
	}
	return 1;
}

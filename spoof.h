#pragma once
#include <iostream>
#include <Windows.h>
#include <winternl.h>
#include <ctime>

using _RtlCreateUserThread = NTSTATUS(NTAPI*)(HANDLE ProcessHandel,
	PSECURITY_DESCRIPTOR SecruityDescriptor,
	BOOLEAN CreateSuspended,
	ULONG StackZeroBits,
	PULONG StackReserved,
	PULONG StackCommit,
	void* StartAdress,
	void* StartParameter,
	PHANDLE ThreadHandle,
	void* ClientID
	);







void SpoofThreadLocation(void* thread, HMODULE& hModule)
{
	HMODULE hNtdll = LoadLibrary("ntdll");

	uintptr_t spoofed_add = NULL;
	int DefaultThreadSize = 1000;

	srand(time(NULL));

	for (int i = 1; i < 4; ++i)
	{
		spoofed_add |= (rand() & 0xFF) << i * 8;
		spoofed_add |= (rand() & 0xFF) << i * 8;
		spoofed_add |= (rand() & 0xFF) << i * 8;
	}
	// if crashes change to  0x7FFFFFFF
	while (spoofed_add > 0x7FFFFFFFFFFFFFFF) // highest 64bit address biggest number in x64 is 9,223,372,036,854,775,807 what is 7FFFFFFFFFFFFFFF
		spoofed_add -= 0x1000; 

	VirtualProtect((void*)spoofed_add, DefaultThreadSize, PAGE_EXECUTE_READWRITE, NULL);

	CONTEXT tContext;

	HANDLE ThreadHandle = nullptr;

	_RtlCreateUserThread NtThread = (_RtlCreateUserThread)(GetProcAddress(GetModuleHandle("ntdll"), "RtlCreateUserThread"));
	NtThread(GetCurrentProcess(), nullptr, TRUE, NULL, NULL, NULL, (PTHREAD_START_ROUTINE)spoofed_add, hModule, &ThreadHandle, NULL);

	tContext.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL; // maybe broken

	GetThreadContext(ThreadHandle, &tContext);

	tContext.Rcx = (ULONG64)thread; // only works for x64
	//32 bit
	// tContext.Eax = (ULONG32)thread;

	tContext.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL; // maybe broken

	SetThreadContext(ThreadHandle, &tContext);

	ResumeThread(ThreadHandle);

}




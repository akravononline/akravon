#include "StdAfx.h"
#include "Mutex.h"

Mutex::Mutex()
{
	InitializeCriticalSection(&lock);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(&lock);
}

void Mutex::Lock()
{
	EnterCriticalSection(&lock);
}

void Mutex::Unlock()
{
	LeaveCriticalSection(&lock);
}
//martysama0134's d3fef13446d3498b8b5c1aceb4c44240

// Cynex iOS: <malloc.h> yerine (Linux/Android basligi) - Apple'da malloc/malloc.h
#pragma once
#include <stdlib.h>
#include <malloc/malloc.h>
#ifndef malloc_usable_size
#define malloc_usable_size(p) malloc_size(p)
#endif

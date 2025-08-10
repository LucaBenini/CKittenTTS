#pragma once
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

wchar_t *os_char_to_wchar(const char *input);
char *os_wchar_to_char(const wchar_t *input);
void* os_load_library(const char* dllname);
void* os_get_function(void* dll, const char* funcname);
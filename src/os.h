#pragma once
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define ESPEAK_DLL "libespeak-ng.dll"
        #define ONNX_DLL "onnxruntime.dll"
#include <windows.h>
#include <io.h>
#define _CRT_SECURE_NO_WARNINGS
#ifdef UNICODE
	#undef UNICODE
#endif

#else
#define ESPEAK_DLL "libespeak-ng"
        #define ONNX_DLL "libonnx.so"
#endif
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

wchar_t *os_char_to_wchar(const char *input);
char *os_wchar_to_char(const wchar_t *input);
void* os_load_library(const char* dllname);
void* os_get_function(void* dll, const char* funcname);
void os_print_last_error(const char* msg);

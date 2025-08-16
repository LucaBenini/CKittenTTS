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
	#include <stdint.h>
#else
	#include <unistd.h>
	#include <dlfcn.h>
	#include <stdio.h>
	#include <stdint.h>
	#define ESPEAK_DLL "libespeak-ng.so"
	#define ONNX_DLL "libonnxruntime.so"
#endif
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

wchar_t *os_char_to_wchar(const char *input);
char *os_wchar_to_char(const wchar_t *input);
void* os_load_library(const char* dllname);
void* os_get_function(void* dll, const char* funcname);
void os_print_last_error(const char* msg);
int os_file_exists(const char* file_path);
int os_write_wav_float32_mono(const char* path, const float* samples,uint32_t nframes, uint32_t sample_rate /* e.g., 24000 */);
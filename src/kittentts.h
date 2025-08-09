#pragma once
#define _CRT_SECURE_NO_WARNINGS
#ifndef UNICODE
	#define UNICODE
#endif
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdio.h>

typedef struct phonemes_manager phonemes_manager;


int pm_create(phonemes_manager** pm);
int pm_init(phonemes_manager* pm);
int pm_encode(phonemes_manager* pm, const wchar_t* text, int** destination, size_t* destination_len);
int pm_destroy(phonemes_manager* pm);


typedef struct onnx_manager onnx_manager;


int onnx_create(onnx_manager** om);
int onnx_init(onnx_manager* om, const wchar_t* model_path, const wchar_t* voice_path);
int onnx_run(onnx_manager* om, int* input_ids, size_t input_ids_len,const char* output_wav);
int onnx_destroy(onnx_manager* om);

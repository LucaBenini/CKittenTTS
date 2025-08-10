#pragma once

#include "os.h"
#include <fcntl.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdio.h>

typedef struct kt_params_init
{
	const char* model_path;
	const char* voice_path;
}kt_params_init;

typedef struct kt_params_run
{
	const char* output;
	const char* message;
	float speed;
	int* input_ids;
	size_t input_ids_len;
}kt_params_run;
typedef struct kt_params
{
	kt_params_init init;
	kt_params_run run;
}kt_params;

int kt_create(kt_params** kp);
int kt_load(kt_params* kp, int argc, char* argv[]);

int kt_validate(kt_params* kp);
int kt_destroy(kt_params* kp);

typedef struct phonemes_manager phonemes_manager;

int pm_create(phonemes_manager** pm);
int pm_init(phonemes_manager* pm);
int pm_encode(phonemes_manager* pm, kt_params* kp);
int pm_destroy(phonemes_manager* pm);


typedef struct onnx_manager onnx_manager;

int onnx_create(onnx_manager** om);
int onnx_init(onnx_manager* om, kt_params* kp);
int onnx_run(onnx_manager* om, kt_params* kp);
int onnx_destroy(onnx_manager* om);

#include "kittentts.h"
#include <windows.h>
#include <stdio.h>


int main(int argc, char* argv[])
{
    int rc = 0;

	/* high-resolution timer frequency */
	LARGE_INTEGER freq;
    LARGE_INTEGER start, end;
    QueryPerformanceFrequency(&freq);

    kt_params* kp = NULL;
    phonemes_manager* pm = NULL;
    onnx_manager* om = NULL;

    // --- Create & load/validate params ---
    if ((rc = kt_create(&kp)) != 0) goto cleanup;
    if ((rc = kt_load(kp, argc, argv)) != 0) goto cleanup;      // prints usage on its own
    if ((rc = kt_validate(kp)) != 0) goto cleanup;

    // --- Create managers ---
    if ((rc = pm_create(&pm)) != 0) goto cleanup;
    if ((rc = onnx_create(&om)) != 0) goto cleanup;

    // --- Initialize managers ---
    if ((rc = pm_init(pm)) != 0) goto cleanup;
    if ((rc = onnx_init(om, kp)) != 0) goto cleanup;

	QueryPerformanceCounter(&start);
    // --- Run pipeline ---
    if ((rc = pm_encode(pm, kp)) != 0) goto cleanup;
    if ((rc = onnx_run(om, kp)) != 0) goto cleanup;
	QueryPerformanceCounter(&end);
    double elapsed_seconds = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
    printf("Pipeline time: %.6f seconds\n", elapsed_seconds);
    os_write_wav_float32_mono(kp->run.output_file, (void*)kp->run.output, kp->run.output_len, 24000);
    free(kp->run.output);
    kp->run.output = 0;

cleanup:
    // Destroy in reverse order; guard NULLs in case creation failed.
    if (om) onnx_destroy(om);
    if (pm) pm_destroy(pm);
    if (kp) kt_destroy(kp);

    return rc;
}

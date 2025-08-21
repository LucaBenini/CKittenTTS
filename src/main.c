#include "kittentts.h"
#include <stdio.h>
#ifdef _WIN32
  #include <windows.h>
#else
#include <time.h>
#endif

int main(int argc, char* argv[])
{
    int rc = 0;

	/* high-resolution timer frequency */
#ifdef _WIN32
    LARGE_INTEGER freq;
    LARGE_INTEGER start, end;
    QueryPerformanceFrequency(&freq);
#else
    struct timespec start, end;
#endif
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

    for(int i =0; i < 3; i++)
      {
#ifdef _WIN32
    QueryPerformanceCounter(&start);
#else
    clock_gettime(CLOCK_MONOTONIC, &start);
#endif

    // --- Run pipeline ---
    if ((rc = pm_encode(pm, kp)) != 0) goto cleanup;
    if ((rc = onnx_run(om, kp)) != 0) goto cleanup;

#ifdef _WIN32
    QueryPerformanceCounter(&end);
    double elapsed_seconds = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;
#else
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_seconds = (end.tv_sec - start.tv_sec)
      + (end.tv_nsec - start.tv_nsec) / 1e9;
#endif
    if(i)
      printf("[%04d] Pipeline time: %.6f seconds\n", i,elapsed_seconds);
    os_write_wav_float32_mono(kp->run.output_file, (void*)kp->run.output, kp->run.output_len, 24000);
    free(kp->run.output);
    kp->run.output = 0;
      }
cleanup:
    // Destroy in reverse order; guard NULLs in case creation failed.
    if (om) onnx_destroy(om);
    if (pm) pm_destroy(pm);
    if (kp) kt_destroy(kp);

    return rc;
}

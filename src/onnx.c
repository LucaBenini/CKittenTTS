#include "kittentts.h"
#include "onnxruntime_c_api.h"
typedef OrtApiBase* (ORT_API_CALL *OrtGetApiBase_t)(void);
typedef struct onnx_manager
{
		const OrtApi* api;
        OrtEnv* env;
        OrtSessionOptions* session_options;
        OrtSession* session;
        OrtMemoryInfo* memory_info;
        
        float* voice;
        size_t voice_size;
} onnx_manager;
static int load_float32_array(const char* path, float** out_data, size_t* out_count);

static void save_tensor_binary(onnx_manager* om, OrtValue* tensor, void** buffer, size_t* len);
//int write_wav_float32_mono(const char* path, const float* samples,
//    uint32_t nframes, uint32_t sample_rate /* e.g., 24000 */);
int onnx_create(onnx_manager** om)
{
	(*om) = calloc(sizeof(onnx_manager), 1);
	if (!(*om))
		return -1;
	return 0;

}
int onnx_init(onnx_manager* om, kt_params* kp)
{
    if (!om)
        return -1;
    void* h = os_load_library(ONNX_DLL);
    if (!h)
    {
        os_print_last_error("Unable to load onnxruntime.dll");
        return -1;
    }
    
    OrtGetApiBase_t OrtGetApiBase_ptr = (OrtGetApiBase_t)os_get_function(h, "OrtGetApiBase");
    if (!OrtGetApiBase_ptr)
    {
        os_print_last_error("Unable to load OrtGetApiBase");
        return -1;
    }
    OrtApiBase* base =OrtGetApiBase_ptr();
    if (!base)
    {
        os_print_last_error("Unable to load OrtApiBase");
        return -1;
    }
    om->api = base->GetApi(ORT_API_VERSION);
    if (!om->api)
    {
        os_print_last_error("Unable to load Ort API");
        return -1;
    }
    om->api->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "kittentts", &om->env);
    if (!om->env)
    {
        os_print_last_error("Unable to api->CreateEnv");
        return -1;
    }
    om->api->CreateSessionOptions(&om->session_options);
    if (!om->env)
    {
        os_print_last_error("Unable to api->CreateSessionOptions");
        return -1;
    }
    // Fast defaults (CPU)
    om->api->SetSessionExecutionMode(om->session_options, 0);        // 0=SEQUENTIAL, 1=PARALLEL 0GOOD
    om->api->SetSessionGraphOptimizationLevel(om->session_options, 1); // 0=DISABLE, 1=BASIC, 2=EXT, 99=ALL 1GOOD

    // Threading (tune these two; start here)
    //om->api->SetIntraOpNumThreads(om->session_options, /*cores*/ 4); // heavy ops (GEMM/conv) parallelism
    //om->api->SetInterOpNumThreads(om->session_options, 1);           // parallel nodes/branches

    //om->api->EnableCpuMemArena(om->session_options);
    //om->api->EnableMemPattern(om->session_options);	
    
    /*
    OrtCUDAProviderOptionsV2* cuda_opts = NULL;
    om->api->CreateCUDAProviderOptions(& cuda_opts);
    const char* keys[] = { "device_id", "do_copy_in_default_stream" };
    const char* vals[] = { "0", "1" }; // <- 1 is critical for Loop on CUDA
    om->api->UpdateCUDAProviderOptions(cuda_opts, keys, vals, 2);

    om->api->EnableProfiling(om->session_options, L"KITTEN.JSON");
    om->api->SetSessionLogSeverityLevel(om->session_options, 1);
    om->api->SessionOptionsAppendExecutionProvider_CUDA_V2(om->session_options, cuda_opts);
    */
    
#ifdef _WIN32
    ORTCHAR_T* model_path_str = os_char_to_wchar(kp->init.model_path);
#else
    ORTCHAR_T* model_path_str = strdup(kp->init.model_path);
#endif    
    om->api->CreateSession(om->env, model_path_str, om->session_options, &om->session);
    free(model_path_str);
    if (!om->session)
    {
        os_print_last_error("Unable to api->CreateSession (==>the model is used here for the first time)");
        return -1;
    }
    load_float32_array(kp->init.voice_path, &om->voice, &om->voice_size);
    if (!om->voice)
    {
        os_print_last_error("Unable to load voice");
        return -1;
    }
    om->api->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &om->memory_info);
    if (!om->memory_info)
    {
        os_print_last_error("Unable to CreateCpuMemoryInfo");
        return -1;
    }
    return 0;
}
int onnx_run(onnx_manager* om, kt_params* kp)
{
    if (!om)
        return -1;
    size_t input_ids_len = kp->run.input_ids_len;
    float speed_value = 1.0;
    const char* input_names[] = { "input_ids", "style", "speed" };
    OrtValue* input_tensors[3] = { NULL };
    input_ids_len = input_ids_len +2;
    int64_t input_ids_shape[] = {1, (int64_t)input_ids_len };

    int64_t* input_ids_data = calloc(sizeof(int64_t), input_ids_len);
    if (!input_ids_data)
        return -1;
    for (size_t i = 1; i < input_ids_len-1; i++) {
        input_ids_data[i] = (int64_t)kp->run.input_ids[i-1];
    }
    int64_t style_len = om->voice_size;
    om->api->CreateTensorWithDataAsOrtValue(
        om->memory_info, input_ids_data,
        input_ids_len * sizeof(int64_t),
        input_ids_shape, 2, ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64,
        &input_tensors[0]);

    int64_t* style_data = calloc(om->voice_size ,sizeof(int64_t));
    if (!style_data)
        return -1;
    for (size_t i = 0; i < om->voice_size; i++) {
        style_data[i] = (int64_t)om->voice[i];
    }

    int64_t style_shape[] = {1, (int64_t)style_len };
    om->api->CreateTensorWithDataAsOrtValue(
        om->memory_info, om->voice,
        style_len * sizeof(float),
        style_shape, 2, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
        &input_tensors[1]);

    int64_t speed_shape[] = { 1 };
    float speed_array[1] = { speed_value };
    om->api->CreateTensorWithDataAsOrtValue(
        om->memory_info, speed_array,
        sizeof(float),
        speed_shape, 1, ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
        &input_tensors[2]);
    const char* output_names[1] = { "waveform" };
    OrtValue* output_tensors[1] = { NULL }; 
    OrtStatus* status=om->api->Run(om->session, NULL,
        input_names, (const OrtValue* const*)input_tensors, 3,
        output_names, 1,(OrtValue **) &output_tensors);
    if (status) {
        const char* msg = om->api->GetErrorMessage(status);
        fprintf(stderr, "ONNX Runtime Error: %s\n", msg);
        om->api->ReleaseStatus(status);
        exit(EXIT_FAILURE);
    }
    save_tensor_binary(om, output_tensors[0], &kp->run.output,&kp->run.output_len);

    for (int i = 0; i < 3; i++) {
        om->api->ReleaseValue(input_tensors[i]);
    }
   
    free(input_ids_data);
    free(style_data);
	return 0;
}
int onnx_destroy(onnx_manager* om)
{
    if (om)
    {
        if (om->session_options)
        {
            om->api->ReleaseSessionOptions(om->session_options);
            om->session_options = 0;
        }
        if (om->session)
        {
            om->api->ReleaseSession(om->session);
            om->session = 0;
        }
        if (om->env)
        {
            om->api->ReleaseEnv(om->env);
            om->env = 0;
        }
        if (om->voice)
        {
            free(om->voice);
            om->voice = 0;
        }
        if (om->memory_info)
        {
            om->api->ReleaseMemoryInfo(om->memory_info);
            om->memory_info = 0;
        }
        om->api = 0;
        free(om);
    }
    return 0;
}

static int is_little_endian(void) {
    uint16_t x = 1;
    return *(uint8_t*)&x == 1;
}
static int load_float32_array(const char* path, float** out_data, size_t* out_count) {

    if (!path || !out_data || !out_count) return 1;

    *out_data = NULL;
    *out_count = 0;

    FILE* f = NULL;
    f=fopen(path, "rb");
    if(!f)
      return 2;

    // Get file size (supports large files)
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 3; }
    int fsize = ftell(f);
    if (fsize < 0) { fclose(f); return 3; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 3; }

    // Must be a multiple of 4 bytes (float32)
    if ((fsize % 4) != 0) { fclose(f); return 4; }

    size_t count = (size_t)(fsize / 4);
    if (count == 0) { fclose(f); return 5; }

    float* buf = (float*)malloc(count * sizeof(float));
    if (!buf) { fclose(f); return 6; }

    size_t read = fread(buf, sizeof(float), count, f);
    fclose(f);
    if (read != count) { free(buf); return 7; }

    // If running on big-endian (rare on Windows), swap bytes in-place.
    if (!is_little_endian()) {
        uint8_t* b = (uint8_t*)buf;
        for (size_t i = 0; i < count; ++i) {
            uint8_t* p = &b[i * 4];
            uint8_t t0 = p[0]; p[0] = p[3]; p[3] = t0;
            uint8_t t1 = p[1]; p[1] = p[2]; p[2] = t1;
        }
    }

    *out_data = buf;
    *out_count = count;
    return 0;
}
static void save_tensor_binary(onnx_manager* om, OrtValue* tensor, void** buffer,size_t* len) {
    int is_tensor = 0;
    om->api->IsTensor(tensor, &is_tensor);
    if (!is_tensor) { fprintf(stderr, "Not a tensor\n"); return; }

    OrtTensorTypeAndShapeInfo* info = NULL;
    om->api->GetTensorTypeAndShape(tensor, &info);

    size_t n_elem = 0;
    om->api->GetTensorShapeElementCount(info, &n_elem);

    ONNXTensorElementDataType et;
    om->api->GetTensorElementType(info, &et);
    void* data_ptr = NULL;
    om->api->GetTensorMutableData(tensor, &data_ptr);
    *buffer = calloc(n_elem, sizeof(float));
    memcpy(*buffer, data_ptr, sizeof(float) * n_elem);
    *len = n_elem;
    //write_wav_float32_mono(path, data_ptr,(uint32_t) n_elem, 24000);
    
    om->api->ReleaseTensorTypeAndShapeInfo(info);
}

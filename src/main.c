#include "kittentts.h"
#define MODEL_PATH "kitten_tts_nano_v0_1.onnx"
#define VOICE_PATH "expr-voice-2-m.bin"
int main(void)
{

    const char* text = "Kitten TTS is an open-source series of tiny and expressive Text-to-Speech models for on-device applications. Our smallest model is less than 25 megabytes.";
    const int print_debug = 1;

    phonemes_manager* pm;
    pm_create(&pm);
    pm_init(pm);
    int* input_ids;
    size_t input_ids_len;
    pm_encode(pm, text, &input_ids, &input_ids_len);
   

    onnx_manager* om;
    onnx_create(&om);
    onnx_init(om,MODEL_PATH, VOICE_PATH);
    onnx_run(om, input_ids, input_ids_len,"audio1.wav");
    onnx_destroy(om);
    /*const OrtApi* g_ort_api;
    g_ort_api = OrtGetApiBase()->GetApi(ORT_API_VERSION);*/
    return 0;
}

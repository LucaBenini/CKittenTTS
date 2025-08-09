#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int write_u16_le(FILE* f, uint16_t v) {
    uint8_t b[2] = { (uint8_t)(v & 0xFF), (uint8_t)((v >> 8) & 0xFF) };
    return fwrite(b, 1, 2, f) == 2 ? 0 : -1;
}

static int write_u32_le(FILE* f, uint32_t v) {
    uint8_t b[4] = {
        (uint8_t)(v & 0xFF),
        (uint8_t)((v >> 8) & 0xFF),
        (uint8_t)((v >> 16) & 0xFF),
        (uint8_t)((v >> 24) & 0xFF)
    };
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}

/* Write a mono float32 WAV. samples = pointer to nframes floats in range ~[-1,1]. */
int write_wav_float32_mono(const char* path, const float* samples,
    uint32_t nframes, uint32_t sample_rate /* e.g., 24000 */)
{
    const uint16_t num_channels = 1;
    const uint16_t bits_per_sample = 32;
    const uint16_t audio_format = 3;   // IEEE float
    const uint32_t byte_rate = sample_rate * num_channels * (bits_per_sample / 8);
    const uint16_t block_align = num_channels * (bits_per_sample / 8);

    const uint32_t data_bytes = nframes * block_align;
    const uint32_t riff_size = 4 /*"WAVE"*/ + (8 + 16) /*fmt chunk*/ + (8 + data_bytes) /*data*/;

    FILE* f = fopen(path, "wb");
    if (!f) return -1;

    // RIFF header
    fwrite("RIFF", 1, 4, f);
    if (write_u32_le(f, riff_size) < 0) goto fail;
    fwrite("WAVE", 1, 4, f);

    // fmt chunk
    fwrite("fmt ", 1, 4, f);
    if (write_u32_le(f, 16) < 0) goto fail;                    // PCM-style fmt chunk size
    if (write_u16_le(f, audio_format) < 0) goto fail;          // 3 = IEEE float
    if (write_u16_le(f, num_channels) < 0) goto fail;          // mono
    if (write_u32_le(f, sample_rate) < 0) goto fail;           // 24000
    if (write_u32_le(f, byte_rate) < 0) goto fail;
    if (write_u16_le(f, block_align) < 0) goto fail;
    if (write_u16_le(f, bits_per_sample) < 0) goto fail;

    // data chunk
    fwrite("data", 1, 4, f);
    if (write_u32_le(f, data_bytes) < 0) goto fail;

    // sample data (little-endian float32)
    if (fwrite(samples, 1, data_bytes, f) != data_bytes) goto fail;

    fclose(f);
    return 0;
fail:
    fclose(f);
    return -2;
}
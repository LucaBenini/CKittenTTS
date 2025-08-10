#include "kittentts.h"

#include <stdio.h>
#include <wchar.h>
#include <locale.h>
static const int AUDIO_OUTPUT_RETRIEVAL = 1;
static const wchar_t* PAD = L"$";
static const wchar_t* PUNCT = L";:,.!?¡¿—…“«»\"\" ";
static const wchar_t* LETTERS = L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static const wchar_t* LETTERS_IPA =
L"ɑɐɒæɓʙβɔɕçɗɖðʤəɘɚɛɜɝɞɟʄɡɠɢʛɦɧħɥʜɨɪʝɭɬɫɮʟɱɯɰŋɳɲɴøɵɸθœɶʘɹɺɾɻʀʁɽʂʃʈʧʉʊʋⱱʌɣɤʍχʎʏʑʐʒʔʡʕʢǀǁǂǃˈˌːˑʼʴʰʱʲʷˠˤ˞↓↑→↗↘'̩'ᵻ";

#define espeakCHARS_AUTO   0
#define espeakCHARS_UTF8   1
#define espeakCHARS_8BIT   2
#define espeakCHARS_WCHAR  3
#define espeakCHARS_16BIT  4

#define espeakSSML        0x10
#define espeakPHONEMES    0x100
#define espeakENDPAUSE    0x1000
#define espeakKEEP_NAMEDATA 0x2000
#define espeakPHONEMES_SHOW    0x01
#define espeakPHONEMES_IPA     0x02
#define espeakPHONEMES_TRACE   0x08
#define espeakPHONEMES_MBROLA  0x10
#define espeakPHONEMES_TIE     0x80

typedef struct {
    wchar_t* symbols;   // concatenated list of all allowed symbols
	size_t   count;     // number of symbols
} text_cleaner;
/* Typedefs */
typedef int (*espeak_Initialize_t)(int output, int buflength, const char* path, int options);
typedef int (*espeak_SetVoiceByName_t)(const char* name);
typedef const char* (*espeak_TextToPhonemes_t)(const void** textptr, int textmode, int phonememode);
typedef int (*espeak_Terminate_t)(void);

typedef struct phonemes_manager
{
	int loaded;
	text_cleaner* cleaner;
    espeak_Initialize_t espeak_Initialize;
    espeak_SetVoiceByName_t espeak_SetVoiceByName;
    espeak_TextToPhonemes_t espeak_TextToPhonemes;
    espeak_Terminate_t espeak_Terminate;
} phonemes_manager;

static text_cleaner* tc_create(void);
static void tc_destroy(text_cleaner* tc);
static int tc_lookup(const text_cleaner* tc, wchar_t ch);
static int* tc_encode(const text_cleaner* tc, const wchar_t* text, size_t* out_len);
static int ensure_capacity(wchar_t** buf, size_t* cap, size_t need_chars);

int pm_create(phonemes_manager** pm)
{
	(*pm) = calloc(sizeof(phonemes_manager), 1);
	if (!(*pm))
		return -1;
    (*pm)->cleaner = tc_create();
	return 0;
}
int pm_init(phonemes_manager* pm)
{
    if (!pm)
        return -1;
    HMODULE h = os_load_library(ESPEAK_DLL);
    if (!h)
    {
        os_print_last_error("Unable to load espeak library");
        return -1;
    }
    pm->espeak_Initialize = (espeak_Initialize_t)os_get_function(h, "espeak_Initialize");
    if (!pm->espeak_Initialize)
    {
        os_print_last_error("Unable to load espeak_Initialize");
        return -1;
    }

    pm->espeak_SetVoiceByName = (espeak_SetVoiceByName_t)os_get_function(h, "espeak_SetVoiceByName");
    if (!pm->espeak_SetVoiceByName)
    {
        os_print_last_error("Unable to load espeak_SetVoiceByName");
        return -1;
    }

    pm->espeak_TextToPhonemes = (espeak_TextToPhonemes_t)os_get_function(h, "espeak_TextToPhonemes");
    if (!pm->espeak_TextToPhonemes)
    {
        os_print_last_error("Unable to load espeak_TextToPhonemes");
        return -1;
    }

    pm->espeak_Terminate = (espeak_Terminate_t)os_get_function(h, "espeak_Terminate");
    if (!pm->espeak_Terminate)
    {
        os_print_last_error("Unable to load espeak_Terminate");
        return -1;
    }
    int sr = pm->espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL,500, 0, 0);
    if (sr <= 0) { fprintf(stderr, "Failed to initialize eSpeak-NG\n"); return -1; }
    if (pm->espeak_SetVoiceByName("en")) {
        fprintf(stderr, "Voice 'en' not found.\n");
        return -1;
    }
	return 0;
}
int pm_encode(phonemes_manager* pm,const char* text, int** destination, size_t* destination_len)
{
    setlocale(LC_ALL, "en_US.UTF-8");
    if (!pm || !text || !destination || !destination_len)
    {
         fprintf(stderr, "Something is null\n"); 
         return -1; 
    }
    // Prepare pointer-to-pointer for espeak_TextToPhonemes
    wchar_t* pText =os_char_to_wchar(text);
    wchar_t* p = pText;
    int textmode = espeakCHARS_WCHAR;

    int phonememode = espeakPHONEMES_IPA;

    wchar_t* out = NULL;
    size_t out_len = 0;         // used chars (excluding NUL)
    size_t out_cap = 0;         // capacity in chars
    while (p && *(const wchar_t*)p != L'\0') {
        const char* phon = pm->espeak_TextToPhonemes(&p, textmode, phonememode);
        if (!phon) break;
        if (phon[0] == '\0') continue;

        // Determine how many wide chars needed (excluding NUL)
        size_t need_no_nul = mbstowcs(NULL, phon, 0); // counts characters (not bytes), no NUL added
        if (need_no_nul == (size_t)-1) {
            fwprintf(stderr, L"Conversion size query failed\n");
            return -1;
        }
        // Space separator between chunks (only if not first)
        size_t extra_sep = (out_len > 0) ? 1 : 0;

        // Ensure capacity: current + sep + new + NUL
        if (!ensure_capacity(&out, &out_cap, out_len + extra_sep + (size_t)need_no_nul + 1)) {
            fwprintf(stderr, L"Out of memory\n");
            return -1;
        }

        // Add separator if needed
        if (extra_sep) {
            out[out_len++] = L' ';
        }

        //// Convert directly into the destination tail (include NUL for convenience)
        size_t written = mbstowcs(out + out_len, phon, out_cap - out_len);
        if (written == (size_t)-1) {
            fwprintf(stderr, L"Conversion failed\n");
            return -1;
        }

        // 'written' includes the NUL; we only advance by the non-NUL chars
        out_len += (size_t)(written - 1);
        out[out_len] = L'\0';

    }
    *destination = tc_encode(pm->cleaner, out, destination_len);
    free(pText);
    free(out);
    if (!(*destination) || !destination_len)
    {
        return -1;
    }
   
    return 0;
}
int pm_destroy(phonemes_manager* pm)
{
	if (pm)
	{
        if (pm->cleaner)
        {
            tc_destroy(pm->cleaner);
            pm->cleaner = 0;
        }
		free(pm);
	}
	return 0;
}



static text_cleaner* tc_create(void) {
    text_cleaner* tc = (text_cleaner*)malloc(sizeof(text_cleaner));
    if (!tc) return NULL;

    size_t n_pad = wcslen(PAD);
    size_t n_punc = wcslen(PUNCT);
    size_t n_let = wcslen(LETTERS);
    size_t n_ipa = wcslen(LETTERS_IPA);
    size_t total = n_pad + n_punc + n_let + n_ipa;

    // +1 for terminating L'\0'
    tc->symbols = (wchar_t*)malloc((total + 1) * sizeof(wchar_t));
    if (!tc->symbols) {
        free(tc);
        return NULL;
    }

    // Concatenate in the exact order used by the Python code
    wcscpy(tc->symbols, PAD);
    wcscat(tc->symbols, PUNCT);
    wcscat(tc->symbols, LETTERS);
    wcscat(tc->symbols, LETTERS_IPA);

    tc->count = total;
    return tc;
}

// Free resources
static void tc_destroy(text_cleaner* tc) {
    if (!tc) return;
    free(tc->symbols);
    free(tc);
}

// Return the index of a wchar_t in the symbol list, or -1 if not found.
// (Simple linear search is fine here given the modest symbol count.)
static int tc_lookup(const text_cleaner* tc, wchar_t ch) {
    if (!tc) return -1;
    for (size_t i = 0; i < tc->count; ++i) {
        if (tc->symbols[i] == ch) {
            return (int)i;
        }
    }
    return -1;
}

// Encode a wide string into indices. Unknown chars are skipped.
// Returns a newly-allocated int array and writes its length to *out_len.
// Caller must free() the returned array.
static int* tc_encode(const text_cleaner* tc, const wchar_t* text, size_t* out_len) {
    if (!tc || !text || !out_len) return NULL;

    size_t n = wcslen(text);
    int* buf = (int*)malloc(n * sizeof(int)); // worst-case size
    if (!buf) return NULL;

    size_t k = 0;
    for (size_t i = 0; i < n; ++i) {
        int idx = tc_lookup(tc, text[i]);
        if (idx >= 0) {
            buf[k++] = idx;
        }
        // else skip, matching Python's KeyError pass
    }

    // Shrink to fit
    int* out = (int*)realloc(buf, k * sizeof(int));
    if (!out && k > 0) {
        // If realloc fails, keep the original buffer
        out = buf;
    }

    *out_len = k;
    return out;
}
static int ensure_capacity(wchar_t** buf, size_t* cap, size_t need_chars) {
    if (*cap >= need_chars) return 1;
    size_t new_cap = (*cap == 0) ? 256 : *cap;
    while (new_cap < need_chars) new_cap *= 2;
    wchar_t* nbuf = (wchar_t*)realloc(*buf, new_cap * sizeof(wchar_t));
    if (!nbuf) return 0;
    *buf = nbuf;
    *cap = new_cap;
    return 1;
}



#include "os.h"
#include <stdio.h>

/**
 * Convert UTF-8 char* to wchar_t*
 * Returns a newly allocated wide string (caller must free)
 */
wchar_t *os_char_to_wchar(const char *input) {
    if (!input) return NULL;

    size_t len = mbstowcs(NULL, input, 0);
    if (len == (size_t)-1) return NULL; // Conversion error

    wchar_t *output = malloc((len + 1) * sizeof(wchar_t));
    if (!output) return NULL;

    mbstowcs(output, input, len + 1);
    return output;
}

/**
 * Convert wchar_t* to UTF-8 char*
 * Returns a newly allocated string (caller must free)
 */
char *os_wchar_to_char(const wchar_t *input) {
    if (!input) return NULL;

    size_t len = wcstombs(NULL, input, 0);
    if (len == (size_t)-1) return NULL; // Conversion error

    char *output = malloc(len + 1);
    if (!output) return NULL;

    wcstombs(output, input, len + 1);
    return output;
}

void* os_load_library(const char* dllname)
{
    return LoadLibraryA(dllname);
}
void* os_get_function(void* dll, const char* funcname)
{
    if (!dll)
        return 0;
    return GetProcAddress((HMODULE)dll, funcname);
}

void os_print_last_error(const char* msg) {
    DWORD err = GetLastError();
    if (!err) { fprintf(stderr, "%s\n", msg); return; }
    LPSTR buf = NULL;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buf, 0, NULL);
    fprintf(stderr, "%s (GetLastError=%lu)%s%s", msg, (unsigned long)err, buf ? ": " : "", buf ? buf : "");
    if (buf) LocalFree(buf);
}
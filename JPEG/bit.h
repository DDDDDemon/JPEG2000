#ifndef _BIT_H_INCLUDED
#define _BIT_H_INCLUDED

#include <stdio.h>

#ifdef WAVELET_DLL
#define WAVELET_DLL_API __declspec(dllexport)

#define WAVELET_DLL_CC
#else
#define WAVELET_DLL_API
#define WAVELET_DLL_CC
#endif

#pragma pack(push, 1)

typedef struct
{
    FILE* file;
    unsigned char* buffer;
    int idx;
    int mask;
    int bufsize;
    int bufleft;
    int bits_left;
    char mode[2];
} t_bit_file;

#pragma pack(pop)

WAVELET_DLL_API t_bit_file* WAVELET_DLL_CC bit_open(char* name, const char *mode, const int mem_size);
WAVELET_DLL_API int WAVELET_DLL_CC bit_close(t_bit_file* f, unsigned char** mem);
WAVELET_DLL_API int WAVELET_DLL_CC bit_read(const int num, t_bit_file* f);
WAVELET_DLL_API int WAVELET_DLL_CC bit_write(const int bits, const int num, t_bit_file* f);

#endif

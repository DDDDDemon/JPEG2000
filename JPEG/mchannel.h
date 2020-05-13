#ifndef _MCHANNEL_H_INCLUDED
#define _MCHANNEL_H_INCLUDED

#include "bit.h"
#include "wavelet.h"

#pragma pack(push, 1)

typedef struct
{
    t_wv_cchannel*	cc;
    float			max_mse;
    // 内部
    int		bits_alloced, bits_unused, old_bits;
} t_wv_mchannel_params;

#pragma pack(pop)

WAVELET_DLL_API int WAVELET_DLL_CC wv_rgb_to_ycbcr(const int Num, const wv_pel* R, const wv_pel* G, const wv_pel* B, wv_pel* Y, wv_pel* Cb, wv_pel* Cr);
WAVELET_DLL_API int WAVELET_DLL_CC wv_ycbcr_to_rgb(const int Num, const wv_pel* Y, const wv_pel* Cb, const wv_pel* Cr, wv_pel* R, wv_pel* G, wv_pel* B);
WAVELET_DLL_API int WAVELET_DLL_CC wv_init_multi_channels(const int MaxBits, const float Threshold, const int NumChannels, t_wv_mchannel_params* Channels, t_wv_csettings** sets);

#endif // _MCHANNEL_H_INCLUDED

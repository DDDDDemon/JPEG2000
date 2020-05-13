#ifndef _WAVELET_H_INCLUDED
#define _WAVELET_H_INCLUDED

#include "bit.h"

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

//定义一个基本的“像素”类型。它的长度不可小于15比特位＋一个符号位
typedef short int wv_pel;

//最大文件头大小设定为1024
#define wv_MAX_HEADER_SIZE 1024
//最多信道数为16
#define wv_MAX_CHANNELS 16


#pragma pack(push, 1)

typedef struct
{
    int		compressed_size;
    float	abs_mse;	//绝对值均方误差
    float	rel_mse;	//从上一次量化块中得到
} t_wv_quantised_block;

typedef struct
 //一个（编码）块
{
    int		max_value, max_bpp, max_quant;
    int		size, offset;
    int		best_bits;		//在本级的最佳比特位
    float	best_mse;		//本级的最佳均方误差
    int		best_quant;		//最佳量化值
    t_wv_quantised_block*	quant;
} t_wv_cblock;

typedef struct
{
    int		width, owidth;
    int		height, oheight;
    int		size;
    int		max_levels;			//小波系数传输的最大级数
    wv_pel*	channel;
    wv_pel*	reordered_channel; // 小波系数以特定次序排列
    int**	reorder_table;		//用于快速的对块进行排序
    int		max_bits;			//按比特压缩图像的最大尺寸
    float	max_mse, final_mse;
    int		size_min_block;		//最小（起始）块
    int		num_blocks;
    t_wv_cblock*	blocks;
} t_wv_cchannel;

typedef void (*wv_progress_function) (int Current, int End, void* UserData);

typedef struct
{
    int		quant;
    int		num_bits;
} t_wv_cblock_settings;

typedef struct
{
    t_wv_cchannel*	cchannel;
    int		num_bits;
    int		size_min_block;
    float	emse;
    int		num_blocks;
    t_wv_cblock_settings*	blocks;
} t_wv_csettings;

typedef struct
{
    int			num_channels;
    int			width, height;		//存储空间大小（2的n次方）
    int			owidth, oheight;	//初始的宽与高
    wv_pel**	channels;
} t_wv_dchannels;

#pragma pack(pop)

WAVELET_DLL_API int WAVELET_DLL_CC log2i(int max_val);
WAVELET_DLL_API float WAVELET_DLL_CC mse_to_psnr(const float mse);
WAVELET_DLL_API float WAVELET_DLL_CC psnr_to_mse(const float psnr);
WAVELET_DLL_API float WAVELET_DLL_CC wv_calc_psnr(const wv_pel *a, const wv_pel *b, const int width, const int height, const int pitch, float *pmse);
WAVELET_DLL_API t_wv_cchannel* WAVELET_DLL_CC wv_init_channel(const int Width, const int Height, const wv_pel* Channel,const int MaxBits, const int Lossless, int *NumBlocks, int*** ReorderTable, wv_progress_function Progress, void* UserData);
WAVELET_DLL_API void WAVELET_DLL_CC wv_done_channel(t_wv_cchannel* CC, const int FreeReorderTable);
WAVELET_DLL_API int WAVELET_DLL_CC wv_init_channel_settings(t_wv_cchannel* CC, const int MaxBits, const float MaxMSE, t_wv_csettings** Settings);
WAVELET_DLL_API void WAVELET_DLL_CC wv_done_channel_settings(t_wv_csettings* Settings);
WAVELET_DLL_API int WAVELET_DLL_CC wv_encode_channels(const int NumChannels, t_wv_csettings** ChannelSettings, t_bit_file* BF);
WAVELET_DLL_API t_wv_dchannels* WAVELET_DLL_CC wv_init_decode_channels(t_bit_file* BF);
WAVELET_DLL_API void WAVELET_DLL_CC wv_done_decode_channels(t_wv_dchannels* dc);

#endif

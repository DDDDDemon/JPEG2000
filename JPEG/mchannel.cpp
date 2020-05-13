#include "wavelet.h"
#include "mchannel.h"

// 颜色空间转换（数据类型：16.16浮点型）
#define RGB_Y(r, g, b) ((int)(r) * 19595 + (int)(g) * 38470 + (int)(b) * 7471)
#define RGB_Cb(r, g, b) ((int)(r) * -11059 + (int)(g) * -21709 + (int)(b) * 32768)
#define RGB_Cr(r, g, b) ((int)(r) * 32768 + (int)(g) * -27439 + (int)(b) * -5329)
#define YCbCr_R(y, cb, cr) ((int)(y) * 65536 + (int)(cb) * 0 + (int)(cr) * 91881)
#define YCbCr_G(y, cb, cr) ((int)(y) * 65536 + (int)(cb) * -22554 + (int)(cr) * -46802)
#define YCbCr_B(y, cb, cr) ((int)(y) * 65536 + (int)(cb) * 116130 + (int)(cr) * 0)

/***************************************************************************
函数功能：	将给定的不同分离位平面的像素数列从RGB转换为YCrCb
函数输出：	函数返回转换象素的数目
函数输入：	1) Num：需要转换的连续象素的署名女
            2)R, G, B：分别表示红、绿、蓝三色的数列首地址指针
            3)Y, Cb, Cr：同样是指针，分别指向三个结果的存放空间
***************************************************************************/
WAVELET_DLL_API int WAVELET_DLL_CC wv_rgb_to_ycbcr(const int Num, const wv_pel* R, const wv_pel* G, const wv_pel* B, wv_pel* Y, wv_pel* Cb, wv_pel* Cr)
{
    if (Num > 0 && R && G && B && Y && Cb && Cr)
    {
        int i;

        for (i = 0; i < Num; i++)
        {
            Y[i] = (RGB_Y(R[i], G[i], B[i]) + 32768) >> 16;
            Cb[i] = ((RGB_Cb(R[i], G[i], B[i]) + 32767) >> 16) + 128;
            Cr[i] = ((RGB_Cr(R[i], G[i], B[i]) + 32767) >> 16) + 128;
        }
        return Num;
    }
    return 0;
}


/***************************************************************************
函数功能：	从YCrCb转换成RGB
函数输出：	函数返回所转换像素的数目
函数输入：	1) Num：需要转换的连续象素的署名女
            2)R, G, B：分别表示红、绿、蓝三色的数列首地址指针
            3)Y, Cb, Cr：同样是指针，分别指向三个结果的存放空间
*****************************************************************************/
WAVELET_DLL_API int WAVELET_DLL_CC wv_ycbcr_to_rgb(const int Num, const wv_pel* Y, const wv_pel* Cb, const wv_pel* Cr, wv_pel* R, wv_pel* G, wv_pel* B)
{
    if (Num > 0 && Y && Cb && Cr && R && G && B)
    {
        int i;

        for (i = 0; i < Num; i++)
        {
            R[i] = min(255, max(0, (YCbCr_R(Y[i], Cb[i] - 128, Cr[i] - 128) + 32768) >> 16));
            G[i] = min(255, max(0, (YCbCr_G(Y[i], Cb[i] - 128, Cr[i] - 128) + 32768) >> 16));
            B[i] = min(255, max(0, (YCbCr_B(Y[i], Cb[i] - 128, Cr[i] - 128) + 32768) >> 16));
        }
        return Num;
    }
    return 0;
}

/**********************************************************************************
功能描述：	本函数试图找到固定数目信道所能匹配的设置（即满足所有给定限制条件）
函数输出：	函数返回信道所需的比特位的数目（这个数目有可能大于最大比特数目）
函数输入：	略
**********************************************************************************/
WAVELET_DLL_API int WAVELET_DLL_CC wv_init_multi_channels(const int MaxBits, const float Threshold, const int NumChannels, t_wv_mchannel_params* Channels, t_wv_csettings** Sets)
{
    int bits = 0;

    if (NumChannels > 0 && Channels && Sets)
    {
        int i;

        for (i = 0; i < NumChannels; i++)
            Sets[i] = NULL;
        if (MaxBits <= 0)
        {
            //加强mse的限制
            for (i = 0; i < NumChannels; i++)
            {
                bits = wv_init_channel_settings(Channels[i].cc, 0, Channels[i].max_mse, &Sets[i]);
                if (bits <= 0)
                    break;
            }
        }
        else
        {
            //平衡信道，这样各信道的MSE大体相等（包括了最值）
            int num_iter = 0, adjustment;
            int omin_idx = -1, omax_idx = -1;
            t_wv_csettings* lsets[wv_MAX_CHANNELS];
            float closest_match = 65536.0f;

            for (i = 0; i < NumChannels; i++)
            {
                Channels[i].bits_unused = Channels[i].old_bits = 0;
                Sets[i] = NULL;
                lsets[i] = NULL;
            }

            //类型初始状态，均匀的分配比特位
            adjustment = 0;
            for (i = 1; i < NumChannels; i++)
            {
                Channels[i].bits_alloced = MaxBits / NumChannels;
                adjustment += Channels[i].bits_alloced;
            }
            Channels[0].bits_alloced = MaxBits - adjustment;
            adjustment = 1;

            do
            {
                float bmin_mse = 65536.0f, bmax_mse = -1.0f;
                int bmin_idx = -1, bmax_idx = -1;
                int bits_available;

                for (i = 0; i < NumChannels; i++)
                {
                    if (lsets[i] && Channels[i].bits_alloced != Channels[i].old_bits)
                    {
                        wv_done_channel_settings(lsets[i]);
                        lsets[i] = NULL;
                    }
                    if (!lsets[i])
                        bits = wv_init_channel_settings(Channels[i].cc, Channels[i].bits_alloced, 65536.0f, &lsets[i]);
                    else
                        bits = lsets[i]->num_bits;
                    if (bits > 0 && lsets[i])
                    {
                        Channels[i].old_bits = bits; //将重新分配那些未用的比特位
                        Channels[i].bits_unused = Channels[i].bits_alloced - bits;
                        //找出最佳和最差的信道
                        if (lsets[i]->emse - Channels[i].max_mse <= bmin_mse)
                        {
                            bmin_mse = lsets[i]->emse - Channels[i].max_mse;
                            bmin_idx = i;
                        }
                        if (lsets[i]->emse - Channels[i].max_mse >= bmax_mse)
                        {
                            bmax_mse = lsets[i]->emse - Channels[i].max_mse;
                            bmax_idx = i;
                        }
                    }
                    else
                        break;
                }

                if (i != NumChannels || bmin_idx == -1 || bmax_idx == -1)
                {
                    bits = 0;
                    break; //没有比特流，退出
                }
                if (bmax_mse - bmin_mse <= closest_match)
                {
                    for (i = 0; i < NumChannels; i++)
                    {
                        Sets[i] = lsets[i]; //拷贝设置
                        lsets[i] = NULL; //保存不被删除
                    }
                    closest_match = bmax_mse - bmin_mse;
                }
                if ((bmin_idx == bmax_idx) || (bmax_mse - bmin_mse <= Threshold))
                    break; //平衡

                if (bmin_idx == omax_idx && bmax_idx == omin_idx)
                {
                    adjustment++;
                    if (adjustment > 256)
                        break;
                }

                //重新分配比特位
                bits_available = 0; //统计所有未用比特位
                for (i = 0; i < NumChannels; i++)
                    if (i != bmin_idx && i != bmax_idx)
                    {
                        bits_available += Channels[i].bits_unused;
                        Channels[i].bits_alloced -= Channels[i].bits_unused;
                    }


                bits_available += Channels[bmin_idx].bits_alloced + Channels[bmax_idx].bits_alloced;

                Channels[bmin_idx].bits_alloced = (Channels[bmin_idx].bits_alloced * adjustment) / (adjustment + 1);
                Channels[bmax_idx].bits_alloced = bits_available - Channels[bmin_idx].bits_alloced; // gets the rest

                omin_idx = bmin_idx;
                omax_idx = bmax_idx;

                num_iter++;
            } while (num_iter < 1024);

            //释放设置空间 （最佳设置已经拷贝保存）
            for (i = 0; i < NumChannels; i++)
                if (lsets[i] != NULL)
                    wv_done_channel_settings(lsets[i]);
        }

        if (bits != 0)
        {
            //重新计算最终的使用比特位的数目
            bits = 0;
            for (i = 0; i < NumChannels; i++)
                if (Sets[i])
                    bits += Sets[i]->num_bits;
        }
    }
    return bits;
}

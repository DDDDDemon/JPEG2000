#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "wavelet.h"

/*首先必须注意的是，在小波变换的时候采用的是串行工作方式
（即，小波系数存放在原图象的存储区内），因此不得不对小波系数重新排序。
这样一来，对于高速缓存的利用是十分低效率的，却可以节省不少的空间。*/

static void wavelet_row(wv_pel *dst, const wv_pel *src, const int len)
{
    //利用的是双正交小波提升
    int i, mid;
    const wv_pel *ptr;
    wv_pel *avg, *det;

    mid = len / 2;

    ptr = src;
    avg = dst; //直流平均值
    det = dst + mid; //高频细节空间

    *det = ptr[1] - (ptr[2] + ptr[0]) / 2; //细节预处理
    *avg = ptr[0] + (det[0] + det[0]) / 4;
    ptr += 2; det++; avg++;

    for (i = 1; i < mid - 1; i++)
    {
    //左边不存在一个奇邻居，右边提升两次
        *det = ptr[1] - (ptr[2] + ptr[0]) / 2;
        *avg = ptr[0] + (det[0] + det[-1]) / 4;
        ptr += 2; det++; avg++;
    }
    //右边不存在一个偶邻居，左边提升两次
    *det = ptr[1] - (ptr[0] + ptr[0]) / 2;
    *avg = ptr[0] + (det[0] + det[-1]) / 4; //最近平均值计算
}


static void inverse_wavelet_row(wv_pel *dst, const wv_pel *src, const int len)
{
    int i, mid;
    wv_pel *ptr;
    const wv_pel *avg, *det;

    mid = len / 2;

    ptr = dst;
    avg = src;
    det = src + mid;
    //边界情况
    *ptr = avg[0] - det[0] / 2;
    ptr += 2; det++; avg++;

    for (i = 0; i < mid - 1; i++)
    {
        ptr[0] = avg[0] - (det[0] + det[-1]) / 4;
        ptr[-1] = det[-1] + (ptr[0] + ptr[-2]) / 2;
        ptr += 2; det++; avg++;
    }
    ptr[-1] = det[-1] + ptr[-2]; //边界
}


static void wavelet_transform(wv_pel *dst, const int width, const int height, const int levels)
{
    int l;
    wv_pel *tmp_lastrow = (wv_pel*)malloc((width + height + height) * sizeof (wv_pel)); // 宽
    wv_pel *tmp_column_in = tmp_lastrow + width; // 高
    wv_pel *tmp_column_out = tmp_lastrow + width + height;  //高

    for (l = 0; l < levels; l++)
    {
        int w = width >> l, h = height >> l;
        int i;

        // 行
        wavelet_row(tmp_lastrow, dst + (h - 1) * width, w); // put last row in tmp_row
        // 将结果存放在一行之下，并且工作方式是从底端往上
        for (i = (h - 2) * width; i >= 0; i -= width)
            wavelet_row(dst + i + width, dst + i, w);

        // 列
        for (i = 0; i < w; i++)
        {
            int j;

            for (j = 1; j < h; j++)
                tmp_column_in[j - 1] = dst[j * width + i]; //矩阵转置
            tmp_column_in[h - 1] = tmp_lastrow[i]; //放入最近一行
            wavelet_row(tmp_column_out, tmp_column_in, h);
            for (j = 0; j < h; j++)
                dst[j * width + i] = tmp_column_out[j]; //再转置
        }
    }
    free(tmp_lastrow);
}


static void inverse_wavelet_transform(wv_pel *dst, const int width, const int height, const int levels)
{
    int l;
    wv_pel *tmp_lastrow = (wv_pel*)malloc((width + height + height) * sizeof (wv_pel));
    wv_pel *tmp_column_in = tmp_lastrow + width;
    wv_pel *tmp_column_out = tmp_lastrow + width + height;

    for (l = levels - 1; l >= 0; l--)
    {
        int w = width >> l, h = height >> l;
        int i;

        //列
        for (i = 0; i < w; i++)
        {
            int j;

            for (j = 0; j < h; j++)
                tmp_column_in[j] = dst[j * width + i];
            inverse_wavelet_row(tmp_column_out, tmp_column_in, h);
            for (j = 0; j < h - 1; j++)
                dst[(j + 1) * width + i] = tmp_column_out[j];
            tmp_lastrow[i] = tmp_column_out[h - 1];  //存储最后一列
        }

        // 行
        for (i = 0; i < (h - 1) * width; i += width)
            inverse_wavelet_row(dst + i, dst + i + width, w);
        inverse_wavelet_row(dst + (h - 1) * width, tmp_lastrow, w);
    }
    free(tmp_lastrow);
}


static void quantize(wv_pel* dst, const int num, const int T)
{
    if (T > 1)
    {
        int i;

        for (i = 0; i < num; i++)
            *dst++ /= T;
    }
}

#define dead_zone_dequant(v, f) (((v) > 0) ? (((v) * 10 + 5) * (f)) / 10 : ((v) < 0) ? (((v) * 10 - 5) * (f)) / 10 : 0)

static void dequantize(wv_pel* dst, const int num, const int T)
{
    if (T > 1)
    {
        int i;

        //以阈值T1开始小波系数恢复
        for (i = 0; i < num; i++)
            *dst++ = dead_zone_dequant(*dst, T);
    }
}

/*
static float log2(const double max_val)
{
    return (float)(log10(max_val) / log10(2.0));
}
*/

/*标题5版式*****************************************************************
函数输入：	int max_val：	对数函数的自变量（比如log2i(0) = 0）
功能描述：	对给定的一个整数作底为2的对数运算
函数输出：	函数返回对数运算的取整结果
***************************************************************************/
WAVELET_DLL_API int WAVELET_DLL_CC log2i(int max_val)
{
    int i;
    for (i = 0; max_val > 0; max_val >>= 1, i++) ;
    return i;
}

//本函数利用比特交错将重排序索引转换称(x,y)形式的坐标
static void get_zig_zag_idx(const int idx, const int maxx, const int maxy, int* xx, int* yy)
{
    int xbits = log2i(maxx) - 1;
    int ybits = log2i(maxy) - 1;
    int mask;

    mask = 1 << (xbits + ybits - 1);
    *xx = *yy = 0;
    if (ybits >= xbits)
    {
        while (ybits > 0)
        {
            *yy <<= 1;
            *yy |= (idx & mask) > 0;
            mask >>= 1;
            ybits--;
            if (xbits > 0)
            {
                *xx <<= 1;
                *xx |= (idx & mask) > 0;
                mask >>= 1;
                xbits--;
            }
        }
    }
    else
    {
        while (xbits > 0)
        {
            *xx <<= 1;
            *xx |= (idx & mask) > 0;
            mask >>= 1;
            xbits--;
            if (ybits > 0)
            {
                *yy <<= 1;
                *yy |= (idx & mask) > 0;
                mask >>= 1;
                ybits--;
            }
        }
    }
}

static int** init_reorder_tables(const int width, const int owidth, const int height, const int oheight, const int levels)
{
    int** tables = NULL;

    if (width > 2 && height > 2)
    {
        int l;

        tables = (int**)malloc((levels + 1) * sizeof (int*));
        for (l = 1; l <= levels + 1; l++)
        {
                        //确定一块小波系数在坐标值的大小
            int block_size_x = width >> l;
            int block_size_y = height >> l;
            int unused_size_x, unused_size_y;
            int *cur_table;
            int i, j;

            tables[l - 1] = cur_table = (int*)malloc(block_size_x * block_size_y * sizeof (int));

            unused_size_x = (width - owidth) >> l;
            unused_size_y = (height - oheight) >> l;
            for (i = 0; i < block_size_x * block_size_y; i++)
            {
                int k;

                get_zig_zag_idx(i, block_size_x, block_size_y, &j, &k);
                if (j <= (block_size_x - unused_size_x) && k <= (block_size_y - unused_size_y))
                    *cur_table++ = k * width + j;
            }
            // 现在求和所有未用的1（原本应该全部为0）
            for (i = 0; i < min(block_size_y, block_size_y - unused_size_y + 1); i++)
                for (j = block_size_x - unused_size_x + 1; j < block_size_x; j++)
                    *cur_table++ = i * width + j;
            for (i = block_size_y - unused_size_y + 1; i < block_size_y; i++)
                for (j = 0; j < block_size_x; j++)
                    *cur_table++ = i * width + j;
        }
    }
    return tables;
}

static void free_reorder_tables(int** tables, const int levels)
{
    if (tables)
    {
        int i;

        for (i = 0; i <= levels; i++)
            free(tables[i]);
        free(tables);
    }
}

//本函数用来重排序小波变换系数，输出是自定义的表
static void reorder(wv_pel *dst, const wv_pel* src, int** tables, const int width, const int height, const int levels)
{
    if (tables)
    {
        int i, l;
        const int *idx;

        //输出基频均值
        idx = tables[levels];
        for (i = 0; i < (width >> (levels + 1)) * (height >> (levels + 1)); i++)
            *dst++ = src[*idx++];
        for (l = levels + 1; l >= 1; l--)
        {
            //确定一块小波系数在坐标值的大小
            int block_size_x = width >> l;
            int block_size_y = height >> l;

            //交替输出LH端/HL端的小波系数
            idx = tables[l - 1];
            for (i = 0; i < block_size_y * block_size_x; i++)
            {
                *dst++ = src[*idx + block_size_x]; // LH端的小波系数以逐行顺序存放
                *dst++ = src[*idx + block_size_y * width]; // HL小波系数也按逐行顺序存放
                idx++;
            }
            //现在是右边下端的块
            idx = tables[l - 1];
            for (i = 0; i < block_size_x * block_size_y; i++)
                *dst++ = src[*idx++ + block_size_y * width + block_size_x];
        }
    }
    else
        memcpy(dst, src, width * height * sizeof *dst);
}

//本函数用于将重排序函数所自定义的交替顺序恢复为直接可用的数据
static void unreorder(wv_pel* dst, const wv_pel* src, int** tables, const int width, const int height, const int levels)
{
    if (tables)
    {
        int i, l;
        const int *idx;

        //输出基频均值
        idx = tables[levels];
        for (i = 0; i < (width >> (levels + 1)) * (height >> (levels + 1)); i++)
            dst[*idx++] = *src++;
        for (l = levels + 1; l >= 1; l--)
        {
            //确定一块小波系数在坐标值的大小
            int block_size_x = width >> l;
            int block_size_y = height >> l;

            //交替输出LH端/HL端的小波系数
            idx = tables[l - 1];
            for (i = 0; i < block_size_y * block_size_x; i++)
            {
                dst[*idx + block_size_x] = *src++; //LH端的小波系数按“锯齿”顺序存放
                dst[*idx + block_size_y * width] = *src++; //HL端的系数同样按“锯齿”顺序存放
                idx++;
            }
            //现在轮到右边下端的块
            idx = tables[l - 1];
            for (i = 0; i < block_size_x * block_size_y; i++)
                dst[*idx++ + block_size_y * width + block_size_x] = *src++;
        }
    }
    else
        memcpy(dst, src, width * height * sizeof *dst);
}


/*标题5版式*****************************************************************
函数输入：	const float mse：输入均方误差
功能描述：	将mse（均方误差）转换成psnr（峰值信噪比）
函数输出：	返回转换成功的峰值信噪比
***************************************************************************/
WAVELET_DLL_API float WAVELET_DLL_CC mse_to_psnr(const float mse)
{
    if (mse > 0.0f)
        return (float)(20.0 * log10(255.0 / sqrt(mse)));
    return 0.0f;
}

/*标题5版式*****************************************************************
函数输入：	const float psnr：输入峰值信噪比
功能描述：	将psnr（峰值信噪比）转换成mse（均方误差）
函数输出：	返回转换成功的均方误差
***************************************************************************/
WAVELET_DLL_API float WAVELET_DLL_CC psnr_to_mse(const float psnr)
{
    if (psnr > 0.0f)
        return (float)pow(255.0 / pow(10.0, psnr / 20.0), 2.0);
    return 65025.0f;
}

/*标题5版式*****************************************************************
函数输入：	1)const wv_pel* a：	信道a
            2)const wv_pel* b： 信道b
            3)const int width：	待比较矩形的宽度
            4)const int height：待比较矩形的高度
            5)const int pitch：	在均方误差情况下的信道偏差
            6)float* pmse：		均方误差存储区的首地址（浮点型）
功能描述：	计算两个给定信道之间的差异
函数输出：	函数返回该误差的峰值信噪比
***************************************************************************/
WAVELET_DLL_API float WAVELET_DLL_CC wv_calc_psnr(const wv_pel *a, const wv_pel *b,
                                                  const int width, const int height,
                                                  const int pitch, float *pmse)
{
    float sum2 = 0.0f;
    int i, j;
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++)
        {
            float diff = (float)(a[i * pitch + j] - b[i * pitch + j]);
            sum2 += diff * diff;
        }

    if (sum2 > 0.0f)
    {
        float mse = sum2 / (width * height); //均方误差
        if (pmse)
            *pmse = mse;
        return mse_to_psnr(mse);
    }
    else
        *pmse = 0.0f;
    return 0.0f;
}

//编码流程

#define RICE_MIN_K 0
#define RICE_MAX_K 30
#define RICE_MIN_SPACE (2 * 4 + 3)

//块编码
static int rice_encode_block(const wv_pel* src, const int num, const int skip_bp, int k, const int bits_left, t_bit_file* f, int* bit_stat)
{
    int bits_written = 0;
    wv_pel* values;
    unsigned char* flags;
    int i, num_bp, imax = 0;

    if ((num == 0) || (bits_left < RICE_MIN_SPACE))
        return 0;
    values = (wv_pel*)malloc(num * sizeof (wv_pel));
    flags = (unsigned char *)malloc(num * sizeof (unsigned char));
    for (i = 0; i < num; i++)
    {
        values[i] = (src[i] < 0) ? -src[i] : src[i];
        values[i] >>= skip_bp;
        imax = max(imax, values[i]);
        flags[i] = src[i] < 0;
    }
    if (imax > 0)
    {
        num_bp = log2i(imax);
        bits_written += bit_write(num_bp, 4, f);
        if (num_bp > 0)
        {
            int mask;
            bits_written += bit_write(skip_bp, 4, f);
            mask = 1 << (num_bp - 1);
            for (i = 0; i < num_bp; i++)
            {
                int j;
                int num_zeros = 0;

                for (j = 0; j < num; j++)
                    if ((flags[j] & 0x02) == 0)
                    {
                        if ((values[j] & mask) == 0)
                            num_zeros++;
                        else
                        {
                            while (num_zeros >= (1 << k))
                            {
                                if (bits_left - bits_written < 1)
                                {
                                    free(flags);
                                    free(values);
                                    return bits_written;
                                }
                                bits_written += bit_write(0, 1, f);
                                num_zeros -= (1 << k);
                                if (k < RICE_MAX_K)
                                    k++;
                            }
                            if (bits_left - bits_written < 2 + k)
                            {
                                free(flags);
                                free(values);
                                return bits_written;
                            }
                            bits_written += bit_write(1, 1, f);
                            bits_written += bit_write(num_zeros, k, f);
                            bits_written += bit_write(flags[j] & 0x01, 1, f);
                            flags[j] |= 0x04;
                            num_zeros = 0;
                            if (k > RICE_MIN_K)
                                k--;
                        }
                    }
                while (num_zeros >= (1 << k))
                {
                    if (bits_left - bits_written < 1)
                    {
                        free(flags);
                        free(values);
                        return bits_written;
                    }
                    bits_written += bit_write(0, 1, f);
                    num_zeros -= (1 << k);
                    if (k < RICE_MAX_K)
                        k++;
                }
                if (num_zeros > 0)
                {
                    if (bits_left - bits_written < 1)
                    {
                        free(flags);
                        free(values);
                        return bits_written;
                    }
                    bits_written += bit_write(0, 1, f);
                    num_zeros = 0;
                    if (k > RICE_MIN_K)
                        k--;
                }
                for (j = 0; j < num; j++)
                {
                    if ((flags[j] & 0x02) != 0)
                    {
                        if (bits_left - bits_written < 1)
                        {
                            free(flags);
                            free(values);
                            return bits_written;
                        }
                        bits_written += bit_write((values[j] & mask) != 0, 1, f);
                    }
                    else if ((flags[j] & 0x04) != 0)
                        flags[j] |= 0x02;

                }
                mask >>= 1;

                if (bit_stat)
                    bit_stat[i] = bits_written;
            }
        }
    }
    else
    {
        bits_written += bit_write(0, 4, f);
        if (bit_stat)
            bit_stat[0] = bits_written;
    }

    free(flags);
    free(values);
    return bits_written;
}

static int rice_decode_block(wv_pel* reordered, const int num, int k, t_bit_file* bf)
{
    if (num > 0 && reordered && bf)
    {
        int num_bp = 0;
        int num_bits = num; //利用行程编码的比特位数目

        memset(reordered, 0, num * sizeof *reordered);

        num_bp = bit_read(4, bf);
        if (num_bp > 0)
        {
            unsigned char* flags = (unsigned char *)malloc(num * sizeof (unsigned char));
            int i, skip_bp;

            skip_bp = bit_read(4, bf);
            memset(flags, 0, num * sizeof *flags);
            for (i = 0; i < num_bp; i++)
            {
                int num_zeros = 0;
                int next_num_bits = num_bits;
                int	j = 0, idx = 0;

                while (j < num_bits)
                {
                    if (bit_read(1, bf) == 0)
                    {
                        num_zeros += 1 << k;
                        j += 1 << k;
                        if ((j > num_bits) && (k > RICE_MIN_K))
                            k--; //以0填充，位平面结束标志
                        else if ((j <= num_bits) && (k < RICE_MAX_K))
                            k++; //动态调整k值
                    }
                    else
                    {
                        int numz = bit_read(k, bf);

                        num_zeros += numz;
                        j += numz + 1;
                        while (num_zeros > 0)
                        {
                            if ((flags[idx] & 0x02) == 0)
                            {
                                reordered[idx] <<= 1;
                                num_zeros--;
                            }
                            idx++;
                        }
                        while ((flags[idx] & 0x02) != 0)
                            idx++;
                        reordered[idx] = (reordered[idx] << 1) | 0x01;
                         //标记为细化起始位和符号起始
                        flags[idx] |= 0x04 | bit_read(1, bf);
                        idx++;
                        next_num_bits--; // 1 less rice-encoded bit
                        if (k > RICE_MIN_K)
                            k--; //动态调整k值
                    }
                }
                if (num_zeros > 0)
                {
                    //最末比特0的特许
                    for (; (idx < num) && (num_zeros > 0); idx++)
                        if ((flags[idx] & 0x02) == 0)
                        {
                            reordered[idx] <<= 1;
                            num_zeros--;
                        }
                    num_zeros = 0;
                }
                num_bits = next_num_bits; //下一平面剩余的比特将进行编码
                //读入细化比特（未编码）
                for (idx = 0; idx < num; idx++)
                {
                    if ((flags[idx] & 0x02) != 0)
                        reordered[idx] = (reordered[idx] << 1) | bit_read(1, bf);
                    else if ((flags[idx] & 0x04) != 0)
                        flags[idx] |= 0x02; //标记为下一位平面的细化部分
                }
            }
            for (i = 0; i < num; i++)
                if ((flags[i] & 0x01) != 0x00)
                    reordered[i] = -reordered[i]; //设置符号
            free(flags);
            dequantize(reordered, num, 1 << skip_bp); //将数值提高
            return num;
        }
    }
    return 0;
}


//返回最接近的一个大浮点数
#define flarger(fl) ((fl) == 0.0f ? FLT_MIN : (fl) + (fl) * FLT_EPSILON)

/*标题5版式*******************************************************************
函数输入:	1)const int Width：信道的初始宽度
            2)const int Height：信道的初始高度
            3)const wv_pel* Channel：信道中的图像像素数据
            4)const int MaxBits：用于压缩。如果NumBlocks不为0，它仅仅是用作
            启发式块数目计算的一个标示
            5)const int Lossless：是否无损压缩标识
            6)int* NumBlocks：一个整型指针数据，包含了需使用的块的数目
            7)int*** ReorderTable：重排序表指针的指针
            8)wv_progress_function Progress：用来渐进显示的功能回调，可以是NULL
            9)void* UserData：用户信息，渐进函数
数据说明：	1)const wv_pel* Channel：它必须存放宽、高值，宽、高值的大小是与给定参数
            Width和Height最接近的2的指数函数值。必须给图像数据足够的空间和用于运算的
            合适的宽高数据值。没有使用的数据控件的值以0填充。例如，原始信道大小是720×
            480，也就是说Width＝720，Height＝480。但是实际信道中的数据大小必须是1025×
            512。
            2)const int Lossless：如果用户仅仅需要的是无损压缩（这样做可以大大提高运算
            的速度），那么应该赋值为非0
            3)int* NumBlocks：假如这个指针为空，或者*NumBlocks==0，那么就要用到启发式的
            猜测（如果MaxBits可用，就要用到它了）
            4)int*** ReorderTable：假设指针所指为空，那么将建立一个新的表，并将它的首地址
            赋值给它
功能描述:	初始化小波编码所需要的一个信道
函数输出：	函数返回信道的句柄（用来寻找压缩参数）
*****************************************************************************/
WAVELET_DLL_API t_wv_cchannel* WAVELET_DLL_CC wv_init_channel(const int Width, const int Height, const wv_pel* Channel,
    const int MaxBits, const int Lossless, int *NumBlocks, int*** ReorderTable, wv_progress_function Progress, void* UserData)
{
    t_wv_cchannel* cc = NULL;

    if (Width >= 1 && Height >= 1 && Channel)
    {
        int i;
        wv_pel* tmp_wav, *channel;
        int cur, end;

        cc = (t_wv_cchannel*)malloc(sizeof (t_wv_cchannel));	//分配信道空间
        //信道初始化
        cc->owidth = Width;
        cc->oheight = Height;
        cc->width = 1 << log2i(cc->owidth - 1);
        cc->height = 1 << log2i(cc->oheight - 1);
        cc->size = cc->width * cc->height;
        cc->max_levels = max(0, log2i(min(cc->width, cc->height)) - 2);

        cc->reorder_table = (ReorderTable && *ReorderTable) ? *ReorderTable
            : init_reorder_tables(cc->width, cc->owidth, cc->height, cc->oheight, cc->max_levels);
        if (ReorderTable && *ReorderTable == NULL)
            *ReorderTable = cc->reorder_table;

        channel = (wv_pel*)malloc(cc->size * sizeof (wv_pel));
        cc->reordered_channel = (wv_pel*)malloc(cc->size * sizeof (wv_pel));
        //用作经过扩展的信道（wv_pel* Channel）的临时存储空间
        memcpy(channel, Channel, cc->size * sizeof *channel);
        /*小波变换，但是在这个整合JPEG2000系统中，为了信道设置的需要没有使用Lazy DWT。
        感兴趣的读者可以将先前介绍的Lazy DWT代替此处的小波变换，压缩效果会得到进一步的提高*/
        wavelet_transform(channel, cc->width, cc->height, cc->max_levels);

        if (cc->width >= 4 && cc->height >= 4 && (cc->width != cc->owidth || cc->height != cc->oheight))
        {
            //将没有使用到的小波系数用0填充
            int j, k, nwidth, nheight;

            i = 1;
            nwidth = (cc->width - cc->owidth) >> i;
            nheight = (cc->height - cc->oheight) >> i;
            while (nwidth > 0 || nheight > 0)
            {
                int lwidth, lheight;
                wv_pel *hd, *vd, *dd;

                lwidth = cc->width >> i;
                lheight = cc->height >> i;
                hd = channel + lwidth; //水平方向细节
                vd = channel + lheight * cc->width; //竖直方向细节
                dd = channel + lwidth + lheight * cc->width; //对角线方向细节
                for (j = 0; j < min(lheight, lheight - nheight + 1); j++)
                    for (k = lwidth - nwidth + 1; k < lwidth; k++)
                    {
                        hd[j * cc->width + k] = 0;
                        vd[j * cc->width + k] = 0;
                        dd[j * cc->width + k] = 0;
                    }
                for (j = lheight - nheight + 1; j < lheight; j++)
                    for (k = 0; k < lwidth; k++)
                    {
                        hd[j * cc->width + k] = 0;
                        vd[j * cc->width + k] = 0;
                        dd[j * cc->width + k] = 0;
                    }
                i++;
                //填充区域的宽、高分别右移指定位数
                nwidth = (cc->width - cc->owidth) >> i;
                nheight = (cc->height - cc->oheight) >> i;
            }
        }
        //详细的描述在reorder函数中
        reorder(cc->reordered_channel, channel, cc->reorder_table, cc->width, cc->height, cc->max_levels);
        //图像压缩的最大尺寸
        cc->max_bits = max(0, MaxBits);

        if (NumBlocks == NULL || *NumBlocks <= 0)
        {
            //计算块的大小（启发式运算）
            if (cc->max_bits > 0)
            {
                //计算初始块的最小尺寸
                int imax;
                int bpp;
                int min_levels = 0;

                //计算传输速率
                imax = 0;
                for (i = 0; i < cc->size; i++)
                {
                    int j = (cc->reordered_channel[i] < 0) ? -cc->reordered_channel[i] : cc->reordered_channel[i];

                    imax = max(imax, j);
                }
                bpp = log2i(imax);
                bpp = (bpp >> 2) << 2; //使传输速率在4的倍数附近

                for (i = cc->size_min_block = (cc->width >> cc->max_levels) * (cc->height >> cc->max_levels); i * bpp <= cc->max_bits / 16; i <<= 2)
                    min_levels++;
                cc->num_blocks = (cc->max_levels + 1) - min_levels + 1;

                if (cc->num_blocks <= 3)
                    cc->num_blocks = max(1, cc->max_levels / 2);
            }
            else
                // 在没有比特限制的情况下，对块个数的选择增加一点任意性
                cc->num_blocks = max(1, cc->max_levels - 3);
        }
        else
            cc->num_blocks = *NumBlocks;

        if (NumBlocks)
            *NumBlocks = cc->num_blocks;

        // 保存已做的设置修改
        cc->num_blocks = max(1, min(cc->max_levels, cc->num_blocks)); // 1-MaxLevel blocks
        cc->size_min_block = (cc->width >> (cc->num_blocks - 1)) * (cc->height >> (cc->num_blocks - 1));

        cc->final_mse = 65536.0f;

        tmp_wav = (wv_pel*)malloc(cc->size * sizeof (wv_pel));

        cc->blocks = (t_wv_cblock*)malloc(cc->num_blocks * sizeof (t_wv_cblock));
        end = 0;
        for (i = 0; i < cc->num_blocks; i++)
        {
            //已经完成块运算，还必须接下来的必须做的工作
            t_wv_cblock* bl = &cc->blocks[i];
            int j;
            //块的初始设置
            bl->offset = (i == 0) ? 0 : (cc->size_min_block << (2 * (i - 1)));
            bl->size = (i == 0) ? cc->size_min_block : ((cc->size_min_block << (2 * i)) - bl->offset);
            bl->max_value = 0;
            for (j = bl->offset; j < bl->offset + bl->size; j++)
                bl->max_value = max(bl->max_value, (cc->reordered_channel[j] < 0) ? -cc->reordered_channel[j] : cc->reordered_channel[j]);
            //块的传输速率和最大量化设置
            bl->max_bpp = log2i(bl->max_value);
            bl->max_quant = Lossless ? 0 : bl->max_bpp;
            end += bl->max_quant + 1;
        }

        //建立字典
        end--;
        cur = 0;
        for (i = 0; i < cc->num_blocks; i++)
        {
            t_wv_cblock* bl = &cc->blocks[i];
            t_bit_file* bf;
            int* bit_stat;
            int j;
            //确定足够多的空间，多余的空间使不需要的
            bit_stat = (int*)malloc(bl->max_bpp * sizeof (int));
            //打开一个比特流文件
            bf = bit_open(NULL, "wm", wv_MAX_HEADER_SIZE + bl->size * bl->max_bpp * 2);
            rice_encode_block(cc->reordered_channel + bl->offset, bl->size, 0, 0, wv_MAX_HEADER_SIZE + bl->size * bl->max_bpp * 2, bf, bit_stat);
            bit_close(bf, NULL);
            //临时空间
            memcpy(tmp_wav, cc->reordered_channel, bl->offset * sizeof *tmp_wav);
            memset(tmp_wav + bl->offset + bl->size, 0, (cc->size - (bl->offset + bl->size)) * sizeof *tmp_wav);

            bl->quant = (t_wv_quantised_block*)malloc((bl->max_quant + 1) * sizeof (t_wv_quantised_block));
            for (j = 0; j <= bl->max_quant; j++)
            {
                t_wv_quantised_block* ebl = &bl->quant[j];

                if (j == bl->max_bpp)
                    ebl->compressed_size = 4;
                else
                    ebl->compressed_size = bit_stat[bl->max_bpp - j - 1];
                memcpy(tmp_wav + bl->offset, cc->reordered_channel + bl->offset, bl->size * sizeof *tmp_wav);
                quantize(tmp_wav + bl->offset, bl->size, 1 << j);	//量化
                dequantize(tmp_wav + bl->offset, bl->size, 1 << j);	//恢复
                unreorder(channel, tmp_wav, cc->reorder_table, cc->width, cc->height, cc->max_levels);
                //小波变换的逆变换。同样在这里的系统中逆变换不是Lazy小波逆变换
                inverse_wavelet_transform(channel, cc->width, cc->height, cc->max_levels);
                wv_calc_psnr(channel, Channel, cc->owidth, cc->oheight, cc->width, &ebl->abs_mse);

                if (j > 0)
                    ebl->abs_mse = max(ebl->abs_mse, flarger(cc->blocks[i].quant[j - 1].abs_mse));

                if (ebl->abs_mse == 0.0f || i == 0)
                {
                    //处理无损压缩设置的情况
                    int k;

                    ebl->rel_mse = 65536.0f;
                    for (k = 0; k < i; k++)
                        ebl->rel_mse -= cc->blocks[k].quant[j].rel_mse;
                    ebl->rel_mse -= ebl->abs_mse;
                }
                else
                    //现在mse包含了本块超出上一块的mse增量
                    ebl->rel_mse = cc->blocks[i - 1].quant[0].abs_mse - ebl->abs_mse;
                if (Progress)
                    Progress(cur, end, UserData);
                cur++;
            }

            free(bit_stat);
        }
        free(tmp_wav);
        free(channel);
    }
    return cc;
}


/*标题5版式*******************************************************************
函数输入：	1)t_wv_cchannel* CC：	指定的信道
            2)const int FreeReorderTable：	如果想要释放制定信道的ReorderTable
            表（如果是一个共享类型的表，只进行一次释放），则它不为0
功能描述：	释放一个信道和与之相关的所有数据
*****************************************************************************/
WAVELET_DLL_API void WAVELET_DLL_CC wv_done_channel(t_wv_cchannel* CC,
                                                    const int FreeReorderTable)
{
    if (CC)		//如果该信道不为空，则释放掉
    {
        int i;
        for (i = 0; i < CC->num_blocks; i++)
            free(CC->blocks[i].quant);
        free(CC->blocks);

        if (FreeReorderTable)
            free_reorder_tables(CC->reorder_table, CC->max_levels);
        free(CC->reordered_channel);
        free(CC);
    }
}

// 量化环节

static float recursive_selector_bits(t_wv_cchannel* cc, const int block_idx, const int bits_left, const float mse)
{
    int i, need_left;
    t_wv_cblock* bl;
    float best_lmse, max_mse_improvement;

    if (block_idx >= cc->num_blocks)
        return mse;

    max_mse_improvement = mse;
    for (i = block_idx; i < cc->num_blocks; i++)
        max_mse_improvement -= cc->blocks[i].quant[0].rel_mse;

    bl = &cc->blocks[block_idx];

    if (max_mse_improvement > bl->best_mse)
        return mse; //如果对于有无限码流可以更好的话，可以试试这一步

    best_lmse = mse;
    need_left = ((cc->num_blocks - 1) - block_idx) * 4; //每块需要4个字节来表明位平面哦个数

    for (i = 0; i <= bl->max_quant; i++)
    {
        t_wv_quantised_block* ebl = &bl->quant[i];

        if (bits_left - ebl->compressed_size >= need_left)
        { //匹配
            float lmse;

            lmse = recursive_selector_bits(cc, block_idx + 1, bits_left - ebl->compressed_size, mse - ebl->rel_mse);
            if (lmse < bl->best_mse)
            {
                if (lmse == mse - ebl->rel_mse)
                {
                    //不需要适合另外的块，因此设置他们的量化值为最大
                    int j;

                    for (j = block_idx + 1; j < cc->num_blocks; j++)
                        cc->blocks[j].best_quant = cc->blocks[j].max_quant;
                }
                bl->best_quant = i;
                bl->best_mse = best_lmse = lmse;
            }
        }
    }
    return best_lmse;
}


static int recursive_selector_mse(t_wv_cchannel* cc, const int block_idx, const int total_bits, const float mse)
{
    int i;
    t_wv_cblock* bl;
    int best_bits;
    float max_mse_improvement;

    if (mse <= cc->max_mse)
    {
        cc->final_mse = mse;
        return total_bits;
    }

    if ((total_bits != 0 && cc->max_bits - total_bits < RICE_MIN_SPACE) || block_idx >= cc->num_blocks)
        return cc->max_bits + 1; //没有满足需要

    max_mse_improvement = mse;
    for (i = block_idx; i < cc->num_blocks; i++)
        max_mse_improvement -= cc->blocks[i].quant[0].rel_mse;

    bl = &cc->blocks[block_idx];

    if (max_mse_improvement > cc->max_mse)
        return cc->max_bits + 1;
    best_bits = cc->max_bits + 1;

    for (i = 0; i <= bl->max_quant; i++)
    {
        t_wv_quantised_block* ebl = &bl->quant[i];

        if (total_bits + ebl->compressed_size <= cc->max_bits)
        { //匹配
            int lbits;

            lbits = recursive_selector_mse(cc, block_idx + 1, total_bits + ebl->compressed_size, mse - ebl->rel_mse);
            if (lbits <= cc->max_bits && lbits < bl->best_bits)
            {
                if (lbits == total_bits + ebl->compressed_size)
                {
                    int j;

                    for (j = block_idx + 1; j < cc->num_blocks; j++)
                    {
                        cc->blocks[j].best_quant = cc->blocks[j].max_quant;
                        cc->blocks[j].best_bits = lbits;
                    }
                }

                bl->best_quant = i;
                bl->best_bits = best_bits = lbits;
                bl->best_mse = mse - ebl->rel_mse;
            }
        }
    }
    return best_bits;
}



/*标题5版式*******************************************************************
函数输入：	1)t_wv_cchannel CC：比特流文件句柄（从wv_init_channel中获得）
            2)const int MaxBits：信道所用到的最大比特数
            3)const float MaxMse：压缩中出现的最大均方误差
            4)t_wv_csettings** Setting：成功存储后信道设置的首地址指针
数据说明：	const float MaxMse：0.0f表示无损压缩
                                65536.of无需考虑
功能描述：	初始化单一信道的压缩设置
函数输出：	将设置存储到用户自定义的指针指向的空间，并返回使用的比特数目（失败返回0）
*****************************************************************************/
WAVELET_DLL_API int WAVELET_DLL_CC wv_init_channel_settings(t_wv_cchannel* CC, const int MaxBits,
                                                            const float MaxMSE, t_wv_csettings** Settings)
{
    int bits = 0;

    if (CC && Settings)
    {
        int i, size;

        for (i = 0; i < CC->num_blocks; i++)
        {
            CC->blocks[i].best_bits = 1 << 30;
            CC->blocks[i].best_mse = 65536.0f;
            CC->blocks[i].best_quant = CC->blocks[i].max_quant;
        }

        CC->max_bits = size = max(0, MaxBits);
        CC->max_mse = min(65536.0f, max(0.0f, MaxMSE));

        if (CC->max_mse < 65536.0f)
        {
            if (CC->max_bits == 0)
                CC->max_bits = 1 << 30;
            size = recursive_selector_mse(CC, 0, 0, 65536.0f);
        }
        else
            CC->final_mse = recursive_selector_bits(CC, 0, CC->max_bits, 65536.0f); // only bit-constraint

        if (CC->max_bits == 0 || size <= CC->max_bits)
        {
            t_wv_csettings* set = (t_wv_csettings*)malloc(sizeof (t_wv_csettings));

            set->cchannel = CC;
            set->size_min_block = CC->size_min_block;
            set->emse = CC->final_mse;
            set->num_bits = 0;
            set->num_blocks = CC->num_blocks;
            set->blocks = (t_wv_cblock_settings*)malloc(CC->num_blocks * sizeof (t_wv_cblock_settings));
            for (i = 0; i < CC->num_blocks; i++)
            {
                set->blocks[i].quant = CC->blocks[i].best_quant;
                set->blocks[i].num_bits = CC->blocks[i].quant[CC->blocks[i].best_quant].compressed_size;
                set->num_bits += set->blocks[i].num_bits;
            }
            bits = set->num_bits;
            *Settings = set;
        }
        else
            *Settings = NULL;
    }
    return bits;
}


/*标题5版式*******************************************************************
函数输入：	t_wv_csettings* Setting：	待释放的信道设置句柄
功能描述：	释放信道设置空间
*****************************************************************************/
WAVELET_DLL_API void WAVELET_DLL_CC wv_done_channel_settings(t_wv_csettings* Settings)
{
    if (Settings)
    {
        free(Settings->blocks);
        free(Settings);
    }
}

/*标题5版式***************************************************************************
函数输入：	1)const int NumChannels：编码信道的个数
            2)t_wv_csettings** ChannelSettings：编码所用到的信道设置（channel-settings）
            的句柄数组
            3)t_bit_file* BF：编码输出的比特流文件句柄（以读模式打开）
数据说明：	t_wv_csettings** ChannelSettings：所有的信道（或信道设置）都必须相同的宽、高
            和块数目。这些块编码存放在同一文件中
功能描述：	渐进编码指定数目的信道，将结果交错存放到给定的比特流文件中。该比特流文件
            有一个小小的文件头，包含了指定数目的信道，以及以2为底的指数函数值的宽和高
函数输出：	函数返回写入比特的数目
*************************************************************************************/
WAVELET_DLL_API int WAVELET_DLL_CC wv_encode_channels(const int NumChannels, t_wv_csettings** ChannelSettings, t_bit_file* BF)
{
    int bits = 0;

    if (NumChannels > 0 && NumChannels <= wv_MAX_CHANNELS && ChannelSettings && BF)
    {
        int i;

        //检查所有的信道的大小以及块的数目是否相同
        for (i = 1; i < NumChannels; i++)
        {
            if (ChannelSettings[0]->num_blocks != ChannelSettings[i]->num_blocks)
                break;
            if (ChannelSettings[0]->cchannel->width != ChannelSettings[i]->cchannel->width)
                break;
            if (ChannelSettings[0]->cchannel->height != ChannelSettings[i]->cchannel->height)
                break;
            if (ChannelSettings[0]->cchannel->size_min_block != ChannelSettings[i]->cchannel->size_min_block)
                break;
        }
        if (i == NumChannels)
        {
            //所有的信道是否匹配
            int j = 1;

            bit_write(ChannelSettings[0]->cchannel->width != ChannelSettings[0]->cchannel->owidth ||
                ChannelSettings[0]->cchannel->height != ChannelSettings[0]->cchannel->oheight, 1, BF);
            if (ChannelSettings[0]->cchannel->width != ChannelSettings[0]->cchannel->owidth ||
                ChannelSettings[0]->cchannel->height != ChannelSettings[0]->cchannel->oheight)
            {
                bit_write(ChannelSettings[0]->cchannel->owidth - 1, 16, BF);
                bit_write(ChannelSettings[0]->cchannel->oheight - 1, 16, BF);
                j += 32;
            }

            bit_write(NumChannels - 1, 4, BF);
            bit_write(log2i(ChannelSettings[0]->cchannel->width) - 1, 4, BF);
            bit_write(log2i(ChannelSettings[0]->cchannel->height) - 1, 4, BF);
            bit_write(ChannelSettings[0]->num_blocks - 1, 4, BF);
            bit_write(log2i(ChannelSettings[0]->size_min_block), 5, BF);

            bits += 21 + j; //头信息大小为21个比特

            for (i = 0; i < ChannelSettings[0]->num_blocks; i++)
                for (j = 0; j < NumChannels; j++)
                    bits += rice_encode_block(ChannelSettings[j]->cchannel->reordered_channel +
                    ChannelSettings[j]->cchannel->blocks[i].offset,
                    ChannelSettings[j]->cchannel->blocks[i].size,
                    ChannelSettings[j]->blocks[i].quant, 0, 1 << 30, BF, NULL);
        }
    }
    return bits;
}


/*标题5版式***************************************************************************
函数输入：	t_bit_file* BF：读入的文件的句柄（文件以读模式打开）
功能描述：	打开通过wv_encode_channels函数写入的比特流文件，从中解码指定数目的信道
函数输出：	函数返回一个包含有所有信道的结构（失败返回NULL）
*************************************************************************************/
WAVELET_DLL_API t_wv_dchannels* WAVELET_DLL_CC wv_init_decode_channels(t_bit_file* BF)
{
    t_wv_dchannels* dc = NULL;

    if (BF)
    {
        int num_blocks, size_min_block;

        dc = (t_wv_dchannels*)malloc(sizeof (t_wv_dchannels));
        dc->owidth = 0;
        //以读模式打开比特流文件，从中读入一比特成功
        if (bit_read(1, BF))
        {
            //读入初始的尺度
            dc->owidth = bit_read(16, BF) + 1;
            dc->oheight = bit_read(16, BF) + 1;
        }
        dc->num_channels = bit_read(4, BF) + 1;
        dc->width = 1 << bit_read(4, BF);
        dc->height = 1 << bit_read(4, BF);
        if (dc->owidth == 0)
        {
            //读入失败，宽高赋值
            dc->owidth = dc->width;
            dc->oheight = dc->height;
        }
        num_blocks = bit_read(4, BF) + 1;
        size_min_block = bit_read(5, BF);
        size_min_block = size_min_block ? (1 << (size_min_block - 1)) : 0;

        if (dc->width >= 1 && dc->height >= 1 && size_min_block > 0)
        {
            int i, j;
            int max_levels = max(0, log2i(min(dc->width, dc->height)) - 2);
            wv_pel* tmp;
            int** reorder_table = init_reorder_tables(dc->width, dc->owidth, dc->height, dc->oheight, max_levels);

            dc->channels = (wv_pel**)malloc(dc->num_channels * sizeof (wv_pel*));
            for (i = 0; i < dc->num_channels; i++)
                dc->channels[i] = (wv_pel*)malloc(dc->width * dc->height * sizeof (wv_pel));
            for (i = 0; i < num_blocks; i++)
            {
                int ofs = (i == 0) ? 0 : (size_min_block << (2 * (i - 1)));
                int size = (i == 0) ? size_min_block : ((size_min_block << (2 * i)) - ofs);

                for (j = 0; j < dc->num_channels; j++)
                    rice_decode_block(dc->channels[j] + ofs, size, 0, BF);
            }

            tmp = (wv_pel*)malloc(dc->width * dc->height * sizeof (wv_pel));

            for (i = 0; i < dc->num_channels; i++)
            {
                unreorder(tmp, dc->channels[i], reorder_table, dc->width, dc->height, max_levels);
                memcpy(dc->channels[i], tmp, dc->width * dc->height * sizeof *dc->channels[i]);
                inverse_wavelet_transform(dc->channels[i], dc->width, dc->height, max_levels);
            }

            free(tmp);
            free_reorder_tables(reorder_table, max_levels);
        }
        else
        {
            free(dc);
            dc = NULL;
        }
    }
    return dc;
}

/*标题5版式*******************************************************************
函数输入:	t_wv_dchannels* dc:	将要释放的解压缩信道结构
功能描述:	示范由wv_init_decode_channels分配的结构,其中wv_init_decode_channels
            表示初始解码信道
*****************************************************************************/
WAVELET_DLL_API void WAVELET_DLL_CC wv_done_decode_channels(t_wv_dchannels* dc)
{
    if (dc)
    {
        int i;
        for (i = 0; i < dc->num_channels; i++)
            free(dc->channels[i]);
        free(dc->channels);
        free(dc);
    }
}

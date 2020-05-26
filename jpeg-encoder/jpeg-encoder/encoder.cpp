#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include "encoder.h"

namespace
{
    //亮度量化转换表
    const unsigned char Luminance_Quantization_Table[64] =
    {
        16,  11,  10,  16,  24,  40,  51,  61,
        12,  12,  14,  19,  26,  58,  60,  55,
        14,  13,  16,  24,  40,  57,  69,  56,
        14,  17,  22,  29,  51,  87,  80,  62,
        18,  22,  37,  56,  68, 109, 103,  77,
        24,  35,  55,  64,  81, 104, 113,  92,
        49,  64,  78,  87, 103, 121, 120, 101,
        72,  92,  95,  98, 112, 100, 103,  99
    };

    //色差值量化转换表
    const unsigned char Chrominance_Quantization_Table[64] =
    {
        17,  18,  24,  47,  99,  99,  99,  99,
        18,  21,  26,  66,  99,  99,  99,  99,
        24,  26,  56,  99,  99,  99,  99,  99,
        47,  66,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99,
        99,  99,  99,  99,  99,  99,  99,  99
    };

    //Z字型变换
    const char ZigZag[64] =
    {
        0, 1, 5, 6,14,15,27,28,
        2, 4, 7,13,16,26,29,42,
        3, 8,12,17,25,30,41,43,
        9,11,18,24,31,40,44,53,
        10,19,23,32,39,45,52,54,
        20,22,33,38,46,51,55,60,
        21,34,37,47,50,56,59,61,
        35,36,48,49,57,58,62,63
    };

    //标准亮度DC值
    const char Standard_DC_Luminance_NRCodes[] = { 0, 0, 7, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
    const unsigned char Standard_DC_Luminance_Values[] = { 4, 5, 3, 2, 6, 1, 0, 7, 8, 9, 10, 11 };

    //标准色差DC值
    const char Standard_DC_Chrominance_NRCodes[] = { 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
    const unsigned char Standard_DC_Chrominance_Values[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

    //标准AC亮度值
    const char Standard_AC_Luminance_NRCodes[] = { 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
    const unsigned char Standard_AC_Luminance_Values[] =
    {
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
        0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
        0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
        0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
        0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
        0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
        0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
        0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
        0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa
    };

    //标准色差AC值
    const char Standard_AC_Chrominance_NRCodes[] = { 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
    const unsigned char Standard_AC_Chrominance_Values[] =
    {
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
        0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
        0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
        0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
        0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
        0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
        0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
        0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
        0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
        0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
        0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa
    };

}

//读取bmp文件
bool JpegEncoder::readBMPFile(const char* fileName)
{
    //清理旧数据
    if (m_rgbBuffer)
        delete[] m_rgbBuffer;
    m_rgbBuffer = 0;

    m_width = 0;
    m_height = 0;

    //BMP 文件格式
#pragma pack(push, 2)
    typedef struct
    {
        unsigned short	bfType;
        unsigned int	bfSize;
        unsigned short	bfReserved1;
        unsigned short	bfReserved2;
        unsigned int	bfOffBits;
    } BITMAPFILEHEADER;

    typedef struct
    {
        unsigned int	biSize;
        int				biWidth;
        int				biHeight;
        unsigned short	biPlanes;
        unsigned short	biBitCount;
        unsigned int	biCompression;
        unsigned int	biSizeImage;
        int				biXPelsPerMeter;
        int				biYPelsPerMeter;
        unsigned int	biClrUsed;
        unsigned int	biClrImportant;
    } BITMAPINFOHEADER;
#pragma pack(pop)

    //打开文件
    FILE* fp = fopen(fileName, "rb");
    if (fp == 0)
        return false;

    bool successed = false;
    do
    {
        BITMAPFILEHEADER fileHeader;
        BITMAPINFOHEADER infoHeader;

        //检测文件头的正确性
        if (1 != fread(&fileHeader, sizeof(fileHeader), 1, fp))
            break;
        if (fileHeader.bfType != 0x4D42)
            break;

        //检测文件格式的正确性
        if (1 != fread(&infoHeader, sizeof(infoHeader), 1, fp))
            break;
        if (infoHeader.biBitCount != 24 || infoHeader.biCompression != 0)
            break;

        //检测文件大小的正确性
        int width = infoHeader.biWidth;
        int height = infoHeader.biHeight < 0 ? (-infoHeader.biHeight) : infoHeader.biHeight;
        if ((width & 7) != 0 || (height & 7) != 0)
            break;	//必须是8的倍数

        int bmpSize = width * height * 3;

        unsigned char* buffer = new unsigned char[bmpSize];
        if (buffer == 0)
            break;

        fseek(fp, fileHeader.bfOffBits, SEEK_SET);

        if (infoHeader.biHeight > 0)
        {
            for (int i = 0; i < height; i++)
            {
                if (width != fread(buffer + (height - 1 - i)*width * 3, 3, width, fp))
                {
                    delete[] buffer; buffer = 0;
                    break;
                }
            }
        }
        else
        {
            if (width*height != fread(buffer, 3, width*height, fp))
            {
                delete[] buffer; buffer = 0;
                break;
            }
        }

        m_rgbBuffer = buffer;
        m_width = width;
        m_height = height;
        successed = true;
    } while (false);

    fclose(fp);
    fp = 0;

    return successed;
}

//编码为jpg文件格式
bool JpegEncoder::encodeToJPG(const char* fileName, int quality_scale)
{
    //尚未读取？
    if (m_rgbBuffer == 0 || m_width == 0 || m_height == 0)
        return false;

    //输出文件
    FILE* fp = fopen(fileName, "wb");
    if (fp == 0)
        return false;

    //初始化量化表
    InitQualityTables(quality_scale);

    //文件头
    Write_jpeg_header(fp);

    short prev_DC_Y = 0, prev_DC_Cb = 0, prev_DC_Cr = 0;
    int newByte = 0, newBytePos = 7;

    for (int yPos = 0; yPos < m_height; yPos += 8)
    {
        for (int xPos = 0; xPos < m_width; xPos += 8)
        {
            char yData[64], cbData[64], crData[64];
            short yQuant[64], cbQuant[64], crQuant[64];

            //转换颜色空间
            ConvertColorSpace(xPos, yPos, yData, cbData, crData);

            BitString outputBitString[128];
            int bitStringCounts;

            //Y通道压缩
            Foword_FDC(yData, yQuant);
            DoHuffmanEncoding(yQuant, prev_DC_Y, m_Y_DC_Huffman_Table, m_Y_AC_Huffman_Table, outputBitString, bitStringCounts);
            Write_bitstring(outputBitString, bitStringCounts, newByte, newBytePos, fp);

            //Cb通道压缩
            Foword_FDC(cbData, cbQuant);
            DoHuffmanEncoding(cbQuant, prev_DC_Cb, m_CbCr_DC_Huffman_Table, m_CbCr_AC_Huffman_Table, outputBitString, bitStringCounts);
            Write_bitstring(outputBitString, bitStringCounts, newByte, newBytePos, fp);

            //Cr通道压缩
            Foword_FDC(crData, crQuant);
            DoHuffmanEncoding(crQuant, prev_DC_Cr, m_CbCr_DC_Huffman_Table, m_CbCr_AC_Huffman_Table, outputBitString, bitStringCounts);
            Write_bitstring(outputBitString, bitStringCounts, newByte, newBytePos, fp);
        }
    }

    Write_word(0xFFD9, fp); //Write End of Image Marker

    fclose(fp);

    return true;
}

//初始化量化表
void JpegEncoder::InitQualityTables(int quality_scale)
{
    if (quality_scale <= 0)
        quality_scale = 1;
    if (quality_scale >= 100)
        quality_scale = 99;

    for (int i = 0; i < 64; i++)
    {
        int temp = ((int)(Luminance_Quantization_Table[i] * quality_scale + 50) / 100);
        if (temp <= 0)
            temp = 1;
        if (temp > 0xFF)
            temp = 0xFF;
        m_YTable[ZigZag[i]] = (unsigned char)temp;

        temp = ((int)(Chrominance_Quantization_Table[i] * quality_scale + 50) / 100);
        if (temp <= 0)
            temp = 1;
        if (temp > 0xFF)
            temp = 0xFF;
        m_CbCrTable[ZigZag[i]] = (unsigned char)temp;
    }
}

//写jpg头文件
void JpegEncoder::Write_jpeg_header(FILE* fp)
{
    //SOI
    Write_word(0xFFD8, fp);		// marker = 0xFFD8

    //APPO
    Write_word(0xFFE0, fp);		// marker = 0xFFE0
    Write_word(16, fp);			// length = 16 for usual JPEG, no thumbnail
    Write("JFIF", 5, fp);			// 'JFIF\0'
    Write_byte(1, fp);			// version_hi
    Write_byte(1, fp);			// version_low
    Write_byte(0, fp);			// xyunits = 0 no units, normal density
    Write_word(1, fp);			// xdensity
    Write_word(1, fp);			// ydensity
    Write_byte(0, fp);			// thumbWidth
    Write_byte(0, fp);			// thumbHeight

    //DQT
    Write_word(0xFFDB, fp);		//marker = 0xFFDB
    Write_word(132, fp);			//size=132
    Write_byte(0, fp);			//QTYinfo== 0:  bit 0..3: number of QT = 0 (table for Y)
                                    //				bit 4..7: precision of QT
                                    //				bit 8	: 0
    Write(m_YTable, 64, fp);		//YTable
    Write_byte(1, fp);			//QTCbinfo = 1 (quantization table for Cb,Cr)
    Write(m_CbCrTable, 64, fp);	//CbCrTable

    //SOFO
    Write_word(0xFFC0, fp);			//marker = 0xFFC0
    Write_word(17, fp);				//length = 17 for a truecolor YCbCr JPG
    Write_byte(8, fp);				//precision = 8: 8 bits/sample
    Write_word(m_height & 0xFFFF, fp);	//height
    Write_word(m_width & 0xFFFF, fp);	//width
    Write_byte(3, fp);				//nrofcomponents = 3: We encode a truecolor JPG

    Write_byte(1, fp);				//IdY = 1
    Write_byte(0x11, fp);				//HVY sampling factors for Y (bit 0-3 vert., 4-7 hor.)(SubSamp 1x1)
    Write_byte(0, fp);				//QTY  Quantization Table number for Y = 0

    Write_byte(2, fp);				//IdCb = 2
    Write_byte(0x11, fp);				//HVCb = 0x11(SubSamp 1x1)
    Write_byte(1, fp);				//QTCb = 1

    Write_byte(3, fp);				//IdCr = 3
    Write_byte(0x11, fp);				//HVCr = 0x11 (SubSamp 1x1)
    Write_byte(1, fp);				//QTCr Normally equal to QTCb = 1

    //DHT
    Write_word(0xFFC4, fp);		//marker = 0xFFC4
    Write_word(0x01A2, fp);		//length = 0x01A2
    Write_byte(0, fp);			//HTYDCinfo bit 0..3	: number of HT (0..3), for Y =0
                                    //			bit 4		: type of HT, 0 = DC table,1 = AC table
                                    //			bit 5..7	: not used, must be 0
    Write(Standard_DC_Luminance_NRCodes, sizeof(Standard_DC_Luminance_NRCodes), fp);	//DC_L_NRC
    Write(Standard_DC_Luminance_Values, sizeof(Standard_DC_Luminance_Values), fp);		//DC_L_VALUE
    Write_byte(0x10, fp);			//HTYACinfo
    Write(Standard_AC_Luminance_NRCodes, sizeof(Standard_AC_Luminance_NRCodes), fp);
    Write(Standard_AC_Luminance_Values, sizeof(Standard_AC_Luminance_Values), fp); //we'll use the standard Huffman tables
    Write_byte(0x01, fp);			//HTCbDCinfo
    Write(Standard_DC_Chrominance_NRCodes, sizeof(Standard_DC_Chrominance_NRCodes), fp);
    Write(Standard_DC_Chrominance_Values, sizeof(Standard_DC_Chrominance_Values), fp);
    Write_byte(0x11, fp);			//HTCbACinfo
    Write(Standard_AC_Chrominance_NRCodes, sizeof(Standard_AC_Chrominance_NRCodes), fp);
    Write(Standard_AC_Chrominance_Values, sizeof(Standard_AC_Chrominance_Values), fp);

    //SOS
    Write_word(0xFFDA, fp);		//marker = 0xFFC4
    Write_word(12, fp);			//length = 12
    Write_byte(3, fp);			//nrofcomponents, Should be 3: truecolor JPG

    Write_byte(1, fp);			//Idy=1
    Write_byte(0, fp);			//HTY	bits 0..3: AC table (0..3)
                                    //		bits 4..7: DC table (0..3)
    Write_byte(2, fp);			//IdCb
    Write_byte(0x11, fp);			//HTCb

    Write_byte(3, fp);			//IdCr
    Write_byte(0x11, fp);			//HTCr

    Write_byte(0, fp);			//Ss not interesting, they should be 0,63,0
    Write_byte(0x3F, fp);			//Se
    Write_byte(0, fp);			//Bf
}

//写字节
void JpegEncoder::Write_byte(unsigned char value, FILE* fp)
{
    Write(&value, 1, fp);
}

//写字
void JpegEncoder::Write_word(unsigned short value, FILE* fp)
{
    unsigned short _value = ((value >> 8) & 0xFF) | ((value & 0xFF) << 8);
    Write(&_value, 2, fp);
}

//写文件
void JpegEncoder::Write(const void* p, int byteSize, FILE* fp)
{
    fwrite(p, 1, byteSize, fp);
}

//颜色空间转换
void JpegEncoder::ConvertColorSpace(int xPos, int yPos, char* yData, char* cbData, char* crData)
{
    for (int y = 0; y < 8; y++)
    {
        unsigned char* p = m_rgbBuffer + (y + yPos)*m_width * 3 + xPos * 3;
        for (int x = 0; x < 8; x++)
        {
            unsigned char B = *p++;
            unsigned char G = *p++;
            unsigned char R = *p++;

            yData[y * 8 + x] = (char)(0.299f * R + 0.587f * G + 0.114f * B - 128);
            cbData[y * 8 + x] = (char)(-0.1687f * R - 0.3313f * G + 0.5f * B);
            crData[y * 8 + x] = (char)(0.5f * R - 0.4187f * G - 0.0813f * B);
        }
    }
}

//正向DCT变换
void JpegEncoder::Foword_FDC(const char* channel_data, short* fdc_data)
{
    const float PI = 3.1415926f;
    for (int v = 0; v < 8; v++)
    {
        for (int u = 0; u < 8; u++)
        {
            float alpha_u = (u == 0) ? 1 / sqrt(8.0f) : 0.5f;
            float alpha_v = (v == 0) ? 1 / sqrt(8.0f) : 0.5f;

            float temp = 0.f;
            for (int x = 0; x < 8; x++)
            {
                for (int y = 0; y < 8; y++)
                {
                    float data = channel_data[y * 8 + x];

                    data *= cos((2 * x + 1)*u*PI / 16.0f);
                    data *= cos((2 * y + 1)*v*PI / 16.0f);

                    temp += data;
                }
            }

            temp *= alpha_u * alpha_v / m_YTable[ZigZag[v * 8 + u]];
            fdc_data[ZigZag[v * 8 + u]] = (short)((short)(temp + 16384.5) - 16384);
        }
    }
}

//进行Huffman编码
void JpegEncoder::DoHuffmanEncoding(const short* DU, short& prevDC, const BitString* HTDC, const BitString* HTAC,
    BitString* outputBitString, int& bitStringCounts)
{
    BitString EOB = HTAC[0x00];
    BitString SIXTEEN_ZEROS = HTAC[0xF0];

    int index = 0;

    // encode DC
    int dcDiff = (int)(DU[0] - prevDC);
    prevDC = DU[0];

    if (dcDiff == 0)
        outputBitString[index++] = HTDC[0];
    else
    {
        BitString bs = GetBitCode(dcDiff);

        outputBitString[index++] = HTDC[bs.length];
        outputBitString[index++] = bs;
    }

    // encode ACs
    int endPos = 63; //end0pos = first element in reverse order != 0
    while ((endPos > 0) && (DU[endPos] == 0))
        endPos--;

    for (int i = 1; i <= endPos; )
    {
        int startPos = i;
        while ((DU[i] == 0) && (i <= endPos))
            i++;

        int zeroCounts = i - startPos;
        if (zeroCounts >= 16)
        {
            for (int j = 1; j <= zeroCounts / 16; j++)
                outputBitString[index++] = SIXTEEN_ZEROS;
            zeroCounts = zeroCounts % 16;
        }

        BitString bs = GetBitCode(DU[i]);

        outputBitString[index++] = HTAC[(zeroCounts << 4) | bs.length];
        outputBitString[index++] = bs;
        i++;
    }

    if (endPos != 63)
        outputBitString[index++] = EOB;

    bitStringCounts = index;
}

//得到bit值
JpegEncoder::BitString JpegEncoder::GetBitCode(int value)
{
    BitString ret;
    int v = (value > 0) ? value : -value;

    //bit 的长度
    int length = 0;
    for (length = 0; v; v >>= 1)
        length++;

    ret.value = value > 0 ? value : ((1 << length) + value - 1);
    ret.length = length;

    return ret;
};

//写bit字符
void JpegEncoder::Write_bitstring(const BitString* bs, int counts, int& newByte, int& newBytePos, FILE* fp)
{
    unsigned short mask[] = { 1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768 };

    for (int i = 0; i < counts; i++)
    {
        int value = bs[i].value;
        int posval = bs[i].length - 1;

        while (posval >= 0)
        {
            if ((value & mask[posval]) != 0)
            {
                newByte = newByte | mask[newBytePos];
            }
            posval--;
            newBytePos--;
            if (newBytePos < 0)
            {
                // Write to stream
                Write_byte((unsigned char)(newByte), fp);
                if (newByte == 0xFF)
                {
                    // Handle special case
                    Write_byte((unsigned char)(0x00), fp);
                }

                // Reinitialize
                newBytePos = 7;
                newByte = 0;
            }
        }
    }
}

//Jpeg编码器构造函数
JpegEncoder::JpegEncoder()
    : m_width(0)
    , m_height(0)
    , m_rgbBuffer(0)
{
    //初始化静态表格
    InitDCLuminanceTable();
    InitACLuminanceTable();
    InitDCChrominanceTable();
    InitACChrominanceTable();
}

void JpegEncoder::InitDCLuminanceTable()
{
    memset(&m_Y_DC_Huffman_Table, 0, sizeof(m_Y_DC_Huffman_Table));
    unsigned char pos_in_table = 0;
    unsigned short code_value = 0;

    for (int k = 1; k <= 16; k++)
    {
        for (int j = 1; j <= Standard_DC_Luminance_NRCodes[k - 1]; j++)
        {
            m_Y_DC_Huffman_Table[Standard_DC_Luminance_Values[pos_in_table]].value = code_value;
            m_Y_DC_Huffman_Table[Standard_DC_Luminance_Values[pos_in_table]].length = k;
            pos_in_table++;
            code_value++;
        }
        code_value <<= 1;
    }
}

void JpegEncoder::InitACLuminanceTable()
{
    memset(&m_Y_AC_Huffman_Table, 0, sizeof(m_Y_AC_Huffman_Table));
    unsigned char pos_in_table = 0;
    unsigned short code_value = 0;

    for (int k = 1; k <= 16; k++)
    {
        for (int j = 1; j <= Standard_AC_Luminance_NRCodes[k - 1]; j++)
        {
            m_Y_AC_Huffman_Table[Standard_AC_Luminance_Values[pos_in_table]].value = code_value;
            m_Y_AC_Huffman_Table[Standard_AC_Luminance_Values[pos_in_table]].length = k;
            pos_in_table++;
            code_value++;
        }
        code_value <<= 1;
    }
}

void JpegEncoder::InitDCChrominanceTable()
{
    memset(&m_CbCr_DC_Huffman_Table, 0, sizeof(m_CbCr_DC_Huffman_Table));
    unsigned char pos_in_table = 0;
    unsigned short code_value = 0;

    for (int k = 1; k <= 16; k++)
    {
        for (int j = 1; j <= Standard_DC_Chrominance_NRCodes[k - 1]; j++)
        {
            m_CbCr_DC_Huffman_Table[Standard_DC_Chrominance_Values[pos_in_table]].value = code_value;
            m_CbCr_DC_Huffman_Table[Standard_DC_Chrominance_Values[pos_in_table]].length = k;
            pos_in_table++;
            code_value++;
        }
        code_value <<= 1;
    }
}

void JpegEncoder::InitACChrominanceTable()
{
    memset(&m_CbCr_AC_Huffman_Table, 0, sizeof(m_CbCr_AC_Huffman_Table));
    unsigned char pos_in_table = 0;
    unsigned short code_value = 0;

    for (int k = 1; k <= 16; k++)
    {
        for (int j = 1; j <= Standard_AC_Chrominance_NRCodes[k - 1]; j++)
        {
            m_CbCr_AC_Huffman_Table[Standard_AC_Chrominance_Values[pos_in_table]].value = code_value;
            m_CbCr_AC_Huffman_Table[Standard_AC_Chrominance_Values[pos_in_table]].length = k;
            pos_in_table++;
            code_value++;
        }
        code_value <<= 1;
    }
}

//Jpeg编码器析构函数
JpegEncoder::~JpegEncoder()
{
    if (m_rgbBuffer)
        delete[] m_rgbBuffer;
    m_rgbBuffer = 0;

    m_width = 0;
    m_height = 0;
}

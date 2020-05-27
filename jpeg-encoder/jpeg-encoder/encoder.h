#ifndef __JPEG_ENCODER_HEADER__
#define __JPEG_ENCODER_HEADER__

#include <stdio.h>

class JpegEncoder
{
public:
    bool readBMPFile(const char* fileName);

    // 压缩到jpg文件中，quality_scale表示质量，取值范围(0,100), 数字越大压缩比例越高
    bool encodeToJPG(const char* fileName, int quality_scale);

private:
    int				m_width;
    int				m_height;
    unsigned char*	m_rgbBuffer;

    unsigned char	m_YTable[64];
    unsigned char	m_CbCrTable[64];

    struct BitString
    {
        int length;
        int value;
    };

    BitString m_Y_DC_Huffman_Table[12];//亮度DC系数Huffman表
    BitString m_Y_AC_Huffman_Table[256];//亮度AC系数Huffman表

    BitString m_CbCr_DC_Huffman_Table[12];//色差值DC系数Huffman表
    BitString m_CbCr_AC_Huffman_Table[256];//色差值AC系数Huffman表

private:
    void InitDCLuminanceTable();
    void InitACLuminanceTable();
    void InitDCChrominanceTable();
    void InitACChrominanceTable();
    void InitQualityTables(int quality);//初始化量化表

    BitString GetBitCode(int value);
    void ConvertColorSpace(int xPos, int yPos, char* yData, char* cbData, char* crData);//转换颜色空间
    void Foword_FDC(const char* channel_data, short* fdc_data);//正向DCT变换及数据量化
    void DoHuffmanEncoding(const short* DU, short& prevDC, const BitString* HTDC, const BitString* HTAC,
        BitString* outputBitString, int& bitStringCounts);//Huffman编码

private:
    void Write_jpeg_header(FILE* fp);
    void Write_byte(unsigned char value, FILE* fp);
    void Write_word(unsigned short value, FILE* fp);
    void Write_bitstring(const BitString* bs, int counts, int& newByte, int& newBytePos, FILE* fp);
    void Write(const void* p, int byteSize, FILE* fp);

public:
    JpegEncoder();
    ~JpegEncoder();
};

#endif

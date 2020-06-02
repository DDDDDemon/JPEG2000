#ifndef __DECODER_H__
#define __DECODER_H__

#include <stdio.h>
#include <utility>
#include <map>
#include <math.h>
#include "qdbmp.h"
#include <stdlib.h>
#define M_PI 3.1415926
using namespace std;

const int SOI_MARKER = 0xD8;
const int APP0_MARKER = 0xE0;
const int DQT_MARKER = 0xDB;
const int SOF_MARKER = 0xC0;
const int DHT_MARKER = 0xC4;
const int SOS_MARKER = 0xDA;
const int EOI_MARKER = 0xD9;
const int COM_MARKER = 0xFE;

struct RGB {
    unsigned char R, G, B;
};

typedef double BLOCK[8][8];

const int DC = 0;
const int AC = 1;

struct acCode {
    unsigned char len;
    unsigned char zeros;
    int value;
};

class MCU {
public:
    BLOCK mcu[4][2][2];
    // 除錯用
    void show();
    void quantify();
    void zigzag();
    void idct();
    void decode();
    RGB **toRGB();
private:
    double cc(int i, int j);
    double c(int i);
    unsigned char chomp(double x);
    double trans(int id, int h, int w);
};

class decoder
{
public:
    decoder();
    void init_cos_cache();
    void readStream(FILE *f);
private:
    // 读取Section使用辅助函数
    void showSectionName(const char *s);
    unsigned int readSectionLength(FILE *f);
    unsigned int EnterNewSection(FILE *f, const char *s);
    // 读取Section的函数
    void readCOM(FILE *f);
    void readAPP(FILE *f);
    void readDQT(FILE *f);
    void readSOF(FILE *f);
    std::pair<unsigned char, unsigned int>* createHuffCode(unsigned char *a, unsigned int number);
    void readDHT(FILE *f);
    void readSOS(FILE *f);
    bool getBit(FILE *f);
    unsigned char matchHuff(FILE *f, unsigned char number, unsigned char ACorDC);
    int readDC(FILE *f, unsigned char number);
    acCode readAC(FILE *f, unsigned char number);//计算ZRL
    MCU readMCU(FILE *f);
    void readData(FILE *f);
};

#endif // __DECODER_H__

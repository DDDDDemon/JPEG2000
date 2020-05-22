#ifndef __DECODER_H__
#define __DECODER_H__
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <map>
#include <math.h>
#include "qdbmp.h"


const int SOI_MARKER = 0xD8;
const int APP0_MARKER = 0xE0;
const int DQT_MARKER = 0xDB;
const int SOF_MARKER = 0xC0;
const int DHT_MARKER = 0xC4;
const int SOS_MARKER = 0xDA;
const int EOI_MARKER = 0xD9;
const int COM_MARKER = 0xFE;

struct {
    int height;
    int width;
} image;

struct {
    unsigned char id;
    unsigned char width;
    unsigned char height;
    unsigned char quant;
} subVector[4];



struct acCode {
    unsigned char len;
    unsigned char zeros;
    int value;
};

struct RGB {
    unsigned char R, G, B;
};

typedef double BLOCK[8][8];


const int DC = 0;
const int AC = 1;

void init_cos_cache();

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


// 讀取 Section 用之輔助函式

void showSectionName(const char *s);

unsigned int readSectionLength(FILE *f);

unsigned int EnterNewSection(FILE *f, const char *s);

// 讀取各 Section 之函式

void readCOM(FILE *f);
void readAPP(FILE *f);
void readDQT(FILE *f);
void readSOF(FILE *f);

std::pair<unsigned char, unsigned int>* createHuffCode(unsigned char *a, unsigned int number);
void readDHT(FILE *f);
void readSOS(FILE *f);

// 必須連續呼叫getBit，中間被fread斷掉就會出問題
bool getBit(FILE *f);

unsigned char matchHuff(FILE *f, unsigned char number, unsigned char ACorDC);

int readDC(FILE *f, unsigned char number);

// 計算ZRL
acCode readAC(FILE *f, unsigned char number);

MCU readMCU(FILE *f);

void readData(FILE *f);

void readStream(FILE *f);


#endif // DECODER_H

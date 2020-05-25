#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "encoder.h"
#include <utility>
#include <map>
#include <math.h>
#include "qdbmp.h"
#include <stdlib.h>
#include <string.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    JpegEncoder e;
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void on_pushButton_OpenFile_clicked();
    void on_pushButton_OpenFile2_clicked();

private:
    Ui::Widget *ui;
    //void init_cos_cache();
    void showSectionName(const char *s);
    unsigned int readSectionLength(FILE *f);
    unsigned int EnterNewSection(FILE *f, const char *s);
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
    void readData(FILE *f);
    //void readStream(FILE *f);
};

#endif // WIDGET_H

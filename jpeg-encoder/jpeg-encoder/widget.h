#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "encoder.h"
#include "decoder.h"
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
    decoder d;
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void on_pushButton_OpenFile_clicked();
    void on_pushButton_OpenFile2_clicked();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H

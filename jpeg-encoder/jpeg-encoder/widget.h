#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "encoder.h"

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

private slots:
    void on_pushButton_OpenFile_2_clicked();

private:
    Ui::Widget *ui;
};

#endif // WIDGET_H

#include "widget.h"
#include "ui_widget.h"
#include "encoder.h"
#include <QFileDialog>
#include <QMessageBox>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "widget.h"
#include <QDebug>
#define printf qDebug
using namespace std;


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}


void Widget::on_pushButton_OpenFile_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("选择图片"), ".", tr("Image Files(*.bmp)"));
    QImage image;
    if(path!="")
    {
        if(image.load(path))
        {
            ui->Image_label->setPixmap(QPixmap::fromImage(image).scaled(ui->Image_label->size()));
        }
    }
    string str=path.toStdString();
    const char* s=str.c_str();
    if(!e.readBMPFile(s))
    {
        QMessageBox::information(nullptr, "警告", "打开图片失败", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
    int quality=ui->spinBox_Quality_scale->text().toInt();
    if(!e.encodeToJPG("out.jpg",quality))
    {
        QMessageBox::information(nullptr, "警告", "图片转换失败", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
}

void Widget::on_pushButton_OpenFile2_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("选择图片"), ".", tr("Image Files(*.jpg)"));
    QImage image;
    if(path!="")
    {
        if(image.load(path))
        {
            ui->Image_label->setPixmap(QPixmap::fromImage(image).scaled(ui->Image_label->size()));
        }
    }
    string str=path.toStdString();
    const char* s=str.c_str();
    FILE *f = fopen(s, "rb");
    if (f == NULL) {
        fprintf(stderr, "图片打开失败\n");
    }
    d.init_cos_cache();
    d.readStream(f);
}

Widget::~Widget()
{
    delete ui;
}

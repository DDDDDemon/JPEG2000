#include "widget.h"
#include "ui_widget.h"
#include "encoder.h"
#include <QFileDialog>
#include <QMessageBox>
#include <stdio.h>
#include <string.h>
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
    string str=path.toStdString();
    const char* s=str.c_str();
    if(!e.readFromBMP(s))
    {
        QMessageBox::information(NULL, "警告", "打开图片失败", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
    int quality=ui->spinBox_Quality_scale->text().toInt();
    if(!e.encodeToJPG("out.jpg",quality))
    {
        QMessageBox::information(NULL, "警告", "图片转换失败", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    }
}

Widget::~Widget()
{
    delete ui;
}

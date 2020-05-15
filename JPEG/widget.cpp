#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStringListModel>
#include <QPushButton>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);// 禁止最大化按钮
    setFixedSize(this->width(),this->height());// 禁止拖动窗口大小
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_LoadButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("选择图片"), ".", tr("Image Files(*.jpg *.png)"));
    QImage* img=new QImage;
    QImage* scaledimg=new QImage;//分别保存原图和缩放之后的图片
    if(! ( img->load(path) ) ) //加载图像
    {
        QMessageBox::information(this,tr("打开图像失败"),tr("打开图像失败!"));
        delete img;
        return;
    }
    QStringList imageDirectory;
    imageDirectory<<path;
    QStringListModel *model=new QStringListModel(imageDirectory);
    ui->DirectoryListView->setModel(model);
}

void Widget::on_ClearButton_clicked()
{
    ui->DirectoryListView->setModel(nullptr);
}

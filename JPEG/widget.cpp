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

void Widget::wv_progress(int Current, int Maximum)
{
    ProgressRecord *pr;
    int ofs;
    ofs=(pr->CurChannel*ui->PSNRHorizontalSlider->maximum())/pr->MaxChannel;
    ui->PSNRHorizontalSlider->setValue(ofs+(Current*ui->PSNRHorizontalSlider->maximum()/(pr->MaxChannel*Maximum)));
    update();
}

void Widget::LoadSourceImage(QString FileName, bool GreyScale, bool YCbCr)
{
    QString channel_names[2][wv_MAX_CHANNELS]={{"R", "G", "B", "A", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"},
                                               {"Y", "Cb", "Cr", "A", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"}};
    QImage temp;
    int i;
    wv_pel y,cb,cr;
    t_bit_file bf;
    bool is_ycbcr;
    t_wv_dchannels dc;
    int old_num_channels;
    int max_bits;

}

void Widget::WidgetInit()
{
    int i;
    QImage tf;
    for(i=0;i<wv_MAX_CHANNELS;i++)
    {
        Channels[i].Data=NULL;
        Channels[i].CData=NULL;
        Channels[i].Channel=nullptr;
    }
    ImageWidth=0;
    ImageHeight=0;
    NumChannels=0;
    NumBlocks=0;
    ReorderTable=NULL;

}

void Widget::on_LoadButton_clicked()
{
    if(((NumChannels+1<=wv_MAX_CHANNELS)&&ui->GreyscaleCheckBox->checkState()==true)||((NumChannels+3<=wv_MAX_CHANNELS)&&(!(ui->GreyscaleCheckBox->checkState()==true))))
    {
        QString path = QFileDialog::getOpenFileName(this, tr("选择图片"), ".", tr("Image Files(*.jpg *.png)"));
    }
            /*
    QImage* img=new QImage;
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
            */
}

void Widget::on_ClearButton_clicked()
{
    ui->DirectoryListView->setModel(nullptr);
}

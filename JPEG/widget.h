#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QSlider>
#include "mchannel.h"
#include "wavelet.h"

typedef struct ProgressRecord
{
    int CurChannel;
    int MaxChannel;
};

typedef struct TChannel
{
    wv_pel Data,CData;
    bool DidYCbCr,IsGreyScale;
    t_wv_cchannel Channel;
    float max_mse,delta_mse;
    float out_mse;
};

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public slots:
    void on_LoadButton_clicked();
    void on_ClearButton_clicked();

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;

    QImage FormImage,FormCImage;
    int ImageWidth,ImageHeight,NWidth,NHeight;
    int ScrollBarX,ScrollBarY;
    int NumChannels;
    TChannel Channels[wv_MAX_CHANNELS];
    void *ReorderTable;
    int NumBlocks;
    int MaxSizeTrack;

    void wv_progress(int Current,int Maximum);
    void LoadSourceImage(QString FileName,bool GreyScale,bool YCbCr);
    void WidgetInit();
};

#endif // WIDGET_H

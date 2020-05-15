#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QSlider>

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
};

#endif // WIDGET_H

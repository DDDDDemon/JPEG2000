/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *pushButton_OpenFile;
    QSpinBox *spinBox_Quality_scale;
    QLabel *label;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(400, 300);
        pushButton_OpenFile = new QPushButton(Widget);
        pushButton_OpenFile->setObjectName(QString::fromUtf8("pushButton_OpenFile"));
        pushButton_OpenFile->setGeometry(QRect(50, 230, 75, 25));
        spinBox_Quality_scale = new QSpinBox(Widget);
        spinBox_Quality_scale->setObjectName(QString::fromUtf8("spinBox_Quality_scale"));
        spinBox_Quality_scale->setGeometry(QRect(250, 230, 75, 25));
        spinBox_Quality_scale->setMinimum(1);
        label = new QLabel(Widget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(170, 230, 54, 25));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", nullptr));
        pushButton_OpenFile->setText(QApplication::translate("Widget", "\346\211\223\345\274\200\346\226\207\344\273\266", nullptr));
        label->setText(QApplication::translate("Widget", "\345\216\213\347\274\251\346\257\224\357\274\232", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H

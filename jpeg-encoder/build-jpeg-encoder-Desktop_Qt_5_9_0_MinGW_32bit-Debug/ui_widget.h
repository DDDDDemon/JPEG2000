/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
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
    QPushButton *pushButton_OpenFile_2;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(400, 300);
        pushButton_OpenFile = new QPushButton(Widget);
        pushButton_OpenFile->setObjectName(QStringLiteral("pushButton_OpenFile"));
        pushButton_OpenFile->setGeometry(QRect(50, 230, 75, 25));
        spinBox_Quality_scale = new QSpinBox(Widget);
        spinBox_Quality_scale->setObjectName(QStringLiteral("spinBox_Quality_scale"));
        spinBox_Quality_scale->setGeometry(QRect(250, 230, 75, 25));
        spinBox_Quality_scale->setMinimum(1);
        label = new QLabel(Widget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(170, 230, 54, 25));
        pushButton_OpenFile_2 = new QPushButton(Widget);
        pushButton_OpenFile_2->setObjectName(QStringLiteral("pushButton_OpenFile_2"));
        pushButton_OpenFile_2->setGeometry(QRect(50, 180, 75, 25));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", Q_NULLPTR));
        pushButton_OpenFile->setText(QApplication::translate("Widget", "\346\211\223\345\274\200\346\226\207\344\273\266", Q_NULLPTR));
        label->setText(QApplication::translate("Widget", "\345\216\213\347\274\251\346\257\224\357\274\232", Q_NULLPTR));
        pushButton_OpenFile_2->setText(QApplication::translate("Widget", "\346\211\223\345\274\200\346\226\207\344\273\266", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H

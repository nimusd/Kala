/********************************************************************************
** Form generated from reading UI file 'container.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Container
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *containerFrameHeader;
    QHBoxLayout *horizontalLayout;
    QLabel *labelInstanceName;
    QFrame *frameBody;
    QHBoxLayout *horizontalLayout_2;
    QFrame *frameInputs;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QLabel *label_2;
    QFrame *frameOutputs;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_3;

    void setupUi(QWidget *Container)
    {
        if (Container->objectName().isEmpty())
            Container->setObjectName("Container");
        Container->resize(247, 213);
        Container->setStyleSheet(QString::fromUtf8("QWidget {\n"
"    border-radius: 8px;\n"
"    background-color: #f8f9fa;\n"
"}"));
        verticalLayout = new QVBoxLayout(Container);
        verticalLayout->setObjectName("verticalLayout");
        containerFrameHeader = new QFrame(Container);
        containerFrameHeader->setObjectName("containerFrameHeader");
        containerFrameHeader->setMinimumSize(QSize(0, 30));
        containerFrameHeader->setMaximumSize(QSize(16777215, 30));
        containerFrameHeader->setStyleSheet(QString::fromUtf8("background-color: #3498db; \n"
"color: white; \n"
"border-top-left-radius: 8px; \n"
"border-top-right-radius: 8px;"));
        containerFrameHeader->setFrameShape(QFrame::Shape::StyledPanel);
        containerFrameHeader->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout = new QHBoxLayout(containerFrameHeader);
        horizontalLayout->setObjectName("horizontalLayout");
        labelInstanceName = new QLabel(containerFrameHeader);
        labelInstanceName->setObjectName("labelInstanceName");

        horizontalLayout->addWidget(labelInstanceName);


        verticalLayout->addWidget(containerFrameHeader);

        frameBody = new QFrame(Container);
        frameBody->setObjectName("frameBody");
        frameBody->setFrameShape(QFrame::Shape::StyledPanel);
        frameBody->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_2 = new QHBoxLayout(frameBody);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        frameInputs = new QFrame(frameBody);
        frameInputs->setObjectName("frameInputs");
        frameInputs->setFrameShape(QFrame::Shape::StyledPanel);
        frameInputs->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_2 = new QVBoxLayout(frameInputs);
        verticalLayout_2->setObjectName("verticalLayout_2");
        label = new QLabel(frameInputs);
        label->setObjectName("label");

        verticalLayout_2->addWidget(label);

        label_2 = new QLabel(frameInputs);
        label_2->setObjectName("label_2");

        verticalLayout_2->addWidget(label_2);


        horizontalLayout_2->addWidget(frameInputs);

        frameOutputs = new QFrame(frameBody);
        frameOutputs->setObjectName("frameOutputs");
        frameOutputs->setFrameShape(QFrame::Shape::StyledPanel);
        frameOutputs->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_3 = new QVBoxLayout(frameOutputs);
        verticalLayout_3->setObjectName("verticalLayout_3");
        label_3 = new QLabel(frameOutputs);
        label_3->setObjectName("label_3");
        label_3->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);

        verticalLayout_3->addWidget(label_3);


        horizontalLayout_2->addWidget(frameOutputs);


        verticalLayout->addWidget(frameBody);


        retranslateUi(Container);

        QMetaObject::connectSlotsByName(Container);
    } // setupUi

    void retranslateUi(QWidget *Container)
    {
        Container->setWindowTitle(QCoreApplication::translate("Container", "Form", nullptr));
        labelInstanceName->setText(QCoreApplication::translate("Container", "Container 1", nullptr));
        label->setText(QCoreApplication::translate("Container", "pitch", nullptr));
        label_2->setText(QCoreApplication::translate("Container", "numharm", nullptr));
        label_3->setText(QCoreApplication::translate("Container", "spectrum", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Container: public Ui_Container {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTAINER_H

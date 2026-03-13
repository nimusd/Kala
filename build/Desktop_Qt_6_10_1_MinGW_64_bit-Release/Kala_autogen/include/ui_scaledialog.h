/********************************************************************************
** Form generated from reading UI file 'scaledialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCALEDIALOG_H
#define UI_SCALEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ScaleDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxScale;
    QFormLayout *formLayout;
    QLabel *labelScale;
    QComboBox *comboScale;
    QLabel *labelBaseFreq;
    QDoubleSpinBox *spinBaseFreq;
    QLabel *labelInfo;
    QTextBrowser *textScaleInfo;
    QPushButton *btnDelete;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ScaleDialog)
    {
        if (ScaleDialog->objectName().isEmpty())
            ScaleDialog->setObjectName("ScaleDialog");
        ScaleDialog->resize(500, 440);
        verticalLayout = new QVBoxLayout(ScaleDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupBoxScale = new QGroupBox(ScaleDialog);
        groupBoxScale->setObjectName("groupBoxScale");
        formLayout = new QFormLayout(groupBoxScale);
        formLayout->setObjectName("formLayout");
        labelScale = new QLabel(groupBoxScale);
        labelScale->setObjectName("labelScale");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelScale);

        comboScale = new QComboBox(groupBoxScale);
        comboScale->setObjectName("comboScale");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboScale->sizePolicy().hasHeightForWidth());
        comboScale->setSizePolicy(sizePolicy);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, comboScale);

        labelBaseFreq = new QLabel(groupBoxScale);
        labelBaseFreq->setObjectName("labelBaseFreq");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelBaseFreq);

        spinBaseFreq = new QDoubleSpinBox(groupBoxScale);
        spinBaseFreq->setObjectName("spinBaseFreq");
        spinBaseFreq->setDecimals(2);
        spinBaseFreq->setMinimum(10.000000000000000);
        spinBaseFreq->setMaximum(1000.000000000000000);
        spinBaseFreq->setValue(25.000000000000000);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, spinBaseFreq);


        verticalLayout->addWidget(groupBoxScale);

        labelInfo = new QLabel(ScaleDialog);
        labelInfo->setObjectName("labelInfo");

        verticalLayout->addWidget(labelInfo);

        textScaleInfo = new QTextBrowser(ScaleDialog);
        textScaleInfo->setObjectName("textScaleInfo");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(textScaleInfo->sizePolicy().hasHeightForWidth());
        textScaleInfo->setSizePolicy(sizePolicy1);
        textScaleInfo->setMinimumSize(QSize(0, 160));
        textScaleInfo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        textScaleInfo->setOpenExternalLinks(false);

        verticalLayout->addWidget(textScaleInfo);

        btnDelete = new QPushButton(ScaleDialog);
        btnDelete->setObjectName("btnDelete");

        verticalLayout->addWidget(btnDelete);

        buttonBox = new QDialogButtonBox(ScaleDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ScaleDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, ScaleDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ScaleDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ScaleDialog);
    } // setupUi

    void retranslateUi(QDialog *ScaleDialog)
    {
        ScaleDialog->setWindowTitle(QCoreApplication::translate("ScaleDialog", "Scale Settings", nullptr));
        groupBoxScale->setTitle(QCoreApplication::translate("ScaleDialog", "Select Scale", nullptr));
        labelScale->setText(QCoreApplication::translate("ScaleDialog", "Scale:", nullptr));
        labelBaseFreq->setText(QCoreApplication::translate("ScaleDialog", "Base Frequency:", nullptr));
        spinBaseFreq->setSuffix(QCoreApplication::translate("ScaleDialog", " Hz", nullptr));
        labelInfo->setText(QCoreApplication::translate("ScaleDialog", "Scale Information:", nullptr));
        btnDelete->setText(QCoreApplication::translate("ScaleDialog", "Delete Modulation", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ScaleDialog: public Ui_ScaleDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCALEDIALOG_H

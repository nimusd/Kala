/********************************************************************************
** Form generated from reading UI file 'tempodialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEMPODIALOG_H
#define UI_TEMPODIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TempoDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxTempo;
    QFormLayout *formLayout;
    QLabel *labelTempo;
    QSpinBox *spinTempo;
    QLabel *labelTimeSig;
    QHBoxLayout *horizontalLayoutTimeSig;
    QSpinBox *spinTimeSigNum;
    QLabel *labelSlash;
    QSpinBox *spinTimeSigDenom;
    QSpacerItem *horizontalSpacer;
    QCheckBox *checkGradual;
    QPushButton *btnDelete;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *TempoDialog)
    {
        if (TempoDialog->objectName().isEmpty())
            TempoDialog->setObjectName("TempoDialog");
        TempoDialog->resize(350, 220);
        verticalLayout = new QVBoxLayout(TempoDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupBoxTempo = new QGroupBox(TempoDialog);
        groupBoxTempo->setObjectName("groupBoxTempo");
        formLayout = new QFormLayout(groupBoxTempo);
        formLayout->setObjectName("formLayout");
        labelTempo = new QLabel(groupBoxTempo);
        labelTempo->setObjectName("labelTempo");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelTempo);

        spinTempo = new QSpinBox(groupBoxTempo);
        spinTempo->setObjectName("spinTempo");
        spinTempo->setMinimum(20);
        spinTempo->setMaximum(300);
        spinTempo->setValue(120);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, spinTempo);

        labelTimeSig = new QLabel(groupBoxTempo);
        labelTimeSig->setObjectName("labelTimeSig");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelTimeSig);

        horizontalLayoutTimeSig = new QHBoxLayout();
        horizontalLayoutTimeSig->setObjectName("horizontalLayoutTimeSig");
        spinTimeSigNum = new QSpinBox(groupBoxTempo);
        spinTimeSigNum->setObjectName("spinTimeSigNum");
        spinTimeSigNum->setMinimum(1);
        spinTimeSigNum->setMaximum(99);
        spinTimeSigNum->setValue(4);

        horizontalLayoutTimeSig->addWidget(spinTimeSigNum);

        labelSlash = new QLabel(groupBoxTempo);
        labelSlash->setObjectName("labelSlash");
        labelSlash->setAlignment(Qt::AlignCenter);

        horizontalLayoutTimeSig->addWidget(labelSlash);

        spinTimeSigDenom = new QSpinBox(groupBoxTempo);
        spinTimeSigDenom->setObjectName("spinTimeSigDenom");
        spinTimeSigDenom->setMinimum(0);
        spinTimeSigDenom->setMaximum(99);
        spinTimeSigDenom->setValue(4);

        horizontalLayoutTimeSig->addWidget(spinTimeSigDenom);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayoutTimeSig->addItem(horizontalSpacer);


        formLayout->setLayout(1, QFormLayout::ItemRole::FieldRole, horizontalLayoutTimeSig);

        checkGradual = new QCheckBox(groupBoxTempo);
        checkGradual->setObjectName("checkGradual");

        formLayout->setWidget(2, QFormLayout::ItemRole::SpanningRole, checkGradual);


        verticalLayout->addWidget(groupBoxTempo);

        btnDelete = new QPushButton(TempoDialog);
        btnDelete->setObjectName("btnDelete");

        verticalLayout->addWidget(btnDelete);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(TempoDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(TempoDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, TempoDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, TempoDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(TempoDialog);
    } // setupUi

    void retranslateUi(QDialog *TempoDialog)
    {
        TempoDialog->setWindowTitle(QCoreApplication::translate("TempoDialog", "Tempo Settings", nullptr));
        groupBoxTempo->setTitle(QCoreApplication::translate("TempoDialog", "Tempo & Time Signature", nullptr));
        labelTempo->setText(QCoreApplication::translate("TempoDialog", "Tempo:", nullptr));
        spinTempo->setSuffix(QCoreApplication::translate("TempoDialog", " BPM", nullptr));
        labelTimeSig->setText(QCoreApplication::translate("TempoDialog", "Time Signature:", nullptr));
        labelSlash->setText(QCoreApplication::translate("TempoDialog", "/", nullptr));
        checkGradual->setText(QCoreApplication::translate("TempoDialog", "Gradual transition to next marker", nullptr));
        btnDelete->setText(QCoreApplication::translate("TempoDialog", "Delete Marker", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TempoDialog: public Ui_TempoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEMPODIALOG_H

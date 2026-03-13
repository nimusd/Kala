/********************************************************************************
** Form generated from reading UI file 'tempochangedialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEMPOCHANGEDIALOG_H
#define UI_TEMPOCHANGEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TempoChangeDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxTiming;
    QFormLayout *formLayout;
    QLabel *labelOriginalTempo;
    QDoubleSpinBox *spinOriginalTempo;
    QLabel *labelNewTempo;
    QDoubleSpinBox *spinNewTempo;
    QLabel *labelProportion;
    QDoubleSpinBox *spinProportion;
    QLabel *labelDescription;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *TempoChangeDialog)
    {
        if (TempoChangeDialog->objectName().isEmpty())
            TempoChangeDialog->setObjectName("TempoChangeDialog");
        TempoChangeDialog->resize(320, 220);
        verticalLayout = new QVBoxLayout(TempoChangeDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupBoxTiming = new QGroupBox(TempoChangeDialog);
        groupBoxTiming->setObjectName("groupBoxTiming");
        formLayout = new QFormLayout(groupBoxTiming);
        formLayout->setObjectName("formLayout");
        labelOriginalTempo = new QLabel(groupBoxTiming);
        labelOriginalTempo->setObjectName("labelOriginalTempo");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelOriginalTempo);

        spinOriginalTempo = new QDoubleSpinBox(groupBoxTiming);
        spinOriginalTempo->setObjectName("spinOriginalTempo");
        spinOriginalTempo->setDecimals(1);
        spinOriginalTempo->setMinimum(1.000000000000000);
        spinOriginalTempo->setMaximum(999.000000000000000);
        spinOriginalTempo->setValue(120.000000000000000);

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, spinOriginalTempo);

        labelNewTempo = new QLabel(groupBoxTiming);
        labelNewTempo->setObjectName("labelNewTempo");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelNewTempo);

        spinNewTempo = new QDoubleSpinBox(groupBoxTiming);
        spinNewTempo->setObjectName("spinNewTempo");
        spinNewTempo->setDecimals(1);
        spinNewTempo->setMinimum(1.000000000000000);
        spinNewTempo->setMaximum(999.000000000000000);
        spinNewTempo->setValue(120.000000000000000);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, spinNewTempo);

        labelProportion = new QLabel(groupBoxTiming);
        labelProportion->setObjectName("labelProportion");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelProportion);

        spinProportion = new QDoubleSpinBox(groupBoxTiming);
        spinProportion->setObjectName("spinProportion");
        spinProportion->setDecimals(4);
        spinProportion->setMinimum(0.010000000000000);
        spinProportion->setMaximum(100.000000000000000);
        spinProportion->setSingleStep(0.100000000000000);
        spinProportion->setValue(1.000000000000000);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, spinProportion);


        verticalLayout->addWidget(groupBoxTiming);

        labelDescription = new QLabel(TempoChangeDialog);
        labelDescription->setObjectName("labelDescription");
        labelDescription->setWordWrap(true);
        labelDescription->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(labelDescription);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        buttonBox = new QDialogButtonBox(TempoChangeDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(TempoChangeDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, TempoChangeDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, TempoChangeDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(TempoChangeDialog);
    } // setupUi

    void retranslateUi(QDialog *TempoChangeDialog)
    {
        TempoChangeDialog->setWindowTitle(QCoreApplication::translate("TempoChangeDialog", "Scale Note Timing", nullptr));
        groupBoxTiming->setTitle(QCoreApplication::translate("TempoChangeDialog", "Tempo Proportion", nullptr));
        labelOriginalTempo->setText(QCoreApplication::translate("TempoChangeDialog", "Original Tempo:", nullptr));
        spinOriginalTempo->setSuffix(QCoreApplication::translate("TempoChangeDialog", " BPM", nullptr));
        labelNewTempo->setText(QCoreApplication::translate("TempoChangeDialog", "New Tempo:", nullptr));
        spinNewTempo->setSuffix(QCoreApplication::translate("TempoChangeDialog", " BPM", nullptr));
        labelProportion->setText(QCoreApplication::translate("TempoChangeDialog", "Proportion:", nullptr));
        spinProportion->setSuffix(QCoreApplication::translate("TempoChangeDialog", "\303\227", nullptr));
        labelDescription->setText(QCoreApplication::translate("TempoChangeDialog", "Proportion > 1.0 stretches notes (slower tempo).\n"
"Proportion < 1.0 compresses notes (faster tempo).", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TempoChangeDialog: public Ui_TempoChangeDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEMPOCHANGEDIALOG_H

/********************************************************************************
** Form generated from reading UI file 'easingdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EASINGDIALOG_H
#define UI_EASINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_EasingDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupEasing;
    QFormLayout *formLayoutEasing;
    QLabel *labelType;
    QComboBox *comboEasingType;
    QLabel *labelAnchor;
    QComboBox *comboAnchorMode;
    QGroupBox *groupParameters;
    QGridLayout *gridLayoutParams;
    QLabel *labelSineAmplitude;
    QSlider *sliderSineAmplitude;
    QDoubleSpinBox *spinSineAmplitude;
    QLabel *labelSineCycles;
    QSlider *sliderSineCycles;
    QDoubleSpinBox *spinSineCycles;
    QLabel *labelPower;
    QSlider *sliderPower;
    QDoubleSpinBox *spinPower;
    QLabel *labelExpoStrength;
    QSlider *sliderExpoStrength;
    QDoubleSpinBox *spinExpoStrength;
    QLabel *labelCircStrength;
    QSlider *sliderCircStrength;
    QDoubleSpinBox *spinCircStrength;
    QLabel *labelBounceCount;
    QSlider *sliderBounceCount;
    QDoubleSpinBox *spinBounceCount;
    QLabel *labelBounceStrength;
    QSlider *sliderBounceStrength;
    QDoubleSpinBox *spinBounceStrength;
    QLabel *labelOvershoot;
    QSlider *sliderOvershoot;
    QDoubleSpinBox *spinOvershoot;
    QLabel *labelAmplitude;
    QSlider *sliderAmplitude;
    QDoubleSpinBox *spinAmplitude;
    QLabel *labelPeriod;
    QSlider *sliderPeriod;
    QDoubleSpinBox *spinPeriod;
    QLabel *labelFrequency;
    QSlider *sliderFrequency;
    QDoubleSpinBox *spinFrequency;
    QLabel *labelDecay;
    QSlider *sliderDecay;
    QDoubleSpinBox *spinDecay;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *weightLayout;
    QLabel *labelWeight;
    QSlider *sliderWeight;
    QDoubleSpinBox *spinWeight;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *EasingDialog)
    {
        if (EasingDialog->objectName().isEmpty())
            EasingDialog->setObjectName("EasingDialog");
        EasingDialog->resize(380, 520);
        verticalLayout = new QVBoxLayout(EasingDialog);
        verticalLayout->setObjectName("verticalLayout");
        groupEasing = new QGroupBox(EasingDialog);
        groupEasing->setObjectName("groupEasing");
        formLayoutEasing = new QFormLayout(groupEasing);
        formLayoutEasing->setObjectName("formLayoutEasing");
        labelType = new QLabel(groupEasing);
        labelType->setObjectName("labelType");

        formLayoutEasing->setWidget(0, QFormLayout::ItemRole::LabelRole, labelType);

        comboEasingType = new QComboBox(groupEasing);
        comboEasingType->setObjectName("comboEasingType");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboEasingType->sizePolicy().hasHeightForWidth());
        comboEasingType->setSizePolicy(sizePolicy);

        formLayoutEasing->setWidget(0, QFormLayout::ItemRole::FieldRole, comboEasingType);

        labelAnchor = new QLabel(groupEasing);
        labelAnchor->setObjectName("labelAnchor");

        formLayoutEasing->setWidget(1, QFormLayout::ItemRole::LabelRole, labelAnchor);

        comboAnchorMode = new QComboBox(groupEasing);
        comboAnchorMode->addItem(QString());
        comboAnchorMode->addItem(QString());
        comboAnchorMode->addItem(QString());
        comboAnchorMode->addItem(QString());
        comboAnchorMode->setObjectName("comboAnchorMode");
        sizePolicy.setHeightForWidth(comboAnchorMode->sizePolicy().hasHeightForWidth());
        comboAnchorMode->setSizePolicy(sizePolicy);

        formLayoutEasing->setWidget(1, QFormLayout::ItemRole::FieldRole, comboAnchorMode);


        verticalLayout->addWidget(groupEasing);

        groupParameters = new QGroupBox(EasingDialog);
        groupParameters->setObjectName("groupParameters");
        gridLayoutParams = new QGridLayout(groupParameters);
        gridLayoutParams->setObjectName("gridLayoutParams");
        labelSineAmplitude = new QLabel(groupParameters);
        labelSineAmplitude->setObjectName("labelSineAmplitude");

        gridLayoutParams->addWidget(labelSineAmplitude, 0, 0, 1, 1);

        sliderSineAmplitude = new QSlider(groupParameters);
        sliderSineAmplitude->setObjectName("sliderSineAmplitude");
        sliderSineAmplitude->setMinimum(10);
        sliderSineAmplitude->setMaximum(300);
        sliderSineAmplitude->setValue(100);
        sliderSineAmplitude->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderSineAmplitude, 0, 1, 1, 1);

        spinSineAmplitude = new QDoubleSpinBox(groupParameters);
        spinSineAmplitude->setObjectName("spinSineAmplitude");
        spinSineAmplitude->setDecimals(2);
        spinSineAmplitude->setMinimum(0.100000000000000);
        spinSineAmplitude->setMaximum(3.000000000000000);
        spinSineAmplitude->setSingleStep(0.100000000000000);
        spinSineAmplitude->setValue(1.000000000000000);

        gridLayoutParams->addWidget(spinSineAmplitude, 0, 2, 1, 1);

        labelSineCycles = new QLabel(groupParameters);
        labelSineCycles->setObjectName("labelSineCycles");

        gridLayoutParams->addWidget(labelSineCycles, 1, 0, 1, 1);

        sliderSineCycles = new QSlider(groupParameters);
        sliderSineCycles->setObjectName("sliderSineCycles");
        sliderSineCycles->setMinimum(25);
        sliderSineCycles->setMaximum(400);
        sliderSineCycles->setValue(50);
        sliderSineCycles->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderSineCycles, 1, 1, 1, 1);

        spinSineCycles = new QDoubleSpinBox(groupParameters);
        spinSineCycles->setObjectName("spinSineCycles");
        spinSineCycles->setDecimals(2);
        spinSineCycles->setMinimum(0.250000000000000);
        spinSineCycles->setMaximum(4.000000000000000);
        spinSineCycles->setSingleStep(0.250000000000000);
        spinSineCycles->setValue(0.500000000000000);

        gridLayoutParams->addWidget(spinSineCycles, 1, 2, 1, 1);

        labelPower = new QLabel(groupParameters);
        labelPower->setObjectName("labelPower");

        gridLayoutParams->addWidget(labelPower, 2, 0, 1, 1);

        sliderPower = new QSlider(groupParameters);
        sliderPower->setObjectName("sliderPower");
        sliderPower->setMinimum(50);
        sliderPower->setMaximum(800);
        sliderPower->setValue(200);
        sliderPower->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderPower, 2, 1, 1, 1);

        spinPower = new QDoubleSpinBox(groupParameters);
        spinPower->setObjectName("spinPower");
        spinPower->setDecimals(1);
        spinPower->setMinimum(0.500000000000000);
        spinPower->setMaximum(8.000000000000000);
        spinPower->setSingleStep(0.500000000000000);
        spinPower->setValue(2.000000000000000);

        gridLayoutParams->addWidget(spinPower, 2, 2, 1, 1);

        labelExpoStrength = new QLabel(groupParameters);
        labelExpoStrength->setObjectName("labelExpoStrength");

        gridLayoutParams->addWidget(labelExpoStrength, 3, 0, 1, 1);

        sliderExpoStrength = new QSlider(groupParameters);
        sliderExpoStrength->setObjectName("sliderExpoStrength");
        sliderExpoStrength->setMinimum(10);
        sliderExpoStrength->setMaximum(200);
        sliderExpoStrength->setValue(100);
        sliderExpoStrength->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderExpoStrength, 3, 1, 1, 1);

        spinExpoStrength = new QDoubleSpinBox(groupParameters);
        spinExpoStrength->setObjectName("spinExpoStrength");
        spinExpoStrength->setDecimals(1);
        spinExpoStrength->setMinimum(1.000000000000000);
        spinExpoStrength->setMaximum(20.000000000000000);
        spinExpoStrength->setSingleStep(1.000000000000000);
        spinExpoStrength->setValue(10.000000000000000);

        gridLayoutParams->addWidget(spinExpoStrength, 3, 2, 1, 1);

        labelCircStrength = new QLabel(groupParameters);
        labelCircStrength->setObjectName("labelCircStrength");

        gridLayoutParams->addWidget(labelCircStrength, 4, 0, 1, 1);

        sliderCircStrength = new QSlider(groupParameters);
        sliderCircStrength->setObjectName("sliderCircStrength");
        sliderCircStrength->setMinimum(10);
        sliderCircStrength->setMaximum(300);
        sliderCircStrength->setValue(100);
        sliderCircStrength->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderCircStrength, 4, 1, 1, 1);

        spinCircStrength = new QDoubleSpinBox(groupParameters);
        spinCircStrength->setObjectName("spinCircStrength");
        spinCircStrength->setDecimals(2);
        spinCircStrength->setMinimum(0.100000000000000);
        spinCircStrength->setMaximum(3.000000000000000);
        spinCircStrength->setSingleStep(0.100000000000000);
        spinCircStrength->setValue(1.000000000000000);

        gridLayoutParams->addWidget(spinCircStrength, 4, 2, 1, 1);

        labelBounceCount = new QLabel(groupParameters);
        labelBounceCount->setObjectName("labelBounceCount");

        gridLayoutParams->addWidget(labelBounceCount, 5, 0, 1, 1);

        sliderBounceCount = new QSlider(groupParameters);
        sliderBounceCount->setObjectName("sliderBounceCount");
        sliderBounceCount->setMinimum(10);
        sliderBounceCount->setMaximum(80);
        sliderBounceCount->setValue(40);
        sliderBounceCount->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderBounceCount, 5, 1, 1, 1);

        spinBounceCount = new QDoubleSpinBox(groupParameters);
        spinBounceCount->setObjectName("spinBounceCount");
        spinBounceCount->setDecimals(1);
        spinBounceCount->setMinimum(1.000000000000000);
        spinBounceCount->setMaximum(8.000000000000000);
        spinBounceCount->setSingleStep(1.000000000000000);
        spinBounceCount->setValue(4.000000000000000);

        gridLayoutParams->addWidget(spinBounceCount, 5, 2, 1, 1);

        labelBounceStrength = new QLabel(groupParameters);
        labelBounceStrength->setObjectName("labelBounceStrength");

        gridLayoutParams->addWidget(labelBounceStrength, 6, 0, 1, 1);

        sliderBounceStrength = new QSlider(groupParameters);
        sliderBounceStrength->setObjectName("sliderBounceStrength");
        sliderBounceStrength->setMinimum(10);
        sliderBounceStrength->setMaximum(200);
        sliderBounceStrength->setValue(76);
        sliderBounceStrength->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderBounceStrength, 6, 1, 1, 1);

        spinBounceStrength = new QDoubleSpinBox(groupParameters);
        spinBounceStrength->setObjectName("spinBounceStrength");
        spinBounceStrength->setDecimals(2);
        spinBounceStrength->setMinimum(1.000000000000000);
        spinBounceStrength->setMaximum(20.000000000000000);
        spinBounceStrength->setSingleStep(0.500000000000000);
        spinBounceStrength->setValue(7.560000000000000);

        gridLayoutParams->addWidget(spinBounceStrength, 6, 2, 1, 1);

        labelOvershoot = new QLabel(groupParameters);
        labelOvershoot->setObjectName("labelOvershoot");

        gridLayoutParams->addWidget(labelOvershoot, 7, 0, 1, 1);

        sliderOvershoot = new QSlider(groupParameters);
        sliderOvershoot->setObjectName("sliderOvershoot");
        sliderOvershoot->setMinimum(0);
        sliderOvershoot->setMaximum(500);
        sliderOvershoot->setValue(170);
        sliderOvershoot->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderOvershoot, 7, 1, 1, 1);

        spinOvershoot = new QDoubleSpinBox(groupParameters);
        spinOvershoot->setObjectName("spinOvershoot");
        spinOvershoot->setDecimals(2);
        spinOvershoot->setMinimum(0.000000000000000);
        spinOvershoot->setMaximum(5.000000000000000);
        spinOvershoot->setSingleStep(0.100000000000000);
        spinOvershoot->setValue(1.700000000000000);

        gridLayoutParams->addWidget(spinOvershoot, 7, 2, 1, 1);

        labelAmplitude = new QLabel(groupParameters);
        labelAmplitude->setObjectName("labelAmplitude");

        gridLayoutParams->addWidget(labelAmplitude, 8, 0, 1, 1);

        sliderAmplitude = new QSlider(groupParameters);
        sliderAmplitude->setObjectName("sliderAmplitude");
        sliderAmplitude->setMinimum(10);
        sliderAmplitude->setMaximum(300);
        sliderAmplitude->setValue(100);
        sliderAmplitude->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderAmplitude, 8, 1, 1, 1);

        spinAmplitude = new QDoubleSpinBox(groupParameters);
        spinAmplitude->setObjectName("spinAmplitude");
        spinAmplitude->setDecimals(2);
        spinAmplitude->setMinimum(0.100000000000000);
        spinAmplitude->setMaximum(3.000000000000000);
        spinAmplitude->setSingleStep(0.100000000000000);
        spinAmplitude->setValue(1.000000000000000);

        gridLayoutParams->addWidget(spinAmplitude, 8, 2, 1, 1);

        labelPeriod = new QLabel(groupParameters);
        labelPeriod->setObjectName("labelPeriod");

        gridLayoutParams->addWidget(labelPeriod, 9, 0, 1, 1);

        sliderPeriod = new QSlider(groupParameters);
        sliderPeriod->setObjectName("sliderPeriod");
        sliderPeriod->setMinimum(5);
        sliderPeriod->setMaximum(100);
        sliderPeriod->setValue(30);
        sliderPeriod->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderPeriod, 9, 1, 1, 1);

        spinPeriod = new QDoubleSpinBox(groupParameters);
        spinPeriod->setObjectName("spinPeriod");
        spinPeriod->setDecimals(2);
        spinPeriod->setMinimum(0.050000000000000);
        spinPeriod->setMaximum(1.000000000000000);
        spinPeriod->setSingleStep(0.050000000000000);
        spinPeriod->setValue(0.300000000000000);

        gridLayoutParams->addWidget(spinPeriod, 9, 2, 1, 1);

        labelFrequency = new QLabel(groupParameters);
        labelFrequency->setObjectName("labelFrequency");

        gridLayoutParams->addWidget(labelFrequency, 10, 0, 1, 1);

        sliderFrequency = new QSlider(groupParameters);
        sliderFrequency->setObjectName("sliderFrequency");
        sliderFrequency->setMinimum(10);
        sliderFrequency->setMaximum(100);
        sliderFrequency->setValue(30);
        sliderFrequency->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderFrequency, 10, 1, 1, 1);

        spinFrequency = new QDoubleSpinBox(groupParameters);
        spinFrequency->setObjectName("spinFrequency");
        spinFrequency->setDecimals(1);
        spinFrequency->setMinimum(1.000000000000000);
        spinFrequency->setMaximum(10.000000000000000);
        spinFrequency->setSingleStep(0.500000000000000);
        spinFrequency->setValue(3.000000000000000);

        gridLayoutParams->addWidget(spinFrequency, 10, 2, 1, 1);

        labelDecay = new QLabel(groupParameters);
        labelDecay->setObjectName("labelDecay");

        gridLayoutParams->addWidget(labelDecay, 11, 0, 1, 1);

        sliderDecay = new QSlider(groupParameters);
        sliderDecay->setObjectName("sliderDecay");
        sliderDecay->setMinimum(10);
        sliderDecay->setMaximum(100);
        sliderDecay->setValue(80);
        sliderDecay->setOrientation(Qt::Horizontal);

        gridLayoutParams->addWidget(sliderDecay, 11, 1, 1, 1);

        spinDecay = new QDoubleSpinBox(groupParameters);
        spinDecay->setObjectName("spinDecay");
        spinDecay->setDecimals(2);
        spinDecay->setMinimum(0.100000000000000);
        spinDecay->setMaximum(1.000000000000000);
        spinDecay->setSingleStep(0.100000000000000);
        spinDecay->setValue(0.800000000000000);

        gridLayoutParams->addWidget(spinDecay, 11, 2, 1, 1);


        verticalLayout->addWidget(groupParameters);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        weightLayout = new QHBoxLayout();
        weightLayout->setObjectName("weightLayout");
        labelWeight = new QLabel(EasingDialog);
        labelWeight->setObjectName("labelWeight");

        weightLayout->addWidget(labelWeight);

        sliderWeight = new QSlider(EasingDialog);
        sliderWeight->setObjectName("sliderWeight");
        sliderWeight->setMinimum(0);
        sliderWeight->setMaximum(200);
        sliderWeight->setValue(100);
        sliderWeight->setOrientation(Qt::Horizontal);

        weightLayout->addWidget(sliderWeight);

        spinWeight = new QDoubleSpinBox(EasingDialog);
        spinWeight->setObjectName("spinWeight");
        spinWeight->setDecimals(2);
        spinWeight->setMinimum(0.000000000000000);
        spinWeight->setMaximum(2.000000000000000);
        spinWeight->setSingleStep(0.100000000000000);
        spinWeight->setValue(1.000000000000000);

        weightLayout->addWidget(spinWeight);


        verticalLayout->addLayout(weightLayout);

        buttonBox = new QDialogButtonBox(EasingDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(EasingDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, EasingDialog, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, EasingDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(EasingDialog);
    } // setupUi

    void retranslateUi(QDialog *EasingDialog)
    {
        EasingDialog->setWindowTitle(QCoreApplication::translate("EasingDialog", "Apply Rhythmic Easing", nullptr));
        groupEasing->setTitle(QCoreApplication::translate("EasingDialog", "Easing Type", nullptr));
        labelType->setText(QCoreApplication::translate("EasingDialog", "Type:", nullptr));
        labelAnchor->setText(QCoreApplication::translate("EasingDialog", "Anchor:", nullptr));
        comboAnchorMode->setItemText(0, QCoreApplication::translate("EasingDialog", "None (all notes move)", nullptr));
        comboAnchorMode->setItemText(1, QCoreApplication::translate("EasingDialog", "First note", nullptr));
        comboAnchorMode->setItemText(2, QCoreApplication::translate("EasingDialog", "Last note", nullptr));
        comboAnchorMode->setItemText(3, QCoreApplication::translate("EasingDialog", "First and last notes", nullptr));

#if QT_CONFIG(tooltip)
        comboAnchorMode->setToolTip(QCoreApplication::translate("EasingDialog", "Which notes stay fixed at their original positions", nullptr));
#endif // QT_CONFIG(tooltip)
        groupParameters->setTitle(QCoreApplication::translate("EasingDialog", "Parameters", nullptr));
        labelSineAmplitude->setText(QCoreApplication::translate("EasingDialog", "Amplitude:", nullptr));
        labelSineCycles->setText(QCoreApplication::translate("EasingDialog", "Cycles:", nullptr));
        labelPower->setText(QCoreApplication::translate("EasingDialog", "Power:", nullptr));
        labelExpoStrength->setText(QCoreApplication::translate("EasingDialog", "Strength:", nullptr));
        labelCircStrength->setText(QCoreApplication::translate("EasingDialog", "Curvature:", nullptr));
        labelBounceCount->setText(QCoreApplication::translate("EasingDialog", "Bounces:", nullptr));
        labelBounceStrength->setText(QCoreApplication::translate("EasingDialog", "Intensity:", nullptr));
        labelOvershoot->setText(QCoreApplication::translate("EasingDialog", "Overshoot:", nullptr));
        labelAmplitude->setText(QCoreApplication::translate("EasingDialog", "Amplitude:", nullptr));
        labelPeriod->setText(QCoreApplication::translate("EasingDialog", "Period:", nullptr));
        labelFrequency->setText(QCoreApplication::translate("EasingDialog", "Frequency:", nullptr));
        labelDecay->setText(QCoreApplication::translate("EasingDialog", "Decay:", nullptr));
        labelWeight->setText(QCoreApplication::translate("EasingDialog", "Weight:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class EasingDialog: public Ui_EasingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EASINGDIALOG_H

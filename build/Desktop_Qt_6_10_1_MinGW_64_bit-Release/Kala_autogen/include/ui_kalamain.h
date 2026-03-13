/********************************************************************************
** Form generated from reading UI file 'kalamain.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KALAMAIN_H
#define UI_KALAMAIN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_KalaMain
{
public:
    QAction *actionNew_project_Ctrl_N;
    QAction *actionOpen_ctrl_o;
    QAction *actionOpen_Recent;
    QAction *actionSave_ctrl_S;
    QAction *actionSave_AS_Ctrl_SHift_S;
    QAction *actionExport_Audio_Ctrl_E;
    QAction *actionExit_Alt_F4;
    QAction *actionUndo_Ctrl_Z;
    QAction *actionRedo_Ctrl_Y;
    QAction *actionCut_Ctrl_X;
    QAction *actionCopy_Ctrl_C;
    QAction *actionPaste_Ctrl_V;
    QAction *actionDuplicate_Ctrl_D;
    QAction *actionDelete_Del;
    QAction *actionSelect_All_ctrl_A;
    QAction *actionDeselect_Esc;
    QAction *actionNew_Sounit;
    QAction *actionLoad_Sounit;
    QAction *actionSave_Sounit_As;
    QAction *actionHarmonic_Generator;
    QAction *actionSpectrum_To_Signal;
    QAction *actionRolloff_Processor;
    QAction *actionFormant_Body;
    QAction *actionBreth_Turbulance;
    QAction *actionNoise_Color_Filter;
    QAction *actionPhysics_System;
    QAction *actionEasing_Applicator;
    QAction *actionEnvelope_Engine;
    QAction *actionDrift_Engine;
    QAction *actionGate_Processor;
    QAction *actionDelete_Container;
    QAction *actionDelete_Container_2;
    QAction *actionDNA_Library;
    QAction *actionEnvelope_Library;
    QAction *actionEasing_Library;
    QAction *actionNew_track;
    QAction *actionDelete_Track;
    QAction *actionScale_Settings;
    QAction *actionAssign_Sounit_to_Track;
    QAction *actionCanvas_Window;
    QAction *actionFull_Screen_Canvas_F11;
    QAction *actionZoom_In_Ctrl;
    QAction *actionZoom_Out_Crl;
    QAction *actionZoom_to_fit_Ctrl_0;
    QAction *actionSound_Engine_Tab_Ctrl_1;
    QAction *actionCompisition_Tab_Ctrl_2;
    QAction *actionPreferences_Tab_Ctrl_3;
    QAction *actionDocumentation_F1;
    QAction *actionKeyboard_Shortcuts;
    QAction *actionAbout_Kala;
    QAction *actionComposition_Settings;
    QAction *actionTablet_mode;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_4;
    QTabWidget *MainTab;
    QWidget *soundEngineTab;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *groupSounitInfo;
    QHBoxLayout *horizontalLayout_sounitInfo;
    QFormLayout *formLayout_2;
    QLabel *label;
    QComboBox *comboSounitSelector;
    QLabel *labelSounitName;
    QLineEdit *editSounitName;
    QLabel *labelVariation;
    QComboBox *comboVariationSelector;
    QGroupBox *groupComment;
    QVBoxLayout *verticalLayout_comment;
    QPlainTextEdit *editSounitComment;
    QGroupBox *groupContainerInspector;
    QHBoxLayout *horizontalLayout_16;
    QFrame *frame;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_5;
    QLineEdit *editInstanceName;
    QLabel *label_4;
    QLabel *labelContainerType;
    QLabel *labelContainerDescription;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_PortsStacked;
    QGroupBox *groupInputs;
    QVBoxLayout *verticalLayout_10;
    QListWidget *listWidgetInputs;
    QGroupBox *groupOutputs;
    QVBoxLayout *verticalLayout_11;
    QListWidget *listWidgetOutputs;
    QGroupBox *groupConfig;
    QVBoxLayout *verticalLayout_9;
    QScrollArea *scrollAreaConfig;
    QWidget *scrollAreaConfigContents;
    QVBoxLayout *verticalLayout_Config;
    QGroupBox *groupConnectionInspector;
    QFormLayout *formLayout_3;
    QLabel *label_7;
    QLabel *labelConnectionTo;
    QLabel *label_8;
    QComboBox *comboConnectionFunction;
    QLabel *label_9;
    QDoubleSpinBox *spinConnectionWeight;
    QPushButton *buttonDisconnect;
    QLabel *labelConnectionFrom;
    QLabel *label_6;
    QWidget *compositionTab;
    QVBoxLayout *verticalLayout_14;
    QGroupBox *groupInspector;
    QHBoxLayout *horizontalLayout;
    QTabWidget *tabInspector;
    QWidget *tabNoteInspector;
    QGridLayout *gridLayout;
    QGroupBox *positiongroupBox;
    QVBoxLayout *verticalLayout;
    QLabel *label_11;
    QDoubleSpinBox *spinNoteStart;
    QLabel *label_14;
    QDoubleSpinBox *spinNoteDuration;
    QLabel *label_16;
    QSpinBox *spinNotePitch;
    QLabel *noteVariationLabel;
    QComboBox *noteVariationComboBox;
    QGroupBox *vibratoGroupBox;
    QFormLayout *formLayout;
    QCheckBox *checkNoteVibrato;
    QPushButton *btnNoteVibratoEdit;
    QGroupBox *expressiveCurvesGroupBox;
    QVBoxLayout *verticalLayoutExpressiveCurves;
    QComboBox *comboExpressiveCurve;
    QHBoxLayout *hboxLayout;
    QPushButton *btnExpressiveCurveAdd;
    QPushButton *btnExpressiveCurveDelete;
    QPushButton *btnExpressiveCurveApply;
    QGroupBox *groupVariations;
    QVBoxLayout *verticalLayoutVariations;
    QLineEdit *editVariationsTrackName;
    QListWidget *listVariations;
    QHBoxLayout *horizontalLayoutVariationButtons;
    QPushButton *btnDeleteVariation;
    QPushButton *btnRenameVariation;
    QGroupBox *groupTracks;
    QVBoxLayout *verticalLayoutTracks;
    QScrollArea *scrollAreaMixer;
    QWidget *scrollAreaMixerContents;
    QHBoxLayout *layoutMixerTracks;
    QPushButton *btnAddTrack;
    QGroupBox *groupScale;
    QHBoxLayout *horizontalLayout_5;
    QSpinBox *spinScaleMarker;
    QLabel *labelScaleDisplay;
    QPushButton *btnScaleEdit;
    QPushButton *btnScaleAdd;
    QGroupBox *groupTempo;
    QHBoxLayout *horizontalLayout_Tempo;
    QSpinBox *spinTempoMarker;
    QLabel *labelTempoDisplay;
    QPushButton *btnTempoEdit;
    QPushButton *btnTempoAdd;
    QWidget *preferencesTab;
    QVBoxLayout *verticalLayout_13;
    QLabel *label_15;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuSound;
    QMenu *menuAdd_Container;
    QMenu *menuEssential;
    QMenu *menuShaping;
    QMenu *menuModifiers;
    QMenu *menuCompose;
    QMenu *menuView;
    QMenu *menuHelp;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *KalaMain)
    {
        if (KalaMain->objectName().isEmpty())
            KalaMain->setObjectName("KalaMain");
        KalaMain->resize(1089, 1071);
        actionNew_project_Ctrl_N = new QAction(KalaMain);
        actionNew_project_Ctrl_N->setObjectName("actionNew_project_Ctrl_N");
        actionOpen_ctrl_o = new QAction(KalaMain);
        actionOpen_ctrl_o->setObjectName("actionOpen_ctrl_o");
        actionOpen_Recent = new QAction(KalaMain);
        actionOpen_Recent->setObjectName("actionOpen_Recent");
        actionSave_ctrl_S = new QAction(KalaMain);
        actionSave_ctrl_S->setObjectName("actionSave_ctrl_S");
        actionSave_AS_Ctrl_SHift_S = new QAction(KalaMain);
        actionSave_AS_Ctrl_SHift_S->setObjectName("actionSave_AS_Ctrl_SHift_S");
        actionExport_Audio_Ctrl_E = new QAction(KalaMain);
        actionExport_Audio_Ctrl_E->setObjectName("actionExport_Audio_Ctrl_E");
        actionExit_Alt_F4 = new QAction(KalaMain);
        actionExit_Alt_F4->setObjectName("actionExit_Alt_F4");
        actionUndo_Ctrl_Z = new QAction(KalaMain);
        actionUndo_Ctrl_Z->setObjectName("actionUndo_Ctrl_Z");
        actionRedo_Ctrl_Y = new QAction(KalaMain);
        actionRedo_Ctrl_Y->setObjectName("actionRedo_Ctrl_Y");
        actionCut_Ctrl_X = new QAction(KalaMain);
        actionCut_Ctrl_X->setObjectName("actionCut_Ctrl_X");
        actionCopy_Ctrl_C = new QAction(KalaMain);
        actionCopy_Ctrl_C->setObjectName("actionCopy_Ctrl_C");
        actionPaste_Ctrl_V = new QAction(KalaMain);
        actionPaste_Ctrl_V->setObjectName("actionPaste_Ctrl_V");
        actionDuplicate_Ctrl_D = new QAction(KalaMain);
        actionDuplicate_Ctrl_D->setObjectName("actionDuplicate_Ctrl_D");
        actionDelete_Del = new QAction(KalaMain);
        actionDelete_Del->setObjectName("actionDelete_Del");
        actionSelect_All_ctrl_A = new QAction(KalaMain);
        actionSelect_All_ctrl_A->setObjectName("actionSelect_All_ctrl_A");
        actionDeselect_Esc = new QAction(KalaMain);
        actionDeselect_Esc->setObjectName("actionDeselect_Esc");
        actionNew_Sounit = new QAction(KalaMain);
        actionNew_Sounit->setObjectName("actionNew_Sounit");
        actionLoad_Sounit = new QAction(KalaMain);
        actionLoad_Sounit->setObjectName("actionLoad_Sounit");
        actionSave_Sounit_As = new QAction(KalaMain);
        actionSave_Sounit_As->setObjectName("actionSave_Sounit_As");
        actionHarmonic_Generator = new QAction(KalaMain);
        actionHarmonic_Generator->setObjectName("actionHarmonic_Generator");
        actionSpectrum_To_Signal = new QAction(KalaMain);
        actionSpectrum_To_Signal->setObjectName("actionSpectrum_To_Signal");
        actionRolloff_Processor = new QAction(KalaMain);
        actionRolloff_Processor->setObjectName("actionRolloff_Processor");
        actionFormant_Body = new QAction(KalaMain);
        actionFormant_Body->setObjectName("actionFormant_Body");
        actionBreth_Turbulance = new QAction(KalaMain);
        actionBreth_Turbulance->setObjectName("actionBreth_Turbulance");
        actionNoise_Color_Filter = new QAction(KalaMain);
        actionNoise_Color_Filter->setObjectName("actionNoise_Color_Filter");
        actionPhysics_System = new QAction(KalaMain);
        actionPhysics_System->setObjectName("actionPhysics_System");
        actionEasing_Applicator = new QAction(KalaMain);
        actionEasing_Applicator->setObjectName("actionEasing_Applicator");
        actionEnvelope_Engine = new QAction(KalaMain);
        actionEnvelope_Engine->setObjectName("actionEnvelope_Engine");
        actionDrift_Engine = new QAction(KalaMain);
        actionDrift_Engine->setObjectName("actionDrift_Engine");
        actionGate_Processor = new QAction(KalaMain);
        actionGate_Processor->setObjectName("actionGate_Processor");
        actionDelete_Container = new QAction(KalaMain);
        actionDelete_Container->setObjectName("actionDelete_Container");
        actionDelete_Container_2 = new QAction(KalaMain);
        actionDelete_Container_2->setObjectName("actionDelete_Container_2");
        actionDNA_Library = new QAction(KalaMain);
        actionDNA_Library->setObjectName("actionDNA_Library");
        actionEnvelope_Library = new QAction(KalaMain);
        actionEnvelope_Library->setObjectName("actionEnvelope_Library");
        actionEasing_Library = new QAction(KalaMain);
        actionEasing_Library->setObjectName("actionEasing_Library");
        actionNew_track = new QAction(KalaMain);
        actionNew_track->setObjectName("actionNew_track");
        actionDelete_Track = new QAction(KalaMain);
        actionDelete_Track->setObjectName("actionDelete_Track");
        actionScale_Settings = new QAction(KalaMain);
        actionScale_Settings->setObjectName("actionScale_Settings");
        actionAssign_Sounit_to_Track = new QAction(KalaMain);
        actionAssign_Sounit_to_Track->setObjectName("actionAssign_Sounit_to_Track");
        actionCanvas_Window = new QAction(KalaMain);
        actionCanvas_Window->setObjectName("actionCanvas_Window");
        actionFull_Screen_Canvas_F11 = new QAction(KalaMain);
        actionFull_Screen_Canvas_F11->setObjectName("actionFull_Screen_Canvas_F11");
        actionZoom_In_Ctrl = new QAction(KalaMain);
        actionZoom_In_Ctrl->setObjectName("actionZoom_In_Ctrl");
        actionZoom_Out_Crl = new QAction(KalaMain);
        actionZoom_Out_Crl->setObjectName("actionZoom_Out_Crl");
        actionZoom_to_fit_Ctrl_0 = new QAction(KalaMain);
        actionZoom_to_fit_Ctrl_0->setObjectName("actionZoom_to_fit_Ctrl_0");
        actionSound_Engine_Tab_Ctrl_1 = new QAction(KalaMain);
        actionSound_Engine_Tab_Ctrl_1->setObjectName("actionSound_Engine_Tab_Ctrl_1");
        actionCompisition_Tab_Ctrl_2 = new QAction(KalaMain);
        actionCompisition_Tab_Ctrl_2->setObjectName("actionCompisition_Tab_Ctrl_2");
        actionPreferences_Tab_Ctrl_3 = new QAction(KalaMain);
        actionPreferences_Tab_Ctrl_3->setObjectName("actionPreferences_Tab_Ctrl_3");
        actionDocumentation_F1 = new QAction(KalaMain);
        actionDocumentation_F1->setObjectName("actionDocumentation_F1");
        actionKeyboard_Shortcuts = new QAction(KalaMain);
        actionKeyboard_Shortcuts->setObjectName("actionKeyboard_Shortcuts");
        actionAbout_Kala = new QAction(KalaMain);
        actionAbout_Kala->setObjectName("actionAbout_Kala");
        actionComposition_Settings = new QAction(KalaMain);
        actionComposition_Settings->setObjectName("actionComposition_Settings");
        actionTablet_mode = new QAction(KalaMain);
        actionTablet_mode->setObjectName("actionTablet_mode");
        centralwidget = new QWidget(KalaMain);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_4 = new QVBoxLayout(centralwidget);
        verticalLayout_4->setObjectName("verticalLayout_4");
        MainTab = new QTabWidget(centralwidget);
        MainTab->setObjectName("MainTab");
        soundEngineTab = new QWidget();
        soundEngineTab->setObjectName("soundEngineTab");
        verticalLayout_5 = new QVBoxLayout(soundEngineTab);
        verticalLayout_5->setObjectName("verticalLayout_5");
        groupSounitInfo = new QGroupBox(soundEngineTab);
        groupSounitInfo->setObjectName("groupSounitInfo");
        horizontalLayout_sounitInfo = new QHBoxLayout(groupSounitInfo);
        horizontalLayout_sounitInfo->setObjectName("horizontalLayout_sounitInfo");
        formLayout_2 = new QFormLayout();
        formLayout_2->setObjectName("formLayout_2");
        label = new QLabel(groupSounitInfo);
        label->setObjectName("label");

        formLayout_2->setWidget(0, QFormLayout::ItemRole::LabelRole, label);

        comboSounitSelector = new QComboBox(groupSounitInfo);
        comboSounitSelector->setObjectName("comboSounitSelector");
        comboSounitSelector->setMaximumSize(QSize(500, 16777215));
        comboSounitSelector->setEditable(false);

        formLayout_2->setWidget(0, QFormLayout::ItemRole::FieldRole, comboSounitSelector);

        labelSounitName = new QLabel(groupSounitInfo);
        labelSounitName->setObjectName("labelSounitName");

        formLayout_2->setWidget(1, QFormLayout::ItemRole::LabelRole, labelSounitName);

        editSounitName = new QLineEdit(groupSounitInfo);
        editSounitName->setObjectName("editSounitName");
        editSounitName->setMaximumSize(QSize(500, 16777215));

        formLayout_2->setWidget(1, QFormLayout::ItemRole::FieldRole, editSounitName);

        labelVariation = new QLabel(groupSounitInfo);
        labelVariation->setObjectName("labelVariation");

        formLayout_2->setWidget(2, QFormLayout::ItemRole::LabelRole, labelVariation);

        comboVariationSelector = new QComboBox(groupSounitInfo);
        comboVariationSelector->setObjectName("comboVariationSelector");
        comboVariationSelector->setMaximumSize(QSize(500, 16777215));
        comboVariationSelector->setEditable(false);

        formLayout_2->setWidget(2, QFormLayout::ItemRole::FieldRole, comboVariationSelector);


        horizontalLayout_sounitInfo->addLayout(formLayout_2);

        groupComment = new QGroupBox(groupSounitInfo);
        groupComment->setObjectName("groupComment");
        verticalLayout_comment = new QVBoxLayout(groupComment);
        verticalLayout_comment->setObjectName("verticalLayout_comment");
        editSounitComment = new QPlainTextEdit(groupComment);
        editSounitComment->setObjectName("editSounitComment");

        verticalLayout_comment->addWidget(editSounitComment);


        horizontalLayout_sounitInfo->addWidget(groupComment);


        verticalLayout_5->addWidget(groupSounitInfo);

        groupContainerInspector = new QGroupBox(soundEngineTab);
        groupContainerInspector->setObjectName("groupContainerInspector");
        horizontalLayout_16 = new QHBoxLayout(groupContainerInspector);
        horizontalLayout_16->setObjectName("horizontalLayout_16");
        frame = new QFrame(groupContainerInspector);
        frame->setObjectName("frame");
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_2 = new QVBoxLayout(frame);
        verticalLayout_2->setObjectName("verticalLayout_2");
        label_5 = new QLabel(frame);
        label_5->setObjectName("label_5");
        label_5->setAlignment(Qt::AlignmentFlag::AlignBottom|Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft);

        verticalLayout_2->addWidget(label_5);

        editInstanceName = new QLineEdit(frame);
        editInstanceName->setObjectName("editInstanceName");

        verticalLayout_2->addWidget(editInstanceName);

        label_4 = new QLabel(frame);
        label_4->setObjectName("label_4");
        label_4->setAlignment(Qt::AlignmentFlag::AlignBottom|Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft);

        verticalLayout_2->addWidget(label_4);

        labelContainerType = new QLabel(frame);
        labelContainerType->setObjectName("labelContainerType");
        labelContainerType->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);

        verticalLayout_2->addWidget(labelContainerType);

        labelContainerDescription = new QLabel(frame);
        labelContainerDescription->setObjectName("labelContainerDescription");
        labelContainerDescription->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignTop);
        labelContainerDescription->setWordWrap(true);

        verticalLayout_2->addWidget(labelContainerDescription);


        horizontalLayout_16->addWidget(frame);

        frame_2 = new QFrame(groupContainerInspector);
        frame_2->setObjectName("frame_2");
        frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_PortsStacked = new QVBoxLayout(frame_2);
        verticalLayout_PortsStacked->setObjectName("verticalLayout_PortsStacked");
        groupInputs = new QGroupBox(frame_2);
        groupInputs->setObjectName("groupInputs");
        verticalLayout_10 = new QVBoxLayout(groupInputs);
        verticalLayout_10->setObjectName("verticalLayout_10");
        listWidgetInputs = new QListWidget(groupInputs);
        listWidgetInputs->setObjectName("listWidgetInputs");
        listWidgetInputs->setMinimumSize(QSize(0, 0));

        verticalLayout_10->addWidget(listWidgetInputs);


        verticalLayout_PortsStacked->addWidget(groupInputs);

        groupOutputs = new QGroupBox(frame_2);
        groupOutputs->setObjectName("groupOutputs");
        verticalLayout_11 = new QVBoxLayout(groupOutputs);
        verticalLayout_11->setObjectName("verticalLayout_11");
        listWidgetOutputs = new QListWidget(groupOutputs);
        listWidgetOutputs->setObjectName("listWidgetOutputs");
        listWidgetOutputs->setMinimumSize(QSize(0, 0));

        verticalLayout_11->addWidget(listWidgetOutputs);


        verticalLayout_PortsStacked->addWidget(groupOutputs);


        horizontalLayout_16->addWidget(frame_2);

        groupConfig = new QGroupBox(groupContainerInspector);
        groupConfig->setObjectName("groupConfig");
        verticalLayout_9 = new QVBoxLayout(groupConfig);
        verticalLayout_9->setObjectName("verticalLayout_9");
        scrollAreaConfig = new QScrollArea(groupConfig);
        scrollAreaConfig->setObjectName("scrollAreaConfig");
        scrollAreaConfig->setWidgetResizable(true);
        scrollAreaConfigContents = new QWidget();
        scrollAreaConfigContents->setObjectName("scrollAreaConfigContents");
        scrollAreaConfigContents->setGeometry(QRect(0, 0, 308, 449));
        verticalLayout_Config = new QVBoxLayout(scrollAreaConfigContents);
        verticalLayout_Config->setSpacing(6);
        verticalLayout_Config->setObjectName("verticalLayout_Config");
        verticalLayout_Config->setContentsMargins(0, 0, 0, 0);
        scrollAreaConfig->setWidget(scrollAreaConfigContents);

        verticalLayout_9->addWidget(scrollAreaConfig);


        horizontalLayout_16->addWidget(groupConfig);


        verticalLayout_5->addWidget(groupContainerInspector);

        groupConnectionInspector = new QGroupBox(soundEngineTab);
        groupConnectionInspector->setObjectName("groupConnectionInspector");
        formLayout_3 = new QFormLayout(groupConnectionInspector);
        formLayout_3->setObjectName("formLayout_3");
        label_7 = new QLabel(groupConnectionInspector);
        label_7->setObjectName("label_7");

        formLayout_3->setWidget(2, QFormLayout::ItemRole::LabelRole, label_7);

        labelConnectionTo = new QLabel(groupConnectionInspector);
        labelConnectionTo->setObjectName("labelConnectionTo");

        formLayout_3->setWidget(2, QFormLayout::ItemRole::FieldRole, labelConnectionTo);

        label_8 = new QLabel(groupConnectionInspector);
        label_8->setObjectName("label_8");

        formLayout_3->setWidget(3, QFormLayout::ItemRole::LabelRole, label_8);

        comboConnectionFunction = new QComboBox(groupConnectionInspector);
        comboConnectionFunction->addItem(QString());
        comboConnectionFunction->addItem(QString());
        comboConnectionFunction->addItem(QString());
        comboConnectionFunction->addItem(QString());
        comboConnectionFunction->addItem(QString());
        comboConnectionFunction->addItem(QString());
        comboConnectionFunction->setObjectName("comboConnectionFunction");
        comboConnectionFunction->setEditable(false);

        formLayout_3->setWidget(3, QFormLayout::ItemRole::FieldRole, comboConnectionFunction);

        label_9 = new QLabel(groupConnectionInspector);
        label_9->setObjectName("label_9");

        formLayout_3->setWidget(4, QFormLayout::ItemRole::LabelRole, label_9);

        spinConnectionWeight = new QDoubleSpinBox(groupConnectionInspector);
        spinConnectionWeight->setObjectName("spinConnectionWeight");

        formLayout_3->setWidget(4, QFormLayout::ItemRole::FieldRole, spinConnectionWeight);

        buttonDisconnect = new QPushButton(groupConnectionInspector);
        buttonDisconnect->setObjectName("buttonDisconnect");

        formLayout_3->setWidget(5, QFormLayout::ItemRole::LabelRole, buttonDisconnect);

        labelConnectionFrom = new QLabel(groupConnectionInspector);
        labelConnectionFrom->setObjectName("labelConnectionFrom");

        formLayout_3->setWidget(1, QFormLayout::ItemRole::FieldRole, labelConnectionFrom);

        label_6 = new QLabel(groupConnectionInspector);
        label_6->setObjectName("label_6");

        formLayout_3->setWidget(1, QFormLayout::ItemRole::LabelRole, label_6);


        verticalLayout_5->addWidget(groupConnectionInspector);

        MainTab->addTab(soundEngineTab, QString());
        compositionTab = new QWidget();
        compositionTab->setObjectName("compositionTab");
        verticalLayout_14 = new QVBoxLayout(compositionTab);
        verticalLayout_14->setObjectName("verticalLayout_14");
        groupInspector = new QGroupBox(compositionTab);
        groupInspector->setObjectName("groupInspector");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupInspector->sizePolicy().hasHeightForWidth());
        groupInspector->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(groupInspector);
        horizontalLayout->setObjectName("horizontalLayout");
        tabInspector = new QTabWidget(groupInspector);
        tabInspector->setObjectName("tabInspector");
        tabNoteInspector = new QWidget();
        tabNoteInspector->setObjectName("tabNoteInspector");
        gridLayout = new QGridLayout(tabNoteInspector);
        gridLayout->setObjectName("gridLayout");
        positiongroupBox = new QGroupBox(tabNoteInspector);
        positiongroupBox->setObjectName("positiongroupBox");
        positiongroupBox->setMinimumSize(QSize(200, 0));
        verticalLayout = new QVBoxLayout(positiongroupBox);
        verticalLayout->setObjectName("verticalLayout");
        label_11 = new QLabel(positiongroupBox);
        label_11->setObjectName("label_11");

        verticalLayout->addWidget(label_11);

        spinNoteStart = new QDoubleSpinBox(positiongroupBox);
        spinNoteStart->setObjectName("spinNoteStart");

        verticalLayout->addWidget(spinNoteStart);

        label_14 = new QLabel(positiongroupBox);
        label_14->setObjectName("label_14");

        verticalLayout->addWidget(label_14);

        spinNoteDuration = new QDoubleSpinBox(positiongroupBox);
        spinNoteDuration->setObjectName("spinNoteDuration");

        verticalLayout->addWidget(spinNoteDuration);

        label_16 = new QLabel(positiongroupBox);
        label_16->setObjectName("label_16");

        verticalLayout->addWidget(label_16);

        spinNotePitch = new QSpinBox(positiongroupBox);
        spinNotePitch->setObjectName("spinNotePitch");

        verticalLayout->addWidget(spinNotePitch);

        noteVariationLabel = new QLabel(positiongroupBox);
        noteVariationLabel->setObjectName("noteVariationLabel");

        verticalLayout->addWidget(noteVariationLabel);

        noteVariationComboBox = new QComboBox(positiongroupBox);
        noteVariationComboBox->setObjectName("noteVariationComboBox");

        verticalLayout->addWidget(noteVariationComboBox);


        gridLayout->addWidget(positiongroupBox, 0, 0, 1, 1);

        vibratoGroupBox = new QGroupBox(tabNoteInspector);
        vibratoGroupBox->setObjectName("vibratoGroupBox");
        formLayout = new QFormLayout(vibratoGroupBox);
        formLayout->setObjectName("formLayout");
        checkNoteVibrato = new QCheckBox(vibratoGroupBox);
        checkNoteVibrato->setObjectName("checkNoteVibrato");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, checkNoteVibrato);

        btnNoteVibratoEdit = new QPushButton(vibratoGroupBox);
        btnNoteVibratoEdit->setObjectName("btnNoteVibratoEdit");

        formLayout->setWidget(1, QFormLayout::ItemRole::SpanningRole, btnNoteVibratoEdit);


        gridLayout->addWidget(vibratoGroupBox, 0, 1, 1, 1);

        expressiveCurvesGroupBox = new QGroupBox(tabNoteInspector);
        expressiveCurvesGroupBox->setObjectName("expressiveCurvesGroupBox");
        verticalLayoutExpressiveCurves = new QVBoxLayout(expressiveCurvesGroupBox);
        verticalLayoutExpressiveCurves->setObjectName("verticalLayoutExpressiveCurves");
        comboExpressiveCurve = new QComboBox(expressiveCurvesGroupBox);
        comboExpressiveCurve->setObjectName("comboExpressiveCurve");

        verticalLayoutExpressiveCurves->addWidget(comboExpressiveCurve);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setObjectName("hboxLayout");
        btnExpressiveCurveAdd = new QPushButton(expressiveCurvesGroupBox);
        btnExpressiveCurveAdd->setObjectName("btnExpressiveCurveAdd");

        hboxLayout->addWidget(btnExpressiveCurveAdd);

        btnExpressiveCurveDelete = new QPushButton(expressiveCurvesGroupBox);
        btnExpressiveCurveDelete->setObjectName("btnExpressiveCurveDelete");

        hboxLayout->addWidget(btnExpressiveCurveDelete);

        btnExpressiveCurveApply = new QPushButton(expressiveCurvesGroupBox);
        btnExpressiveCurveApply->setObjectName("btnExpressiveCurveApply");

        hboxLayout->addWidget(btnExpressiveCurveApply);


        verticalLayoutExpressiveCurves->addLayout(hboxLayout);


        gridLayout->addWidget(expressiveCurvesGroupBox, 0, 2, 1, 1);

        tabInspector->addTab(tabNoteInspector, QString());

        horizontalLayout->addWidget(tabInspector);

        groupVariations = new QGroupBox(groupInspector);
        groupVariations->setObjectName("groupVariations");
        verticalLayoutVariations = new QVBoxLayout(groupVariations);
        verticalLayoutVariations->setSpacing(4);
        verticalLayoutVariations->setObjectName("verticalLayoutVariations");
        editVariationsTrackName = new QLineEdit(groupVariations);
        editVariationsTrackName->setObjectName("editVariationsTrackName");

        verticalLayoutVariations->addWidget(editVariationsTrackName);

        listVariations = new QListWidget(groupVariations);
        listVariations->setObjectName("listVariations");
        listVariations->setMaximumSize(QSize(16777215, 100));

        verticalLayoutVariations->addWidget(listVariations);

        horizontalLayoutVariationButtons = new QHBoxLayout();
        horizontalLayoutVariationButtons->setObjectName("horizontalLayoutVariationButtons");
        btnDeleteVariation = new QPushButton(groupVariations);
        btnDeleteVariation->setObjectName("btnDeleteVariation");
        btnDeleteVariation->setEnabled(false);

        horizontalLayoutVariationButtons->addWidget(btnDeleteVariation);

        btnRenameVariation = new QPushButton(groupVariations);
        btnRenameVariation->setObjectName("btnRenameVariation");
        btnRenameVariation->setEnabled(false);

        horizontalLayoutVariationButtons->addWidget(btnRenameVariation);


        verticalLayoutVariations->addLayout(horizontalLayoutVariationButtons);


        horizontalLayout->addWidget(groupVariations);


        verticalLayout_14->addWidget(groupInspector);

        groupTracks = new QGroupBox(compositionTab);
        groupTracks->setObjectName("groupTracks");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(groupTracks->sizePolicy().hasHeightForWidth());
        groupTracks->setSizePolicy(sizePolicy1);
        groupTracks->setMinimumSize(QSize(0, 120));
        verticalLayoutTracks = new QVBoxLayout(groupTracks);
        verticalLayoutTracks->setSpacing(4);
        verticalLayoutTracks->setObjectName("verticalLayoutTracks");
        verticalLayoutTracks->setContentsMargins(4, 4, 4, 4);
        scrollAreaMixer = new QScrollArea(groupTracks);
        scrollAreaMixer->setObjectName("scrollAreaMixer");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(1);
        sizePolicy2.setHeightForWidth(scrollAreaMixer->sizePolicy().hasHeightForWidth());
        scrollAreaMixer->setSizePolicy(sizePolicy2);
        scrollAreaMixer->setFrameShape(QFrame::Shape::NoFrame);
        scrollAreaMixer->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        scrollAreaMixer->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        scrollAreaMixer->setWidgetResizable(true);
        scrollAreaMixerContents = new QWidget();
        scrollAreaMixerContents->setObjectName("scrollAreaMixerContents");
        scrollAreaMixerContents->setGeometry(QRect(0, 0, 1029, 498));
        layoutMixerTracks = new QHBoxLayout(scrollAreaMixerContents);
        layoutMixerTracks->setSpacing(4);
        layoutMixerTracks->setObjectName("layoutMixerTracks");
        layoutMixerTracks->setContentsMargins(0, 0, 0, 0);
        scrollAreaMixer->setWidget(scrollAreaMixerContents);

        verticalLayoutTracks->addWidget(scrollAreaMixer);

        btnAddTrack = new QPushButton(groupTracks);
        btnAddTrack->setObjectName("btnAddTrack");

        verticalLayoutTracks->addWidget(btnAddTrack);


        verticalLayout_14->addWidget(groupTracks);

        groupScale = new QGroupBox(compositionTab);
        groupScale->setObjectName("groupScale");
        horizontalLayout_5 = new QHBoxLayout(groupScale);
        horizontalLayout_5->setObjectName("horizontalLayout_5");
        spinScaleMarker = new QSpinBox(groupScale);
        spinScaleMarker->setObjectName("spinScaleMarker");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(spinScaleMarker->sizePolicy().hasHeightForWidth());
        spinScaleMarker->setSizePolicy(sizePolicy3);
        spinScaleMarker->setMinimumSize(QSize(50, 0));
        spinScaleMarker->setMinimum(1);
        spinScaleMarker->setMaximum(1);
        spinScaleMarker->setValue(1);

        horizontalLayout_5->addWidget(spinScaleMarker);

        labelScaleDisplay = new QLabel(groupScale);
        labelScaleDisplay->setObjectName("labelScaleDisplay");
        QSizePolicy sizePolicy4(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(labelScaleDisplay->sizePolicy().hasHeightForWidth());
        labelScaleDisplay->setSizePolicy(sizePolicy4);

        horizontalLayout_5->addWidget(labelScaleDisplay);

        btnScaleEdit = new QPushButton(groupScale);
        btnScaleEdit->setObjectName("btnScaleEdit");
        sizePolicy3.setHeightForWidth(btnScaleEdit->sizePolicy().hasHeightForWidth());
        btnScaleEdit->setSizePolicy(sizePolicy3);

        horizontalLayout_5->addWidget(btnScaleEdit);

        btnScaleAdd = new QPushButton(groupScale);
        btnScaleAdd->setObjectName("btnScaleAdd");
        sizePolicy3.setHeightForWidth(btnScaleAdd->sizePolicy().hasHeightForWidth());
        btnScaleAdd->setSizePolicy(sizePolicy3);
        btnScaleAdd->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_5->addWidget(btnScaleAdd);


        verticalLayout_14->addWidget(groupScale);

        groupTempo = new QGroupBox(compositionTab);
        groupTempo->setObjectName("groupTempo");
        horizontalLayout_Tempo = new QHBoxLayout(groupTempo);
        horizontalLayout_Tempo->setObjectName("horizontalLayout_Tempo");
        spinTempoMarker = new QSpinBox(groupTempo);
        spinTempoMarker->setObjectName("spinTempoMarker");
        sizePolicy3.setHeightForWidth(spinTempoMarker->sizePolicy().hasHeightForWidth());
        spinTempoMarker->setSizePolicy(sizePolicy3);
        spinTempoMarker->setMinimumSize(QSize(50, 0));
        spinTempoMarker->setMinimum(1);
        spinTempoMarker->setMaximum(1);
        spinTempoMarker->setValue(1);

        horizontalLayout_Tempo->addWidget(spinTempoMarker);

        labelTempoDisplay = new QLabel(groupTempo);
        labelTempoDisplay->setObjectName("labelTempoDisplay");
        sizePolicy4.setHeightForWidth(labelTempoDisplay->sizePolicy().hasHeightForWidth());
        labelTempoDisplay->setSizePolicy(sizePolicy4);

        horizontalLayout_Tempo->addWidget(labelTempoDisplay);

        btnTempoEdit = new QPushButton(groupTempo);
        btnTempoEdit->setObjectName("btnTempoEdit");
        sizePolicy3.setHeightForWidth(btnTempoEdit->sizePolicy().hasHeightForWidth());
        btnTempoEdit->setSizePolicy(sizePolicy3);

        horizontalLayout_Tempo->addWidget(btnTempoEdit);

        btnTempoAdd = new QPushButton(groupTempo);
        btnTempoAdd->setObjectName("btnTempoAdd");
        sizePolicy3.setHeightForWidth(btnTempoAdd->sizePolicy().hasHeightForWidth());
        btnTempoAdd->setSizePolicy(sizePolicy3);
        btnTempoAdd->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_Tempo->addWidget(btnTempoAdd);


        verticalLayout_14->addWidget(groupTempo);

        MainTab->addTab(compositionTab, QString());
        preferencesTab = new QWidget();
        preferencesTab->setObjectName("preferencesTab");
        verticalLayout_13 = new QVBoxLayout(preferencesTab);
        verticalLayout_13->setObjectName("verticalLayout_13");
        label_15 = new QLabel(preferencesTab);
        label_15->setObjectName("label_15");

        verticalLayout_13->addWidget(label_15);

        MainTab->addTab(preferencesTab, QString());

        verticalLayout_4->addWidget(MainTab);

        KalaMain->setCentralWidget(centralwidget);
        menubar = new QMenuBar(KalaMain);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1089, 18));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName("menuEdit");
        menuSound = new QMenu(menubar);
        menuSound->setObjectName("menuSound");
        menuAdd_Container = new QMenu(menuSound);
        menuAdd_Container->setObjectName("menuAdd_Container");
        menuEssential = new QMenu(menuAdd_Container);
        menuEssential->setObjectName("menuEssential");
        menuShaping = new QMenu(menuAdd_Container);
        menuShaping->setObjectName("menuShaping");
        menuModifiers = new QMenu(menuAdd_Container);
        menuModifiers->setObjectName("menuModifiers");
        menuCompose = new QMenu(menubar);
        menuCompose->setObjectName("menuCompose");
        menuView = new QMenu(menubar);
        menuView->setObjectName("menuView");
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName("menuHelp");
        KalaMain->setMenuBar(menubar);
        statusbar = new QStatusBar(KalaMain);
        statusbar->setObjectName("statusbar");
        KalaMain->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuSound->menuAction());
        menubar->addAction(menuCompose->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionNew_project_Ctrl_N);
        menuFile->addAction(actionOpen_ctrl_o);
        menuFile->addAction(actionSave_ctrl_S);
        menuFile->addAction(actionSave_AS_Ctrl_SHift_S);
        menuFile->addSeparator();
        menuFile->addAction(actionExport_Audio_Ctrl_E);
        menuFile->addSeparator();
        menuFile->addAction(actionExit_Alt_F4);
        menuEdit->addAction(actionUndo_Ctrl_Z);
        menuEdit->addAction(actionRedo_Ctrl_Y);
        menuEdit->addSeparator();
        menuEdit->addAction(actionCut_Ctrl_X);
        menuEdit->addAction(actionCopy_Ctrl_C);
        menuEdit->addAction(actionPaste_Ctrl_V);
        menuEdit->addAction(actionDelete_Del);
        menuEdit->addSeparator();
        menuEdit->addAction(actionSelect_All_ctrl_A);
        menuEdit->addAction(actionDeselect_Esc);
        menuSound->addAction(actionNew_Sounit);
        menuSound->addAction(actionLoad_Sounit);
        menuSound->addAction(actionSave_Sounit_As);
        menuSound->addSeparator();
        menuSound->addAction(menuAdd_Container->menuAction());
        menuSound->addAction(actionDelete_Container_2);
        menuSound->addSeparator();
        menuAdd_Container->addAction(menuEssential->menuAction());
        menuAdd_Container->addAction(menuShaping->menuAction());
        menuAdd_Container->addAction(menuModifiers->menuAction());
        menuAdd_Container->addAction(actionGate_Processor);
        menuEssential->addAction(actionHarmonic_Generator);
        menuEssential->addAction(actionSpectrum_To_Signal);
        menuShaping->addAction(actionRolloff_Processor);
        menuShaping->addAction(actionFormant_Body);
        menuShaping->addAction(actionBreth_Turbulance);
        menuShaping->addAction(actionNoise_Color_Filter);
        menuModifiers->addAction(actionPhysics_System);
        menuModifiers->addAction(actionEasing_Applicator);
        menuModifiers->addAction(actionEnvelope_Engine);
        menuModifiers->addAction(actionDrift_Engine);
        menuCompose->addAction(actionNew_track);
        menuCompose->addAction(actionDelete_Track);
        menuCompose->addSeparator();
        menuCompose->addAction(actionScale_Settings);
        menuCompose->addSeparator();
        menuCompose->addAction(actionAssign_Sounit_to_Track);
        menuView->addAction(actionCanvas_Window);
        menuView->addAction(actionFull_Screen_Canvas_F11);
        menuView->addSeparator();
        menuView->addAction(actionZoom_In_Ctrl);
        menuView->addAction(actionZoom_Out_Crl);
        menuView->addAction(actionZoom_to_fit_Ctrl_0);
        menuView->addSeparator();
        menuView->addAction(actionSound_Engine_Tab_Ctrl_1);
        menuView->addAction(actionCompisition_Tab_Ctrl_2);
        menuView->addAction(actionPreferences_Tab_Ctrl_3);
        menuView->addAction(actionTablet_mode);
        menuHelp->addAction(actionDocumentation_F1);
        menuHelp->addAction(actionKeyboard_Shortcuts);
        menuHelp->addSeparator();
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout_Kala);

        retranslateUi(KalaMain);

        MainTab->setCurrentIndex(0);
        comboConnectionFunction->setCurrentIndex(5);
        tabInspector->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(KalaMain);
    } // setupUi

    void retranslateUi(QMainWindow *KalaMain)
    {
        KalaMain->setWindowTitle(QCoreApplication::translate("KalaMain", "Kala", nullptr));
        actionNew_project_Ctrl_N->setText(QCoreApplication::translate("KalaMain", "New project (Ctrl + N)", nullptr));
        actionOpen_ctrl_o->setText(QCoreApplication::translate("KalaMain", "Open (ctrl +o)", nullptr));
        actionOpen_Recent->setText(QCoreApplication::translate("KalaMain", "Open Recent ", nullptr));
        actionSave_ctrl_S->setText(QCoreApplication::translate("KalaMain", "Save (ctrl + S)", nullptr));
        actionSave_AS_Ctrl_SHift_S->setText(QCoreApplication::translate("KalaMain", "Save AS (Ctrl+SHift+S)", nullptr));
        actionExport_Audio_Ctrl_E->setText(QCoreApplication::translate("KalaMain", "Export Audio (Ctrl + E)", nullptr));
        actionExit_Alt_F4->setText(QCoreApplication::translate("KalaMain", "Exit (Alt + F4)", nullptr));
        actionUndo_Ctrl_Z->setText(QCoreApplication::translate("KalaMain", "Undo (Ctrl+Z)", nullptr));
        actionRedo_Ctrl_Y->setText(QCoreApplication::translate("KalaMain", "Redo (Ctrl+Y)", nullptr));
        actionCut_Ctrl_X->setText(QCoreApplication::translate("KalaMain", "Cut (Ctrl +X)", nullptr));
        actionCopy_Ctrl_C->setText(QCoreApplication::translate("KalaMain", "Copy (Ctrl +C)", nullptr));
        actionPaste_Ctrl_V->setText(QCoreApplication::translate("KalaMain", "Paste (Ctrl + V)", nullptr));
        actionDuplicate_Ctrl_D->setText(QCoreApplication::translate("KalaMain", "Duplicate (Ctrl + D)", nullptr));
        actionDelete_Del->setText(QCoreApplication::translate("KalaMain", "Delete (Del)", nullptr));
        actionSelect_All_ctrl_A->setText(QCoreApplication::translate("KalaMain", "Select All (ctrl A)", nullptr));
        actionDeselect_Esc->setText(QCoreApplication::translate("KalaMain", "Deselect (Esc)", nullptr));
        actionNew_Sounit->setText(QCoreApplication::translate("KalaMain", "New Sounit", nullptr));
        actionLoad_Sounit->setText(QCoreApplication::translate("KalaMain", "Load Sounit", nullptr));
        actionSave_Sounit_As->setText(QCoreApplication::translate("KalaMain", "Save Sounit As", nullptr));
        actionHarmonic_Generator->setText(QCoreApplication::translate("KalaMain", "Harmonic Generator", nullptr));
        actionSpectrum_To_Signal->setText(QCoreApplication::translate("KalaMain", "Spectrum To Signal", nullptr));
        actionRolloff_Processor->setText(QCoreApplication::translate("KalaMain", "Rolloff Processor", nullptr));
        actionFormant_Body->setText(QCoreApplication::translate("KalaMain", "Formant Body", nullptr));
        actionBreth_Turbulance->setText(QCoreApplication::translate("KalaMain", "Breath Turbulence", nullptr));
        actionNoise_Color_Filter->setText(QCoreApplication::translate("KalaMain", "Noise Color Filter", nullptr));
        actionPhysics_System->setText(QCoreApplication::translate("KalaMain", "Physics System", nullptr));
        actionEasing_Applicator->setText(QCoreApplication::translate("KalaMain", "Easing Applicator", nullptr));
        actionEnvelope_Engine->setText(QCoreApplication::translate("KalaMain", "Envelope Engine", nullptr));
        actionDrift_Engine->setText(QCoreApplication::translate("KalaMain", "Drift Engine", nullptr));
        actionGate_Processor->setText(QCoreApplication::translate("KalaMain", "Gate Processor", nullptr));
        actionDelete_Container->setText(QCoreApplication::translate("KalaMain", "Delete Container", nullptr));
        actionDelete_Container_2->setText(QCoreApplication::translate("KalaMain", "Delete Container", nullptr));
        actionDNA_Library->setText(QCoreApplication::translate("KalaMain", "DNA Library", nullptr));
        actionEnvelope_Library->setText(QCoreApplication::translate("KalaMain", "Envelope Library", nullptr));
        actionEasing_Library->setText(QCoreApplication::translate("KalaMain", "Easing Library", nullptr));
        actionNew_track->setText(QCoreApplication::translate("KalaMain", "New track", nullptr));
        actionDelete_Track->setText(QCoreApplication::translate("KalaMain", "Delete Track", nullptr));
        actionScale_Settings->setText(QCoreApplication::translate("KalaMain", "Scale Settings", nullptr));
        actionAssign_Sounit_to_Track->setText(QCoreApplication::translate("KalaMain", "Assign Sounit to Track", nullptr));
        actionCanvas_Window->setText(QCoreApplication::translate("KalaMain", "Canvas Window", nullptr));
        actionFull_Screen_Canvas_F11->setText(QCoreApplication::translate("KalaMain", "Full Screen Canvas (F11)", nullptr));
        actionZoom_In_Ctrl->setText(QCoreApplication::translate("KalaMain", "Zoom In (Ctrl ++)", nullptr));
        actionZoom_Out_Crl->setText(QCoreApplication::translate("KalaMain", "Zoom Out (Crl +-)", nullptr));
        actionZoom_to_fit_Ctrl_0->setText(QCoreApplication::translate("KalaMain", "Zoom to fit (Ctrl + 0)", nullptr));
        actionSound_Engine_Tab_Ctrl_1->setText(QCoreApplication::translate("KalaMain", "Sound Engine Tab (Ctrl + 1)", nullptr));
        actionCompisition_Tab_Ctrl_2->setText(QCoreApplication::translate("KalaMain", "Composition Tab (Ctrl + 2)", nullptr));
        actionPreferences_Tab_Ctrl_3->setText(QCoreApplication::translate("KalaMain", "Settings Tab (Ctrl + 3)", nullptr));
        actionDocumentation_F1->setText(QCoreApplication::translate("KalaMain", "Documentation (F1)", nullptr));
        actionKeyboard_Shortcuts->setText(QCoreApplication::translate("KalaMain", "Keyboard Shortcuts", nullptr));
        actionAbout_Kala->setText(QCoreApplication::translate("KalaMain", "About Kala", nullptr));
        actionComposition_Settings->setText(QCoreApplication::translate("KalaMain", "Composition Settings", nullptr));
        actionTablet_mode->setText(QCoreApplication::translate("KalaMain", "Tablet mode", nullptr));
        groupSounitInfo->setTitle(QCoreApplication::translate("KalaMain", "Sounit Info", nullptr));
        label->setText(QCoreApplication::translate("KalaMain", "Edit Sounit:", nullptr));
        labelSounitName->setText(QCoreApplication::translate("KalaMain", "Name:", nullptr));
        labelVariation->setText(QCoreApplication::translate("KalaMain", "Variation:", nullptr));
        groupComment->setTitle(QCoreApplication::translate("KalaMain", "Comment", nullptr));
        editSounitComment->setPlaceholderText(QCoreApplication::translate("KalaMain", "Add a comment...", nullptr));
        groupContainerInspector->setTitle(QCoreApplication::translate("KalaMain", "Container inspector", nullptr));
        label_5->setText(QCoreApplication::translate("KalaMain", "Instance: ", nullptr));
        label_4->setText(QCoreApplication::translate("KalaMain", "Type: ", nullptr));
        labelContainerType->setText(QCoreApplication::translate("KalaMain", "TextLabel", nullptr));
        labelContainerDescription->setText(QString());
        groupInputs->setTitle(QCoreApplication::translate("KalaMain", "Inputs", nullptr));
        groupOutputs->setTitle(QCoreApplication::translate("KalaMain", "Outputs", nullptr));
        groupConfig->setTitle(QCoreApplication::translate("KalaMain", "Config", nullptr));
        groupConnectionInspector->setTitle(QCoreApplication::translate("KalaMain", "Connection Inspector", nullptr));
        label_7->setText(QCoreApplication::translate("KalaMain", "To:", nullptr));
        labelConnectionTo->setText(QCoreApplication::translate("KalaMain", "TextLabel", nullptr));
        label_8->setText(QCoreApplication::translate("KalaMain", "Function:", nullptr));
        comboConnectionFunction->setItemText(0, QCoreApplication::translate("KalaMain", "passthrough", nullptr));
        comboConnectionFunction->setItemText(1, QCoreApplication::translate("KalaMain", "add", nullptr));
        comboConnectionFunction->setItemText(2, QCoreApplication::translate("KalaMain", "multiply", nullptr));
        comboConnectionFunction->setItemText(3, QCoreApplication::translate("KalaMain", "substract", nullptr));
        comboConnectionFunction->setItemText(4, QCoreApplication::translate("KalaMain", "replace", nullptr));
        comboConnectionFunction->setItemText(5, QCoreApplication::translate("KalaMain", "modulate", nullptr));

        comboConnectionFunction->setCurrentText(QCoreApplication::translate("KalaMain", "modulate", nullptr));
        label_9->setText(QCoreApplication::translate("KalaMain", "Weight:", nullptr));
        buttonDisconnect->setText(QCoreApplication::translate("KalaMain", "Disconnect", nullptr));
        labelConnectionFrom->setText(QCoreApplication::translate("KalaMain", "TextLabel", nullptr));
        label_6->setText(QCoreApplication::translate("KalaMain", "From:", nullptr));
        MainTab->setTabText(MainTab->indexOf(soundEngineTab), QCoreApplication::translate("KalaMain", "Sound Engine", nullptr));
        groupInspector->setTitle(QCoreApplication::translate("KalaMain", "Inspector", nullptr));
        positiongroupBox->setTitle(QCoreApplication::translate("KalaMain", "Position", nullptr));
        label_11->setText(QCoreApplication::translate("KalaMain", "Start:", nullptr));
        label_14->setText(QCoreApplication::translate("KalaMain", "Duration:", nullptr));
        label_16->setText(QCoreApplication::translate("KalaMain", "Pitch:", nullptr));
        noteVariationLabel->setText(QCoreApplication::translate("KalaMain", "Variation", nullptr));
        vibratoGroupBox->setTitle(QCoreApplication::translate("KalaMain", "Vibrato", nullptr));
        checkNoteVibrato->setText(QCoreApplication::translate("KalaMain", "Vibrato", nullptr));
        btnNoteVibratoEdit->setText(QCoreApplication::translate("KalaMain", "Edit", nullptr));
        expressiveCurvesGroupBox->setTitle(QCoreApplication::translate("KalaMain", "Expressive Curves", nullptr));
        btnExpressiveCurveAdd->setText(QCoreApplication::translate("KalaMain", "Add", nullptr));
        btnExpressiveCurveDelete->setText(QCoreApplication::translate("KalaMain", "Delete", nullptr));
        btnExpressiveCurveApply->setText(QCoreApplication::translate("KalaMain", "Apply", nullptr));
        tabInspector->setTabText(tabInspector->indexOf(tabNoteInspector), QCoreApplication::translate("KalaMain", "Note", nullptr));
        groupVariations->setTitle(QCoreApplication::translate("KalaMain", "Sounit Variations", nullptr));
#if QT_CONFIG(tooltip)
        editVariationsTrackName->setToolTip(QCoreApplication::translate("KalaMain", "Track name \342\200\224 edit to rename", nullptr));
#endif // QT_CONFIG(tooltip)
        editVariationsTrackName->setPlaceholderText(QCoreApplication::translate("KalaMain", "(no track selected)", nullptr));
        btnDeleteVariation->setText(QCoreApplication::translate("KalaMain", "Delete", nullptr));
        btnRenameVariation->setText(QCoreApplication::translate("KalaMain", "Rename", nullptr));
        groupTracks->setTitle(QCoreApplication::translate("KalaMain", "Tracks / Mixer", nullptr));
        btnAddTrack->setText(QCoreApplication::translate("KalaMain", "Add Track", nullptr));
        groupScale->setTitle(QCoreApplication::translate("KalaMain", "Scale", nullptr));
        labelScaleDisplay->setText(QCoreApplication::translate("KalaMain", "0:00:000 | Just Intonation | 25.00 Hz", nullptr));
        btnScaleEdit->setText(QCoreApplication::translate("KalaMain", "Edit", nullptr));
        btnScaleAdd->setText(QCoreApplication::translate("KalaMain", "+", nullptr));
        groupTempo->setTitle(QCoreApplication::translate("KalaMain", "Tempo", nullptr));
        labelTempoDisplay->setText(QCoreApplication::translate("KalaMain", "0:00:000 | 120 BPM | 4/4", nullptr));
        btnTempoEdit->setText(QCoreApplication::translate("KalaMain", "Edit", nullptr));
        btnTempoAdd->setText(QCoreApplication::translate("KalaMain", "+", nullptr));
        MainTab->setTabText(MainTab->indexOf(compositionTab), QCoreApplication::translate("KalaMain", "Score", nullptr));
        label_15->setText(QCoreApplication::translate("KalaMain", "TextLabel", nullptr));
        MainTab->setTabText(MainTab->indexOf(preferencesTab), QCoreApplication::translate("KalaMain", "Sound Engine Settings", nullptr));
        menuFile->setTitle(QCoreApplication::translate("KalaMain", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("KalaMain", "Edit", nullptr));
        menuSound->setTitle(QCoreApplication::translate("KalaMain", "Sound", nullptr));
        menuAdd_Container->setTitle(QCoreApplication::translate("KalaMain", "Add Container", nullptr));
        menuEssential->setTitle(QCoreApplication::translate("KalaMain", "Essential", nullptr));
        menuShaping->setTitle(QCoreApplication::translate("KalaMain", "Shaping", nullptr));
        menuModifiers->setTitle(QCoreApplication::translate("KalaMain", "Modifiers", nullptr));
        menuCompose->setTitle(QCoreApplication::translate("KalaMain", "Score", nullptr));
        menuView->setTitle(QCoreApplication::translate("KalaMain", "View", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("KalaMain", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class KalaMain: public Ui_KalaMain {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KALAMAIN_H

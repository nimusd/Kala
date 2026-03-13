/********************************************************************************
** Form generated from reading UI file 'scorecanvas.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCORECANVAS_H
#define UI_SCORECANVAS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_scorecanvas
{
public:
    QAction *actionDrawDiscreteNotes;
    QAction *actionDrawContinuousNotes;
    QAction *actionRecordDiscreteNotes;
    QAction *actionRecordContinuousNotes;
    QAction *actionRecordPhrase;
    QAction *actionScoreCanvasSelect;
    QAction *actionScoreCanvasZoom;
    QAction *actionPlay;
    QAction *actionstop;
    QAction *actionCompositionSettings;
    QAction *actionSnapToScale;
    QAction *actionAddTrack;
    QWidget *centralwidget;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayoutTimeline;
    QScrollArea *scrollAreatracks;
    QWidget *scrollAreaWidgetContents;
    QToolBar *scoreCanvasBottomToolbar;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuTrack;
    QToolBar *toolBar;

    void setupUi(QMainWindow *scorecanvas)
    {
        if (scorecanvas->objectName().isEmpty())
            scorecanvas->setObjectName("scorecanvas");
        scorecanvas->resize(1171, 930);
        actionDrawDiscreteNotes = new QAction(scorecanvas);
        actionDrawDiscreteNotes->setObjectName("actionDrawDiscreteNotes");
        actionDrawDiscreteNotes->setMenuRole(QAction::MenuRole::NoRole);
        actionDrawContinuousNotes = new QAction(scorecanvas);
        actionDrawContinuousNotes->setObjectName("actionDrawContinuousNotes");
        actionDrawContinuousNotes->setMenuRole(QAction::MenuRole::NoRole);
        actionRecordDiscreteNotes = new QAction(scorecanvas);
        actionRecordDiscreteNotes->setObjectName("actionRecordDiscreteNotes");
        actionRecordDiscreteNotes->setMenuRole(QAction::MenuRole::NoRole);
        actionRecordContinuousNotes = new QAction(scorecanvas);
        actionRecordContinuousNotes->setObjectName("actionRecordContinuousNotes");
        actionRecordContinuousNotes->setMenuRole(QAction::MenuRole::NoRole);
        actionRecordPhrase = new QAction(scorecanvas);
        actionRecordPhrase->setObjectName("actionRecordPhrase");
        actionRecordPhrase->setMenuRole(QAction::MenuRole::NoRole);
        actionScoreCanvasSelect = new QAction(scorecanvas);
        actionScoreCanvasSelect->setObjectName("actionScoreCanvasSelect");
        actionScoreCanvasSelect->setMenuRole(QAction::MenuRole::NoRole);
        actionScoreCanvasZoom = new QAction(scorecanvas);
        actionScoreCanvasZoom->setObjectName("actionScoreCanvasZoom");
        actionScoreCanvasZoom->setMenuRole(QAction::MenuRole::NoRole);
        actionPlay = new QAction(scorecanvas);
        actionPlay->setObjectName("actionPlay");
        actionPlay->setMenuRole(QAction::MenuRole::NoRole);
        actionstop = new QAction(scorecanvas);
        actionstop->setObjectName("actionstop");
        actionstop->setMenuRole(QAction::MenuRole::NoRole);
        actionCompositionSettings = new QAction(scorecanvas);
        actionCompositionSettings->setObjectName("actionCompositionSettings");
        actionCompositionSettings->setMenuRole(QAction::MenuRole::NoRole);
        actionSnapToScale = new QAction(scorecanvas);
        actionSnapToScale->setObjectName("actionSnapToScale");
        actionSnapToScale->setMenuRole(QAction::MenuRole::NoRole);
        actionAddTrack = new QAction(scorecanvas);
        actionAddTrack->setObjectName("actionAddTrack");
        actionAddTrack->setMenuRole(QAction::MenuRole::NoRole);
        centralwidget = new QWidget(scorecanvas);
        centralwidget->setObjectName("centralwidget");
        centralwidget->setStyleSheet(QString::fromUtf8("background-color: white;"));
        horizontalLayoutWidget = new QWidget(centralwidget);
        horizontalLayoutWidget->setObjectName("horizontalLayoutWidget");
        horizontalLayoutWidget->setGeometry(QRect(49, -1, 751, 61));
        horizontalLayoutTimeline = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayoutTimeline->setObjectName("horizontalLayoutTimeline");
        horizontalLayoutTimeline->setContentsMargins(0, 0, 0, 0);
        scrollAreatracks = new QScrollArea(centralwidget);
        scrollAreatracks->setObjectName("scrollAreatracks");
        scrollAreatracks->setGeometry(QRect(0, -1, 51, 551));
        scrollAreatracks->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName("scrollAreaWidgetContents");
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 49, 549));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(scrollAreaWidgetContents->sizePolicy().hasHeightForWidth());
        scrollAreaWidgetContents->setSizePolicy(sizePolicy);
        scrollAreatracks->setWidget(scrollAreaWidgetContents);
        scorecanvas->setCentralWidget(centralwidget);
        scoreCanvasBottomToolbar = new QToolBar(scorecanvas);
        scoreCanvasBottomToolbar->setObjectName("scoreCanvasBottomToolbar");
        scoreCanvasBottomToolbar->setMinimumSize(QSize(0, 30));
        scoreCanvasBottomToolbar->setMaximumSize(QSize(16777215, 30));
        scorecanvas->addToolBar(Qt::ToolBarArea::BottomToolBarArea, scoreCanvasBottomToolbar);
        menubar = new QMenuBar(scorecanvas);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1171, 18));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuTrack = new QMenu(menubar);
        menuTrack->setObjectName("menuTrack");
        scorecanvas->setMenuBar(menubar);
        toolBar = new QToolBar(scorecanvas);
        toolBar->setObjectName("toolBar");
        scorecanvas->addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);

        scoreCanvasBottomToolbar->addAction(actionPlay);
        scoreCanvasBottomToolbar->addAction(actionstop);
        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuTrack->menuAction());
        menuFile->addAction(actionCompositionSettings);
        menuTrack->addAction(actionAddTrack);
        toolBar->addAction(actionDrawDiscreteNotes);
        toolBar->addAction(actionDrawContinuousNotes);
        toolBar->addSeparator();
        toolBar->addSeparator();
        toolBar->addAction(actionScoreCanvasSelect);
        toolBar->addAction(actionScoreCanvasZoom);
        toolBar->addSeparator();
        toolBar->addAction(actionSnapToScale);

        retranslateUi(scorecanvas);

        QMetaObject::connectSlotsByName(scorecanvas);
    } // setupUi

    void retranslateUi(QMainWindow *scorecanvas)
    {
        scorecanvas->setWindowTitle(QCoreApplication::translate("scorecanvas", "scoreCanvas", nullptr));
        actionDrawDiscreteNotes->setText(QCoreApplication::translate("scorecanvas", "Draw Discrete Notes", nullptr));
        actionDrawContinuousNotes->setText(QCoreApplication::translate("scorecanvas", "Draw Continuous Notes", nullptr));
        actionRecordDiscreteNotes->setText(QCoreApplication::translate("scorecanvas", "Record Discrete Notes", nullptr));
        actionRecordContinuousNotes->setText(QCoreApplication::translate("scorecanvas", "Record Continuous Notes", nullptr));
        actionRecordPhrase->setText(QCoreApplication::translate("scorecanvas", "Record Phrase", nullptr));
        actionScoreCanvasSelect->setText(QCoreApplication::translate("scorecanvas", "Select", nullptr));
        actionScoreCanvasZoom->setText(QCoreApplication::translate("scorecanvas", "Zoom", nullptr));
        actionPlay->setText(QCoreApplication::translate("scorecanvas", "\342\226\266", nullptr));
        actionstop->setText(QCoreApplication::translate("scorecanvas", "\342\226\240", nullptr));
        actionCompositionSettings->setText(QCoreApplication::translate("scorecanvas", "Composition Settings...", nullptr));
#if QT_CONFIG(shortcut)
        actionCompositionSettings->setShortcut(QCoreApplication::translate("scorecanvas", "Ctrl+,", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSnapToScale->setText(QCoreApplication::translate("scorecanvas", "Snap to Scale", nullptr));
#if QT_CONFIG(tooltip)
        actionSnapToScale->setToolTip(QCoreApplication::translate("scorecanvas", "Quantize selected continuous note to scale degrees", nullptr));
#endif // QT_CONFIG(tooltip)
        actionAddTrack->setText(QCoreApplication::translate("scorecanvas", "Add Track...", nullptr));
#if QT_CONFIG(tooltip)
        actionAddTrack->setToolTip(QCoreApplication::translate("scorecanvas", "Add a new track to the composition", nullptr));
#endif // QT_CONFIG(tooltip)
        scoreCanvasBottomToolbar->setWindowTitle(QCoreApplication::translate("scorecanvas", "toolBar_2", nullptr));
        menuFile->setTitle(QCoreApplication::translate("scorecanvas", "Settings", nullptr));
        menuTrack->setTitle(QCoreApplication::translate("scorecanvas", "Track", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("scorecanvas", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class scorecanvas: public Ui_scorecanvas {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCORECANVAS_H

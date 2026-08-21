/********************************************************************************
** Form generated from reading UI file 'dialogNewProject.ui'
**
** Created by: Qt User Interface Compiler version 6.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGNEWPROJECT_H
#define UI_DIALOGNEWPROJECT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_DialogNewProject
{
public:
    QDialogButtonBox *buttonBox;
    QLabel *label;
    QLineEdit *lineEditProjectName;

    void setupUi(QDialog *DialogNewProject)
    {
        if (DialogNewProject->objectName().isEmpty())
            DialogNewProject->setObjectName("DialogNewProject");
        DialogNewProject->resize(400, 300);
        DialogNewProject->setStyleSheet(QString::fromUtf8("background-color = rgb(220, 232, 214)"));
        DialogNewProject->setModal(true);
        buttonBox = new QDialogButtonBox(DialogNewProject);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setGeometry(QRect(30, 240, 341, 32));
        buttonBox->setStyleSheet(QString::fromUtf8(""));
        buttonBox->setOrientation(Qt::Orientation::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Cancel|QDialogButtonBox::StandardButton::Ok);
        label = new QLabel(DialogNewProject);
        label->setObjectName("label");
        label->setGeometry(QRect(40, 46, 61, 16));
        lineEditProjectName = new QLineEdit(DialogNewProject);
        lineEditProjectName->setObjectName("lineEditProjectName");
        lineEditProjectName->setGeometry(QRect(100, 50, 201, 20));

        retranslateUi(DialogNewProject);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, DialogNewProject, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, DialogNewProject, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(DialogNewProject);
    } // setupUi

    void retranslateUi(QDialog *DialogNewProject)
    {
        DialogNewProject->setWindowTitle(QCoreApplication::translate("DialogNewProject", "New project", nullptr));
        label->setText(QCoreApplication::translate("DialogNewProject", "project name:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogNewProject: public Ui_DialogNewProject {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGNEWPROJECT_H

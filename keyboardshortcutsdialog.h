#ifndef KEYBOARDSHORTCUTSDIALOG_H
#define KEYBOARDSHORTCUTSDIALOG_H

#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;

class KeyboardShortcutsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeyboardShortcutsDialog(QWidget *parent = nullptr);

private:
    void setupUI();
    QTreeWidgetItem *addCategory(QTreeWidget *tree, const QString &name);
    void addShortcut(QTreeWidgetItem *category, const QString &action, const QString &shortcut);
};

#endif // KEYBOARDSHORTCUTSDIALOG_H

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QMap>

class QSplitter;
class QTreeWidget;
class QTreeWidgetItem;
class QTextBrowser;
class QLineEdit;
class QPushButton;

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget *parent = nullptr);

    void navigateTo(const QString &htmlFile, const QString &anchor = QString());
    void navigateToContainer(const QString &containerName);

    static QString helpPath();

private:
    struct PageRef {
        QString file;
        QString anchor;
    };

    void setupUI();
    void buildTableOfContents();
    QTreeWidgetItem *addTopLevel(const QString &label, const QString &file, const QString &anchor = QString());
    QTreeWidgetItem *addChild(QTreeWidgetItem *parent, const QString &label, const QString &file, const QString &anchor);
    QTreeWidgetItem *addCategory(const QString &label);
    void highlightTocItem(const QString &file, const QString &anchor);

    static QMap<QString, QString> containerToFileMap();

    QSplitter *m_splitter;
    QTreeWidget *m_tocTree;
    QTextBrowser *m_browser;
    QLineEdit *m_searchLine;
    QPushButton *m_searchNext;
    QPushButton *m_searchPrev;

    QMap<QTreeWidgetItem*, PageRef> m_tocMap;
    QString m_currentFile;

private slots:
    void onTocItemClicked(QTreeWidgetItem *item, int column);
    void onAnchorClicked(const QUrl &url);
    void onSearchNext();
    void onSearchPrev();
    void onSearchReturnPressed();
};

#endif // HELPDIALOG_H

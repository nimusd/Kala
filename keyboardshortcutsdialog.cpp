#include "keyboardshortcutsdialog.h"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QFont>

KeyboardShortcutsDialog::KeyboardShortcutsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("Keyboard Shortcuts");
    resize(500, 600);
}

void KeyboardShortcutsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QTreeWidget *tree = new QTreeWidget(this);
    tree->setColumnCount(2);
    tree->setHeaderLabels({"Action", "Shortcut"});
    tree->setRootIsDecorated(false);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->setFocusPolicy(Qt::NoFocus);
    tree->header()->setStretchLastSection(false);
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    // File
    QTreeWidgetItem *file = addCategory(tree, "File");
    addShortcut(file, "New Project", "Ctrl+N");
    addShortcut(file, "Open Project", "Ctrl+O");
    addShortcut(file, "Save Project", "Ctrl+S");
    addShortcut(file, "Save As", "Ctrl+Shift+S");
    addShortcut(file, "Export Audio", "Ctrl+E");
    addShortcut(file, "Exit", "Alt+F4");

    // Edit
    QTreeWidgetItem *edit = addCategory(tree, "Edit");
    addShortcut(edit, "Undo", "Ctrl+Z");
    addShortcut(edit, "Redo", "Ctrl+Y");
    addShortcut(edit, "Cut", "Ctrl+X");
    addShortcut(edit, "Copy", "Ctrl+C");
    addShortcut(edit, "Paste", "Ctrl+V");
    addShortcut(edit, "Delete", "Delete");
    addShortcut(edit, "Select All", "Ctrl+A");
    addShortcut(edit, "Deselect", "Escape");

    // View
    QTreeWidgetItem *view = addCategory(tree, "View");
    addShortcut(view, "Full Screen Canvas", "F11");
    addShortcut(view, "Zoom In", "Ctrl+=");
    addShortcut(view, "Zoom Out", "Ctrl+-");
    addShortcut(view, "Zoom to Fit", "Ctrl+0");
    addShortcut(view, "Sound Engine Tab", "Ctrl+1");
    addShortcut(view, "Composition Tab", "Ctrl+2");
    addShortcut(view, "Preferences Tab", "Ctrl+3");
    addShortcut(view, "Toggle Canvas Focus", "Tab");

    // Sound Engine
    QTreeWidgetItem *soundEngine = addCategory(tree, "Sound Engine");
    addShortcut(soundEngine, "Play/Stop", "Space");
    addShortcut(soundEngine, "Play Test Tone", "T");
    addShortcut(soundEngine, "Stop Playback", "S");
    addShortcut(soundEngine, "Toggle Zoom Mode", "Z");
    addShortcut(soundEngine, "Reset Zoom", "R");
    addShortcut(soundEngine, "Toggle Pan Mode", "Ctrl+Space");

    // Composition
    QTreeWidgetItem *composition = addCategory(tree, "Composition");
    addShortcut(composition, "Select Tool", "S");
    addShortcut(composition, "Draw Discrete Notes", "D");
    addShortcut(composition, "Draw Continuous Notes", "C");
    addShortcut(composition, "Toggle Zoom Mode", "Z");
    addShortcut(composition, "Play/Stop", "Space");
    addShortcut(composition, "Toggle Pan Mode", "Ctrl+Space");
    addShortcut(composition, "Set Loop", "L");
    addShortcut(composition, "Go To Position", "G");
    addShortcut(composition, "Clear Loop", "Delete (with loop selected)");

    // Score Canvas Editing
    QTreeWidgetItem *scoreCanvas = addCategory(tree, "Score Canvas Editing");
    addShortcut(scoreCanvas, "Enter Segment Edit", "Enter");
    addShortcut(scoreCanvas, "Exit Segment Edit", "Escape");
    addShortcut(scoreCanvas, "Previous Segment", "Left Arrow");
    addShortcut(scoreCanvas, "Next Segment", "Right Arrow");
    addShortcut(scoreCanvas, "Context Menu", "Menu / Shift+F10");

    // Spectrum Visualizer
    QTreeWidgetItem *spectrumVis = addCategory(tree, "Spectrum Visualizer");
    addShortcut(spectrumVis, "Remove Selected Harmonic", "Delete");

    // Composition Settings
    QTreeWidgetItem *compSettings = addCategory(tree, "Composition Settings");
    addShortcut(compSettings, "Open Composition Settings", "Ctrl+,");

    // Curve Editing
    QTreeWidgetItem *curveEditing = addCategory(tree, "Curve Editing (Envelope & Vibrato)");
    addShortcut(curveEditing, "Snap to Grid", "Shift+Drag");

    tree->expandAll();

    mainLayout->addWidget(tree);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
}

QTreeWidgetItem *KeyboardShortcutsDialog::addCategory(QTreeWidget *tree, const QString &name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tree);
    item->setText(0, name);
    item->setFlags(Qt::ItemIsEnabled);
    QFont bold = item->font(0);
    bold.setBold(true);
    item->setFont(0, bold);
    return item;
}

void KeyboardShortcutsDialog::addShortcut(QTreeWidgetItem *category, const QString &action, const QString &shortcut)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(category);
    item->setText(0, action);
    item->setText(1, shortcut);
    item->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
}

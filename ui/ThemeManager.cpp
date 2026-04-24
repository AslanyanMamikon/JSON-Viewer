#include "ThemeManager.h"
#include "mainwindow.h"  // provides the full CodeEditor definition

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {}

void ThemeManager::toggle() {
    setDark(!dark);
}

void ThemeManager::setDark(bool enabled) {
    dark = enabled;
    if (dark) applyDark(); else applyLight();
    if (themeButton) themeButton->setText(dark ? "☀️" : "🌙");
    if (codeEditor)  codeEditor->applyTheme(dark);
    emit themeChanged(dark);
}

// ---------------------------------------------------------------------------
void ThemeManager::applyDark() {
    if (mainWindow) {
        mainWindow->setStyleSheet(
            "QMainWindow { background-color: #1e1e1e; }"
            "QMenuBar { background-color: #2d2d30; color: #cccccc;"
            "  border-bottom: 1px solid #3e3e42; }"
            "QMenuBar::item:selected { background-color: #3e3e42; }"
            "QMenu { background-color: #2d2d30; color: #cccccc;"
            "  border: 1px solid #3e3e42; }"
            "QMenu::item:selected { background-color: #094771; }"
            "QToolBar { background-color: #2d2d30; border-bottom: 1px solid #3e3e42;"
            "  spacing: 5px; padding: 5px; }"
            "QToolBar QLabel  { color: #cccccc; }"
            "QToolBar QComboBox { background-color: #3c3c3c; color: #cccccc;"
            "  border: 1px solid #3e3e42; padding: 4px 8px; border-radius: 2px; }"
            "QToolBar QComboBox:hover { background-color: #505050; }"
            "QToolBar QComboBox::drop-down { border: none; }"
            "QToolBar QComboBox QAbstractItemView { background-color: #2d2d30;"
            "  color: #cccccc; selection-background-color: #094771; }"
            "QToolBar QLineEdit { background-color: #3c3c3c; color: #cccccc;"
            "  border: 1px solid #3e3e42; padding: 4px 6px; border-radius: 2px; }"
            "QToolBar QPushButton { background-color: #3c3c3c; color: #cccccc;"
            "  border: 1px solid #3e3e42; border-radius: 3px; padding: 5px; }"
            "QToolBar QPushButton:hover { background-color: #505050; }"
            "QStatusBar { background-color: #007acc; color: #ffffff; }"
            );
    }

    if (treeView) {
        treeView->setStyleSheet(
            "QTreeWidget { background-color: #252526; color: #cccccc; border: none;"
            "  alternate-background-color: #2d2d30;"
            "  selection-background-color: #094771; selection-color: #ffffff; }"
            "QTreeWidget::item { padding: 3px; border: none; }"
            "QTreeWidget::item:hover { background-color: #2a2d2e; }"
            "QTreeWidget::item:selected { background-color: #094771; }"
            "QHeaderView::section { background-color: #2d2d30; color: #cccccc;"
            "  padding: 5px; border: none; border-right: 1px solid #3e3e42;"
            "  border-bottom: 1px solid #3e3e42; }"
            "QScrollBar:vertical { background-color: #1e1e1e; width: 14px; border: none; }"
            "QScrollBar::handle:vertical { background-color: #424242; min-height: 20px;"
            "  border-radius: 7px; }"
            "QScrollBar::handle:vertical:hover { background-color: #4e4e4e; }"
            );
    }

    if (statusLabel)
        statusLabel->setStyleSheet("QLabel { color: #ffffff; padding: 2px 5px; }");
}

// ---------------------------------------------------------------------------
void ThemeManager::applyLight() {
    if (mainWindow) {
        mainWindow->setStyleSheet(
            "QMainWindow { background-color: #e1e1e1; }"
            "QMenuBar { background-color: #d2d2cf; color: #333333;"
            "  border-bottom: 1px solid #c1c1bd; }"
            "QMenuBar::item:selected { background-color: #c1c1bd; }"
            "QMenu { background-color: #d2d2cf; color: #333333;"
            "  border: 1px solid #c1c1bd; }"
            "QMenu::item:selected { background-color: #094771; }"
            "QToolBar { background-color: #d2d2cf; border-bottom: 1px solid #c1c1bd;"
            "  spacing: 5px; padding: 5px; }"
            "QToolBar QLabel { color: #000000; }"
            "QToolBar QToolButton { color: #000000; }"
            "QToolBar QComboBox { background-color: #c3c3c3; color: #333333;"
            "  border: 1px solid #c1c1bd; padding: 4px 8px; border-radius: 2px; }"
            "QToolBar QComboBox:hover { background-color: #b0b0b0; }"
            "QToolBar QComboBox::drop-down { border: none; }"
            "QToolBar QComboBox QAbstractItemView { background-color: #d2d2cf;"
            "  color: #333333; selection-background-color: #f6b88e; }"
            "QToolBar QLineEdit { background-color: #c3c3c3; color: #333333;"
            "  border: 1px solid #c1c1bd; padding: 4px 6px; border-radius: 2px; }"
            "QToolBar QPushButton { background-color: #c3c3c3; color: #333333;"
            "  border: 1px solid #c1c1bd; border-radius: 3px; padding: 5px; }"
            "QToolBar QPushButton:hover { background-color: #b0b0b0; }"
            "QStatusBar { background-color: #007acc; color: #ffffff; }"
            );
    }

    if (treeView) {
        treeView->setStyleSheet(
            "QTreeWidget { background-color: #d4d4d4; color: #333333; border: none;"
            "  alternate-background-color: #c2c2c2;"
            "  selection-background-color: #f6b88e; selection-color: #000000; }"
            "QTreeWidget::item { padding: 3px; border: none; }"
            "QTreeWidget::item:hover { background-color: #d5d2d1; }"
            "QTreeWidget::item:selected { background-color: #f6b88e; }"
            "QHeaderView::section { background-color: #d2d2cf; color: #333333;"
            "  padding: 5px; border: none; border-right: 1px solid #c1c1bd;"
            "  border-bottom: 1px solid #c1c1bd; }"
            "QScrollBar:vertical { background-color: #e1e1e1; width: 14px; border: none; }"
            "QScrollBar::handle:vertical { background-color: #bdbdbd; min-height: 20px;"
            "  border-radius: 7px; }"
            "QScrollBar::handle:vertical:hover { background-color: #b1b1b1; }"
            );
    }

    if (statusLabel)
        statusLabel->setStyleSheet("QLabel { color: #000000; padding: 2px 5px; }");
}

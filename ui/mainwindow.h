#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QSplitter>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollBar>
#include <QPaintEvent>
#include <QResizeEvent>
#include <memory>

#include "JSONController.h"
#include "SearchController.h"
#include "TreeViewController.h"
// NOTE: ThemeManager.h is included AFTER CodeEditor is defined below,
// because ThemeManager forward-declares CodeEditor and mainwindow.cpp
// needs the full type when calling setCodeEditor().

class LineNumberArea;

// ---------------------------------------------------------------------------
// CodeEditor – QTextEdit wrapped with a line-number gutter.
// ---------------------------------------------------------------------------
class CodeEditor : public QWidget {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget* parent = nullptr);

    QTextEdit* textEdit() const { return m_textEdit; }

    int  lineNumberAreaWidth();
    void applyTheme(bool isDark);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void lineNumberAreaPaintEvent(QPaintEvent* event);

    friend class LineNumberArea;

private slots:
    void updateLineNumberAreaWidth(int);

private:
    QTextEdit*      m_textEdit;
    LineNumberArea* m_lineNumberArea;
    bool            m_isDarkMode = false;

    void setupLineNumberArea();
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor* editor)
        : QWidget(editor), m_codeEditor(editor) {}

    QSize sizeHint() const override {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor* m_codeEditor;
    friend class CodeEditor;
};

// ThemeManager needs CodeEditor to be a complete type (stores CodeEditor*
// and calls applyTheme on it), so we include it here after the definition.
#include "ThemeManager.h"

// ---------------------------------------------------------------------------
// MainWindow – coordinates the application; delegates to sub-controllers.
// ---------------------------------------------------------------------------
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // File
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    // Edit
    void onFormatJSON();
    void onValidateJSON();
    void onParserModeChanged(int index);
    void onTextChanged();
    // UI
    void updateCursorPosition();

private:
    // ---- Widgets ------------------------------------------------------------
    CodeEditor*  textEditor      = nullptr;
    QTreeWidget* treeView        = nullptr;
    QComboBox*   parserModeCombo = nullptr;
    QLabel*      statusLabel     = nullptr;
    QLabel*      cursorPosLabel  = nullptr;
    QLineEdit*   searchEdit      = nullptr;
    QSplitter*   mainSplitter    = nullptr;
    QPushButton* themeToggleBtn  = nullptr;

    // ---- Sub-controllers (Fix 5) --------------------------------------------
    std::unique_ptr<SearchController>   searchCtrl;
    std::unique_ptr<TreeViewController> treeCtrl;
    std::unique_ptr<ThemeManager>       themeManager;

    // ---- Domain controller --------------------------------------------------
    std::unique_ptr<JSONController> controller;

    // ---- Setup --------------------------------------------------------------
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupConnections();

    // ---- Helpers ------------------------------------------------------------
    void updateStatusBar (const QString& message, bool isError = false);
    void updateWindowTitle();
    bool maybeSave();

    // No more parseErrorMessage() – ParseError carries location directly.
    // No more applyTheme() – ThemeManager owns that.
    // No more search methods – SearchController owns those.
    // No more populateTreeView() – TreeViewController owns that.
};

#endif // MAINWINDOW_H

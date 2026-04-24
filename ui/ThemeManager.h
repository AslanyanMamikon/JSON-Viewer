#ifndef THEME_MANAGER_H_
#define THEME_MANAGER_H_

#include <QMainWindow>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>

// Forward declaration – CodeEditor is defined in mainwindow.h.
// We only store a pointer here, so a full include is not needed and
// would create a circular dependency (mainwindow.h → ThemeManager.h → mainwindow.h).
class CodeEditor;

// ---------------------------------------------------------------------------
// ThemeManager
//
// Owns dark/light theme state and applies stylesheets to registered widgets.
// Previously all of this was inlined inside MainWindow::applyTheme() as one
// 80-line method.  Extracting it keeps MainWindow focused on coordination.
// ---------------------------------------------------------------------------
class ThemeManager : public QObject {
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);

    // Register widgets that need theming.
    // MainWindow calls these once during construction.
    void setMainWindow   (QMainWindow*  w)  { mainWindow   = w; }
    void setTreeView     (QTreeWidget*  w)  { treeView     = w; }
    void setStatusLabel  (QLabel*       w)  { statusLabel  = w; }
    void setThemeButton  (QPushButton*  w)  { themeButton  = w; }
    void setCodeEditor   (CodeEditor*   w)  { codeEditor   = w; }

    bool isDark() const { return dark; }

    void setDark(bool enabled);
    void toggle();

signals:
    void themeChanged(bool isDark);

private:
    void applyDark();
    void applyLight();

    bool          dark        = false;
    QMainWindow*  mainWindow  = nullptr;
    QTreeWidget*  treeView    = nullptr;
    QLabel*       statusLabel = nullptr;
    QPushButton*  themeButton = nullptr;
    CodeEditor*   codeEditor  = nullptr;
};

#endif // THEME_MANAGER_H_

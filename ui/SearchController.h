#ifndef SEARCH_CONTROLLER_H_
#define SEARCH_CONTROLLER_H_

#include <QTextEdit>
#include <QList>
#include <QTextCursor>
#include <QColor>
#include <QString>

// ---------------------------------------------------------------------------
// SearchController
//
// Owns all search/highlight state that was previously scattered across
// MainWindow.  MainWindow simply calls into this class and reads results.
//
// Responsibilities:
//   - Finding all occurrences of a query in a QTextEdit document
//   - Maintaining the "current" result index with wrap-around navigation
//   - Applying highlight extra-selections (normal + active colours)
//   - Clearing state when the query changes or the document changes
// ---------------------------------------------------------------------------
class SearchController : public QObject {
    Q_OBJECT

public:
    explicit SearchController(QTextEdit* editor, QObject* parent = nullptr);

    // Perform a fresh search; replaces any previous results.
    // Returns the number of matches found.
    int search(const QString& text);

    // Navigate; safe to call when results are empty (no-ops).
    void next();
    void previous();

    // Clear all highlights and reset state.
    void clear();

    // Called when the document changes mid-session so highlights are refreshed.
    void onDocumentChanged();

    // Theming
    void setDarkMode(bool dark);

    // State queries
    bool    hasResults()        const { return !results.isEmpty(); }
    int     resultCount()       const { return results.size(); }
    int     currentIndex()      const { return index; }
    QString currentQuery()      const { return query; }

signals:
    // Emitted after every search/navigate so the status bar can be updated.
    void statusChanged(const QString& message);

private:
    void applyHighlights();
    void navigateTo(int i);

    QTextEdit*           editor;
    QString              query;
    QList<QTextCursor>   results;
    int                  index      = -1;
    bool                 darkMode   = false;
};

#endif // SEARCH_CONTROLLER_H_

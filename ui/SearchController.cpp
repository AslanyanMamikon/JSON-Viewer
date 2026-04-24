#include "SearchController.h"
#include <QTextDocument>

SearchController::SearchController(QTextEdit* editor, QObject* parent)
    : QObject(parent), editor(editor)
{}

void SearchController::setDarkMode(bool dark) {
    darkMode = dark;
    if (hasResults()) applyHighlights();   // refresh colours immediately
}

int SearchController::search(const QString& text) {
    clear();
    query = text;

    if (text.isEmpty()) {
        emit statusChanged("Ready");
        return 0;
    }

    QTextDocument* doc = editor->document();
    QTextCursor cursor(doc);

    while (true) {
        cursor = doc->find(text, cursor);
        if (cursor.isNull()) break;
        results.append(cursor);
    }

    if (results.isEmpty()) {
        emit statusChanged("No results found");
        return 0;
    }

    index = 0;
    navigateTo(index);
    emit statusChanged(QString("Found %1 result(s)").arg(results.size()));
    return results.size();
}

void SearchController::next() {
    if (results.isEmpty()) return;
    index = (index + 1) % results.size();
    navigateTo(index);
}

void SearchController::previous() {
    if (results.isEmpty()) return;
    index = (index - 1 + results.size()) % results.size();
    navigateTo(index);
}

void SearchController::clear() {
    results.clear();
    index = -1;
    query.clear();
    editor->setExtraSelections({});
}

void SearchController::onDocumentChanged() {
    // Re-run the last query if there was one; the document content shifted.
    if (!query.isEmpty()) search(query);
}

void SearchController::navigateTo(int i) {
    if (i < 0 || i >= results.size()) return;
    applyHighlights();                          // repaint with current index
    editor->setTextCursor(results[i]);
    editor->ensureCursorVisible();
    emit statusChanged(QString("Result %1 of %2").arg(i + 1).arg(results.size()));
}

void SearchController::applyHighlights() {
    QColor normal  = darkMode ? QColor(58,  86, 138) : QColor(255, 230, 140);
    QColor current = darkMode ? QColor(94, 138,  58) : QColor(255, 165,   0);

    QList<QTextEdit::ExtraSelection> selections;
    for (int i = 0; i < results.size(); ++i) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = results[i];
        sel.format.setBackground(i == index ? current : normal);
        selections.append(sel);
    }
    editor->setExtraSelections(selections);
}

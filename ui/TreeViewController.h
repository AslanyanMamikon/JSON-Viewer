#ifndef TREE_VIEW_CONTROLLER_H_
#define TREE_VIEW_CONTROLLER_H_

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <memory>
#include "Value.h"

// ---------------------------------------------------------------------------
// TreeViewController
//
// Owns all logic for populating and refreshing the JSON tree view.
// Previously this lived as several methods on MainWindow, violating SRP.
//
// Responsibilities:
//   - Clearing and repopulating the QTreeWidget from a JSONValue tree
//   - Computing display type names for each node
//   - Handling item-click signals and emitting a clean status string
// ---------------------------------------------------------------------------
class TreeViewController : public QObject {
    Q_OBJECT

public:
    explicit TreeViewController(QTreeWidget* treeView, QObject* parent = nullptr);

    // Rebuild the tree from the given document root.
    // Passing nullptr clears the tree.
    void populate(const std::shared_ptr<JSONValue>& document);

    // Refresh without changing the document (e.g. after a theme switch).
    void refresh();

signals:
    // Emitted when the user clicks a node; carries a status string.
    void itemSelected(const QString& statusText);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    void populateItem(const std::shared_ptr<JSONValue>& value,
                      QTreeWidgetItem* parent);

    static QString typeName(const std::shared_ptr<JSONValue>& value);

    QTreeWidget*               treeView;
    std::shared_ptr<JSONValue> currentDocument;
};

#endif // TREE_VIEW_CONTROLLER_H_

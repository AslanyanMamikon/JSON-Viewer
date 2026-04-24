#include "TreeViewController.h"
#include <sstream>

TreeViewController::TreeViewController(QTreeWidget* treeView, QObject* parent)
    : QObject(parent), treeView(treeView)
{
    connect(treeView, &QTreeWidget::itemClicked,
            this,     &TreeViewController::onItemClicked);
}

void TreeViewController::populate(const std::shared_ptr<JSONValue>& document) {
    currentDocument = document;
    treeView->clear();

    if (!document) return;

    QTreeWidgetItem* rootItem = new QTreeWidgetItem();
    rootItem->setText(0, "root");
    rootItem->setText(2, typeName(document));
    treeView->addTopLevelItem(rootItem);

    populateItem(document, rootItem);
    treeView->expandAll();
}

void TreeViewController::refresh() {
    populate(currentDocument);
}

void TreeViewController::populateItem(const std::shared_ptr<JSONValue>& value,
                                      QTreeWidgetItem* parent)
{
    if (value->isObject()) {
        for (const auto& [key, val] : value->asObject()) {
            auto* item = new QTreeWidgetItem(parent);
            item->setText(0, QString::fromStdString(key));
            item->setText(2, typeName(val));

            if (val->isObject() || val->isArray()) {
                populateItem(val, item);
            } else {
                std::ostringstream oss;
                val->print(oss, 0);
                item->setText(1, QString::fromStdString(oss.str()));
            }
        }
    } else if (value->isArray()) {
        const auto& arr = value->asArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            auto* item = new QTreeWidgetItem(parent);
            item->setText(0, QString("[%1]").arg(i));
            item->setText(2, typeName(arr[i]));

            if (arr[i]->isObject() || arr[i]->isArray()) {
                populateItem(arr[i], item);
            } else {
                std::ostringstream oss;
                arr[i]->print(oss, 0);
                item->setText(1, QString::fromStdString(oss.str()));
            }
        }
    }
}

QString TreeViewController::typeName(const std::shared_ptr<JSONValue>& value) {
    if (!value)          return "null";
    if (value->isNull()) return "null";
    if (value->isInt())  return "Integer";
    if (value->isFloat())return "Float";
    if (value->isBool()) return "Boolean";
    if (value->isString())return "String";
    if (value->isObject())return "Object";
    if (value->isArray()) return "Array";
    return "unknown";
}

void TreeViewController::onItemClicked(QTreeWidgetItem* item, int /*column*/) {
    if (!item) return;
    QString status = QString("Selected: %1").arg(item->text(0));
    if (!item->text(1).isEmpty())
        status += QString(" = %1").arg(item->text(1));
    status += QString(" (%1)").arg(item->text(2));
    emit itemSelected(status);
}

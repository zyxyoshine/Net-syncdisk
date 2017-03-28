#include "treeviewmodel.h"

using namespace TMY;

TreeViewModel::TreeViewModel()
{
    model = new QStandardItemModel;
    QStandardItem *root = model->invisibleRootItem();
}

TreeViewModel::TreeViewModel(DirInfo di)
{
    model = new QStandardItemModel;
    QStandardItem *root = model->invisibleRootItem();
    fromDirInfo(di);
}

void TreeViewModel::fromDirInfo(DirInfo di)
{
    root->removeRows(0, root->rowCount());
    for (DirInfoEntry &entry : di)
        add(entry.filePath);
}

void TreeViewModel::add(FilePath path)
{
    QStandardItem *p = root;
    for (std::string &node : path.pathArr)
        p = getChild(p, QString(node.c_str()));
    p->appendRow(new QStandardItem(QString(path.filename.c_str())));
}

void TreeViewModel::del(FilePath path)
{
    QStandardItem *p = root;
    for (std::string &node : path.pathArr)
        p = getChild(p, QString(node.c_str()));
    for (int i = 0; i < p->rowCount(); ++i)
    {
        if (p->child(i)->text() == QString(path.filename.c_str()))
            p->removeRow(i);
    }
}

QStandardItem *TreeViewModel::getChild(QStandardItem *parent, QString name)
{
    QStandardItem *p;
    for (int i = 0; i < parent->rowCount(); ++i)
    {
        p = parent->child(i);
        if (p->text() == name)
            return p;
    }
    parent->appendRow(new QStandardItem(name));
}

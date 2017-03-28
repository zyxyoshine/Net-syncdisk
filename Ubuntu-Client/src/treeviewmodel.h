#ifndef TREEVIEWMODEL_H
#define TREEVIEWMODEL_H

#include <QStandardItemModel>
#include "common.h"

namespace TMY
{

class TreeViewModel
{
public:
    QStandardItemModel *model;
    TreeViewModel();
    TreeViewModel(DirInfo di);
    void fromDirInfo(DirInfo di);
    void add(FilePath path);
    void del(FilePath path);
private:
    QStandardItem *root;
    QStandardItem *getChild(QStandardItem *parent, QString name);
};

}

#endif // TREEVIEWMODEL_H

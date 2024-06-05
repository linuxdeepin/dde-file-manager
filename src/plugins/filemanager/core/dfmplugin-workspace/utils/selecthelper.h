// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SELECTHELPER_H
#define SELECTHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QModelIndex>
#include <QItemSelectionModel>

QT_BEGIN_NAMESPACE
class QItemSelection;
QT_END_NAMESPACE

namespace dfmplugin_workspace {
class FileView;
class SelectHelper : public QObject
{
    Q_OBJECT

public:
    explicit SelectHelper(FileView *parent);
    QModelIndex getCurrentPressedIndex() const;
    void click(const QModelIndex &index);
    void release();
    void setSelection(const QItemSelection &selection);
    void selection(const QRect &rect, QItemSelectionModel::SelectionFlags flags);
    bool select(const QList<QUrl> &urls);

    void saveSelectedFilesList(const QUrl &current, const QList<QUrl> &urls);
    void resortSelectFiles();
private:
    void caculateSelection(const QRect &rect, QItemSelection *selection);
    void caculateIconViewSelection(const QRect &rect, QItemSelection *selection);
    void caculateListViewSelection(const QRect &rect, QItemSelection *selection);
    // Calculate the different items of the original selection item and the current
    // selection item, and the selection of these differnt items will be select
    void caculateAndSelectIndex(const QItemSelection &lastSelection,
                                const QItemSelection &newSelection,
                                QItemSelectionModel::SelectionFlags flags);

private:
    FileView *view { nullptr };
    QModelIndex lastPressedIndex;
    QModelIndex currentPressedIndex;
    QItemSelection currentSelection;
    QItemSelection lastSelection;

    QList<QUrl> selectedFiles {};
    QUrl currentSelectedFile {};
};

}
#endif   // SELECTHELPER_H

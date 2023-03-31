// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEVIEWHELPER_H
#define FILEVIEWHELPER_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>

QT_BEGIN_NAMESPACE
class QStyleOptionViewItem;
QT_END_NAMESPACE

namespace dfmplugin_workspace {
extern const char *const kEidtorShowSuffix;
class BaseItemDelegate;
class FileViewModel;
class FileView;

class FileViewHelper : public QObject
{
    Q_OBJECT
public:
    explicit FileViewHelper(FileView *parent = nullptr);
    ~FileViewHelper();

    FileView *parent() const;
    virtual bool isTransparent(const QModelIndex &index) const;
    const FileInfoPointer fileInfo(const QModelIndex &index) const;
    QMargins fileViewViewportMargins() const;
    QWidget *indexWidget(const QModelIndex &index) const;
    int selectedIndexsCount() const;
    bool isSelected(const QModelIndex &index) const;
    bool isDropTarget(const QModelIndex &index) const;
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    void updateGeometries();
    void keyboardSearch(const QString &search);
    bool isEmptyArea(const QPoint &pos);
    QSize viewContentSize() const;
    int verticalOffset() const;

    static int caculateListItemIndex(const QSize &itemSize, const QPoint &pos);
    static int caculateIconItemIndex(const FileView *view, const QSize &itemSize, const QPoint &pos);

public slots:
    void handleCommitData(QWidget *editor) const;
    void selectFiles(const QList<QUrl> &files);
    void handleTrashStateChanged();

private slots:
    void clipboardDataChanged();
    void clearSearchKey();

signals:
    void triggerEdit(const QModelIndex &index);

private:
    void init();
    BaseItemDelegate *itemDelegate() const;
    QModelIndex findIndex(const QByteArray &keys, bool matchStart,
                          int current, bool reverseOrder, bool excludeCurrent) const;

    QByteArray keyboardSearchKeys;
    QTimer *keyboardSearchTimer;
};

}

#endif   // FILEVIEWHELPER_H

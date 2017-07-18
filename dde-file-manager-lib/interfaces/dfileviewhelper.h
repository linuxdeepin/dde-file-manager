/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DFILEVIEWHELPER_H
#define DFILEVIEWHELPER_H

#include <QObject>
#include <QMargins>
#include <QEvent>

#include "dabstractfileinfo.h"

QT_BEGIN_NAMESPACE
class QAbstractItemView;
class QStyleOptionViewItem;
QT_END_NAMESPACE
class DAbstractFileInfo;
class DStyledItemDelegate;
class DFileSystemModel;
class DFileViewHelperPrivate;
class DFileViewHelper : public QObject
{
    Q_OBJECT

public:
    explicit DFileViewHelper(QAbstractItemView *parent);
    ~DFileViewHelper();

    QAbstractItemView *parent() const;

    virtual quint64 windowId() const;
    virtual bool isCut(const QModelIndex &index) const;
    virtual bool isSelected(const QModelIndex &index) const;
    virtual bool isDropTarget(const QModelIndex &index) const;
    virtual int selectedIndexsCount() const;
    virtual int rowCount() const;
    virtual int indexOfRow(const QModelIndex &index) const;
    QList<QIcon> additionalIcon(const QModelIndex &index) const;
    QString baseName(const QModelIndex &index) const;
    virtual QList<int> columnRoleList() const;
    virtual int columnWidth(int columnIndex) const;
    virtual DUrl currentUrl() const;

    virtual const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const = 0;
    virtual DStyledItemDelegate *itemDelegate() const = 0;
    virtual DFileSystemModel *model() const = 0;
    virtual const DUrlList selectedUrls() const = 0;
    virtual void select(const QList<DUrl> &list) = 0;

    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;

    void setIndexWidget(const QModelIndex &index, QWidget *widget);
    QWidget *indexWidget(const QModelIndex &index) const;
    void updateGeometries();
    QMargins fileViewViewportMargins() const;

    void keyboardSearch(char key);
    bool isEmptyArea(const QPoint &pos) const;

    void showPreviewFileDialog();

public slots:
    void handleCommitData(QWidget *editor) const;

signals:
    void triggerEdit(const QModelIndex &index);

private:
    QScopedPointer<DFileViewHelperPrivate> d_ptr;

    Q_PRIVATE_SLOT(d_func(), void _q_edit(const DFMUrlBaseEvent &event))
    Q_PRIVATE_SLOT(d_func(), void _q_selectAndRename(const DFMUrlBaseEvent &event))

    Q_DECLARE_PRIVATE(DFileViewHelper)
    Q_DISABLE_COPY(DFileViewHelper)
};

#endif // DFILEVIEWHELPER_H

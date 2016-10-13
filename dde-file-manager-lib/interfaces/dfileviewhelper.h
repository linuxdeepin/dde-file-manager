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

QT_BEGIN_NAMESPACE
class QAbstractItemView;
QT_END_NAMESPACE
class AbstractFileInfo;
class DFileViewHelper : public QObject
{
    Q_OBJECT

public:
    explicit DFileViewHelper(QAbstractItemView *parent);

    QAbstractItemView *parent() const;

    virtual int windowId() const;
    virtual bool isCut(const QModelIndex &index) const;
    virtual bool isSelected(const QModelIndex &index) const;
    virtual bool isDropTarget(const QModelIndex &index) const;
    virtual int selectedIndexsCount() const;
    virtual int rowCount() const;
    virtual int indexOfRow(const QModelIndex &index) const;
    virtual QList<QIcon> additionalIcon(const QModelIndex &index) const;
    virtual QString selectionWhenEditing(const QModelIndex &index) const;
    virtual const AbstractFileInfo *fileInfo(const QModelIndex &index) const;

    virtual QList<int> columnRoleList() const;
    virtual int columnWidth(int columnIndex) const;

    void setIndexWidget(const QModelIndex &index, QWidget *widget);
    QWidget *indexWidget(const QModelIndex &index) const;
    void updateGeometries();
    QMargins fileViewViewportMargins() const;

signals:
    void triggerEdit(const QModelIndex &index);
};

#endif // DFILEVIEWHELPER_H

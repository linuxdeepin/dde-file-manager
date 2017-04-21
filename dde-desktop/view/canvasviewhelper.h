/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <dfileviewhelper.h>

class DFMEvent;
class CanvasGridView;
class CanvasViewHelper : public DFileViewHelper
{
    Q_OBJECT
public:
    CanvasViewHelper(CanvasGridView *parent);

    CanvasGridView *parent() const;

    virtual quint64 windowId() const Q_DECL_OVERRIDE;
    virtual const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual DStyledItemDelegate *itemDelegate() const Q_DECL_OVERRIDE;
    virtual DFileSystemModel *model() const Q_DECL_OVERRIDE;
    virtual const DUrlList selectedUrls() const Q_DECL_OVERRIDE;
    virtual void select(const QList<DUrl> &list) Q_DECL_OVERRIDE;
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual int selectedIndexsCount() const Q_DECL_OVERRIDE;
    virtual bool isSelected(const QModelIndex &index) const Q_DECL_OVERRIDE;

public slots:
    void edit(const DFMEvent &event);
};

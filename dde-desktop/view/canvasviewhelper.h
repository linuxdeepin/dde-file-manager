/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <dfmevent.h>
#include <dfileviewhelper.h>

class DFMEvent;
class CanvasGridView;
class CanvasViewHelper : public DFileViewHelper
{
    Q_OBJECT
public:
    explicit CanvasViewHelper(CanvasGridView *parent);

    CanvasGridView *parent() const;

    virtual quint64 windowId() const override;
    virtual const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const override;
    virtual DFMStyledItemDelegate *itemDelegate() const override;
    virtual DFileSystemModel *model() const override;
    virtual const DUrlList selectedUrls() const override;
    virtual void select(const QList<DUrl> &list) override;
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    virtual int selectedIndexsCount() const override;
    virtual bool isSelected(const QModelIndex &index) const override;

public slots:
    void edit(const DFMEvent &event);
    void onRequestSelectFiles(const QList<DUrl> &urls);
    void handleSelectEvent(const DFMUrlListBaseEvent &event);
};

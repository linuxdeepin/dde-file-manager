/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef FILEVIEWHELPER_H
#define FILEVIEWHELPER_H

#include "dfmevent.h"
#include "interfaces/dfileviewhelper.h"

#include <QPointer>

#define FILE_SELECT_THRESHOLD   6   // 是否启用线程选择文件的阈值

class DFileView;
class FileViewHelper : public DFileViewHelper
{
    Q_OBJECT

public:
    explicit FileViewHelper(DFileView *parent);

    DFileView *parent() const;

    quint64 windowId() const override;
    bool isSelected(const QModelIndex &index) const override;
    bool isDropTarget(const QModelIndex &index) const override;
    int selectedIndexsCount() const override;
    int rowCount() const override;
    int indexOfRow(const QModelIndex &index) const override;
    const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const override;
    DFMStyledItemDelegate *itemDelegate() const override;
    DFileSystemModel *model() const override;
    const DUrlList selectedUrls() const override;
    DUrl currentUrl() const override;
    QList<int> columnRoleList() const override;
    int columnWidth(int columnIndex) const override;
    void select(const QList<DUrl> &list) override;
    virtual void viewFlicker() override;
public slots:
    // helper fm event
    void preHandleCd(const DFMUrlBaseEvent &event);
    void cd(const DFMUrlBaseEvent &event);
    void cdUp(const DFMUrlBaseEvent &event);
    void handleSelectEvent(const DFMUrlListBaseEvent &event);
    void selectAll(quint64 windowId);
    void setFoucsOnFileView(quint64 winId);
    void refreshFileView(quint64 winId);

private:
    DFMUrlBaseEvent lastEvent;
};

#endif // FILEVIEWHELPER_H

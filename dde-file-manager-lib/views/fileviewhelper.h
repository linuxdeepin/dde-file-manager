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

#include "interfaces/dfileviewhelper.h"

class DFileView;
class FMEvent;
class FileViewHelper : public DFileViewHelper
{
    Q_OBJECT

public:
    explicit FileViewHelper(DFileView *parent);

    DFileView *parent() const;

    int windowId() const Q_DECL_OVERRIDE;
    bool isSelected(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool isDropTarget(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int selectedIndexsCount() const Q_DECL_OVERRIDE;
    int rowCount() const Q_DECL_OVERRIDE;
    int indexOfRow(const QModelIndex &index) const Q_DECL_OVERRIDE;
    const AbstractFileInfo *fileInfo(const QModelIndex &index) const Q_DECL_OVERRIDE;
    DStyledItemDelegate *itemDelegate() const Q_DECL_OVERRIDE;
    DFileSystemModel *model() const Q_DECL_OVERRIDE;
    const DUrlList selectedUrls() const Q_DECL_OVERRIDE;
    DUrl currentUrl() const Q_DECL_OVERRIDE;
    QList<int> columnRoleList() const Q_DECL_OVERRIDE;
    int columnWidth(int columnIndex) const Q_DECL_OVERRIDE;

public slots:
    // helper fm event
    void preHandleCd(const FMEvent &event);
    void cd(const FMEvent &event);
    void cdUp(const FMEvent &event);
    void edit(const FMEvent &event);
    void select(const FMEvent &event);
    void selectAll(int windowId);
    void selectAndRename(const FMEvent &event);
    void setFoucsOnFileView(const FMEvent& event);
    void refreshFileView(const FMEvent& event);

private:
    void emitUrlChanged();

    int lastEventSource;
};

#endif // FILEVIEWHELPER_H

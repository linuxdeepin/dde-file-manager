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
class DFMEvent;
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
    const DAbstractFileInfoPointer fileInfo(const QModelIndex &index) const Q_DECL_OVERRIDE;
    DStyledItemDelegate *itemDelegate() const Q_DECL_OVERRIDE;
    DFileSystemModel *model() const Q_DECL_OVERRIDE;
    const DUrlList selectedUrls() const Q_DECL_OVERRIDE;
    DUrl currentUrl() const Q_DECL_OVERRIDE;
    QList<int> columnRoleList() const Q_DECL_OVERRIDE;
    int columnWidth(int columnIndex) const Q_DECL_OVERRIDE;

public slots:
    // helper fm event
    void preHandleCd(const DFMEvent &event);
    void cd(const DFMEvent &event);
    void cdUp(const DFMEvent &event);
    void edit(const DFMEvent &event);
    void select(const DFMEvent &event);
    void selectAll(int windowId);
    void selectAndRename(const DFMEvent &event);
    void setFoucsOnFileView(const DFMEvent& event);
    void refreshFileView(const DFMEvent& event);

private:
    void onCurrentUrlChanged(const DUrl &url);

    int lastEventSource;
};

#endif // FILEVIEWHELPER_H

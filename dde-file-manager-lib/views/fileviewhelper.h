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
class FileViewHelper : public DFileViewHelper
{
public:
    explicit FileViewHelper(DFileView *parent);

    DFileView *parent() const;

    bool isSelected(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool isDropTarget(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int selectedIndexsCount() const Q_DECL_OVERRIDE;
    int rowCount() const Q_DECL_OVERRIDE;
    int indexOfRow(const QModelIndex &index) const Q_DECL_OVERRIDE;
    const AbstractFileInfo *fileInfo(const QModelIndex &index) const Q_DECL_OVERRIDE;

    QList<int> columnRoleList() const Q_DECL_OVERRIDE;
    int columnWidth(int columnIndex) const Q_DECL_OVERRIDE;
};

#endif // FILEVIEWHELPER_H

// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEITEMDATA_H
#define FILEITEMDATA_H

#include "dfmplugin_workspace_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/interfaces/abstractdiriterator.h"
#include "dfm-base/dfm_base_global.h"

namespace dfmplugin_workspace {

class FileItemData
{
public:
    explicit FileItemData(const QUrl &url, const AbstractFileInfoPointer &info = nullptr, FileItemData *parent = nullptr);
    explicit FileItemData(const SortInfoPointer &info, FileItemData *parent = nullptr);

    void setParentData(FileItemData *p);
    void setSortFileInfo(SortInfoPointer info);

    void refreshInfo();
    AbstractFileInfoPointer fileInfo() const;
    FileItemData *parentData() const;

    QVariant data(int role) const;

private:
    FileItemData *parent { nullptr };
    QUrl url;
    AbstractFileInfoPointer info { nullptr };
    SortInfoPointer sortInfo { nullptr };
};

}

#endif   // FILEITEMDATA_H

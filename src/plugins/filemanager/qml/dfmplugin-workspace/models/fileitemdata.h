// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEITEMDATA_H
#define FILEITEMDATA_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/dfm_base_global.h>

namespace dfmplugin_workspace {

class FileItemData
{
public:
    explicit FileItemData(const QUrl &url, const FileInfoPointer &info = nullptr, FileItemData *parent = nullptr);
    explicit FileItemData(const SortInfoPointer &info, FileItemData *parent = nullptr);

    void setParentData(FileItemData *p);
    void setSortFileInfo(SortInfoPointer info);

    void refreshInfo();
    void clearThumbnail();
    FileInfoPointer fileInfo() const;
    FileItemData *parentData() const;
    QIcon fileIcon() const;

    QVariant data(int role) const;

    void setAvailableState(bool b);
    void setExpanded(bool b);
    void setDepth(const int8_t depth);

private:
    bool isDir() const;

private:
    FileItemData *parent { nullptr };
    QUrl url;
    FileInfoPointer info { nullptr };
    SortInfoPointer sortInfo { nullptr };
    bool isAvailable { true };
    std::atomic_int8_t depth { 0 };
    std::atomic_bool expanded { false };
    std::atomic_int subFileCount{ 0 }; // sub file count,not contain hide file
};

}

typedef QSharedPointer<DPWORKSPACE_NAMESPACE::FileItemData> FileItemDataPointer;

#endif   // FILEITEMDATA_H

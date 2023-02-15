// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEITEMDATA_H
#define FILEITEMDATA_H

#include "dfmplugin_workspace_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_base_global.h"

#include <QSharedData>

namespace dfmplugin_workspace {

class FileItemData : public QObject
{
public:
    explicit FileItemData(const QUrl &url,const AbstractFileInfoPointer &info = nullptr, FileItemData *parent = nullptr);

    void setParentData(FileItemData *p);

    void refreshInfo();
    AbstractFileInfoPointer fileInfo() const;
    FileItemData *parentData() const;

    QVariant data(int role) const;

private:
    FileItemData *parent;
    QUrl url;
    AbstractFileInfoPointer info{ nullptr };
};

}

#endif   // FILEITEMDATA_H

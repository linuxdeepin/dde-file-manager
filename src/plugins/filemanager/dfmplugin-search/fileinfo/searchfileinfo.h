// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHFILEINFO_H
#define SEARCHFILEINFO_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/fileinfo.h>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_search {

class SearchFileInfo : public FileInfo
{
public:
    explicit SearchFileInfo(const QUrl &url);
    ~SearchFileInfo() override;
    virtual bool exists() const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual Qt::DropActions supportedOfAttributes(const SupportType type) const override;
    virtual qint64 size() const override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    QString displayOf(const DisplayInfoType type) const override;

    // property for view
    virtual QString viewOfTip(const ViewType type) const override;
};

using SearchFileInfoPointer = QSharedPointer<SearchFileInfo>;
}

#endif   // SEARCHFILEINFO_H

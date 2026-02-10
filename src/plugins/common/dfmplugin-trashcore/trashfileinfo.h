// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEINFO_H
#define TRASHFILEINFO_H

#include "dfmplugin_trashcore_global.h"
#include <dfm-base/interfaces/proxyfileinfo.h>

namespace dfmplugin_trashcore {

class TrashFileInfoPrivate;
class TrashFileInfo : public DFMBASE_NAMESPACE::ProxyFileInfo
{
    friend class TrashFileInfoPrivate;

public:
    explicit TrashFileInfo(const QUrl &url);
    ~TrashFileInfo() override;

    virtual void refresh() override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QString pathOf(const FilePathInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual bool exists() const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual Qt::DropActions supportedOfAttributes(const SupportType type) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual QFile::Permissions permissions() const override;
    virtual QIcon fileIcon() override;

    virtual qint64 size() const override;
    virtual int countChildFile() const override;

    virtual QVariant timeOf(const FileTimeType type) const override;
    virtual QVariant customData(int role) const override;

private:
    QSharedPointer<TrashFileInfoPrivate> d { nullptr };
};

}

#endif   // TRASHFILEINFO_H

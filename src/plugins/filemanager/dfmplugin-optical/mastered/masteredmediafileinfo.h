// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTEREDMEDIAFILEINFO_H
#define MASTEREDMEDIAFILEINFO_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/interfaces/proxyfileinfo.h>

namespace dfmplugin_optical {
class MasteredMediaFileInfoPrivate;
class MasteredMediaFileInfo : public DFMBASE_NAMESPACE::ProxyFileInfo
{
    QSharedPointer<MasteredMediaFileInfoPrivate> d { nullptr };

public:
    explicit MasteredMediaFileInfo(const QUrl &url);
    explicit MasteredMediaFileInfo(const QUrl &url, const FileInfoPointer proxy);

    bool exists() const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    QString nameOf(const FileNameInfoType type) const override;
    virtual QUrl urlOf(const FileUrlInfoType type) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    QVariantHash extraProperties() const override;

    void refresh() override;
    virtual bool canAttributes(const FileCanType type) const override;
    Qt::DropActions supportedOfAttributes(const SupportType type) const override;
    QString viewOfTip(const ViewType type = ViewType::kEmptyDir) const override;
    virtual void updateAttributes(const QList<FileInfoAttributeID> &types = {}) override;
};

}   // namepsace dfmplugin_optical

#endif   // MASTEREDMEDIAFILEINFO_H

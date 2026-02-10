// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGFILEINFO_H
#define TAGFILEINFO_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/interfaces/proxyfileinfo.h>

namespace dfmplugin_tag {
class TagFileInfoPrivate;
class TagFileInfo : public DFMBASE_NAMESPACE::ProxyFileInfo
{
    Q_GADGET
    friend class TagFileInfoPrivate;
    QSharedPointer<TagFileInfoPrivate> d { nullptr };

public:
    explicit TagFileInfo(const QUrl &url);
    ~TagFileInfo() override;

    bool exists() const override;
    QFile::Permissions permissions() const override;

    QString nameOf(const FileNameInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual bool isAttributes(const FileIsType type) const override;
    FileType fileType() const override;
    QIcon fileIcon() override;

    QString tagName() const;
};

using TagFileInfoPointer = QSharedPointer<TagFileInfo>;

}

#endif   // TAGFILEINFO_H

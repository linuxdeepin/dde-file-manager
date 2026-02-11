// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBSHAREFILEINFO_H
#define SMBSHAREFILEINFO_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-base/interfaces/fileinfo.h>

namespace dfmplugin_smbbrowser {
class SmbShareFileInfoPrivate;
class SmbShareFileInfo : public dfmbase::FileInfo
{
    QSharedPointer<SmbShareFileInfoPrivate> d { nullptr };
    friend class SmbShareFileInfoPrivate;

public:
    explicit SmbShareFileInfo(const QUrl &url);
    virtual ~SmbShareFileInfo() override;

    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QIcon fileIcon() override;
    virtual bool isAttributes(const FileIsType type) const override;
    virtual bool canAttributes(const FileCanType type) const override;
    virtual bool exists() const override;
};

}

#endif   // SMBSHAREFILEINFO_H

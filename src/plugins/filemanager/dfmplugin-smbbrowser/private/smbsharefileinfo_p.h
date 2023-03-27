// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBSHAREFILEINFO_P_H
#define SMBSHAREFILEINFO_P_H

#include "dfmplugin_smbbrowser_global.h"
#include "typedefines.h"

#include "dfm-base/interfaces/private/fileinfo_p.h"

namespace dfmplugin_smbbrowser {

class SmbShareFileInfo;
class SmbShareFileInfoPrivate : public dfmbase::FileInfoPrivate
{
    friend class SmbShareFileInfo;

public:
    explicit SmbShareFileInfoPrivate(const QUrl &url, dfmbase::FileInfo *qq);
    virtual ~SmbShareFileInfoPrivate();

private:
    SmbShareNode node;
    QString fileName() const;
};

}

#endif   // SMBSHAREFILEINFO_P_H

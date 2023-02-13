// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREFILEINFO_P_H
#define SHAREFILEINFO_P_H

#include "dfmplugin_myshares_global.h"

#include "dfm-base/interfaces/private/abstractfileinfo_p.h"

namespace dfmplugin_myshares {

class ShareFileInfo;
class ShareFileInfoPrivate : public DFMBASE_NAMESPACE::AbstractFileInfoPrivate
{
    friend class ShareFileInfo;

public:
    explicit ShareFileInfoPrivate(const QUrl &url, DFMBASE_NAMESPACE::AbstractFileInfo *qq);
    virtual ~ShareFileInfoPrivate();
    void refresh();

private:
    ShareInfo info;
    QString fileName() const;
};

}

#endif   // SHAREFILEINFO_P_H

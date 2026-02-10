// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREFILEINFO_P_H
#define SHAREFILEINFO_P_H

#include "dfmplugin_myshares_global.h"

namespace dfmplugin_myshares {

class ShareFileInfo;
class ShareFileInfoPrivate
{
    friend class ShareFileInfo;

public:
    explicit ShareFileInfoPrivate(ShareFileInfo *qq);
    virtual ~ShareFileInfoPrivate();
    void refresh();

private:
    ShareInfo info;
    QString fileName() const;
    ShareFileInfo *const q;
};

}

#endif   // SHAREFILEINFO_P_H

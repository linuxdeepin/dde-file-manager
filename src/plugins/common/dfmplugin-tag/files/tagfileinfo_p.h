// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGFILEINFO_P_H
#define TAGFILEINFO_P_H

#include "dfmplugin_tag_global.h"

#include <dfm-base/interfaces/private/fileinfo_p.h>

namespace dfmplugin_tag {

class TagFileInfo;
class TagFileInfoPrivate
{
    friend class TagFileInfo;

public:
    explicit TagFileInfoPrivate(TagFileInfo *qq);
    virtual ~TagFileInfoPrivate();

private:
    QString fileName() const;
    TagFileInfo *const q;
};

}

#endif   // TAGFILEINFO_P_H

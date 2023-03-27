// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGFILEINFO_P_H
#define TAGFILEINFO_P_H

#include "dfmplugin_tag_global.h"

#include "dfm-base/interfaces/private/fileinfo_p.h"

namespace dfmplugin_tag {

class TagFileInfo;
class TagFileInfoPrivate : public DFMBASE_NAMESPACE::FileInfoPrivate
{
    friend class TagFileInfo;

public:
    explicit TagFileInfoPrivate(const QUrl &url, DFMBASE_NAMESPACE::FileInfo *qq);
    virtual ~TagFileInfoPrivate();

private:
    QString fileName() const;
};

}

#endif   // TAGFILEINFO_P_H

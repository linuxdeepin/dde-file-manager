// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILEINFO_H
#define AVFSFILEINFO_H

#include "dfmplugin_avfsbrowser_global.h"

#include "dfm-base/file/local/localfileinfo.h"

namespace dfmplugin_avfsbrowser {

class AvfsFileInfo : public DFMBASE_NAMESPACE::AbstractFileInfo
{
public:
    explicit AvfsFileInfo(const QUrl &url);
    virtual ~AvfsFileInfo() override;
};

}

#endif   // AVFSFILEINFO_H

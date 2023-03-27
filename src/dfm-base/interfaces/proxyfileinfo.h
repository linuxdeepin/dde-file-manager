// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROXYFILEINFO_H
#define PROXYFILEINFO_H

#include "fileinfo.h"

namespace dfmbase {
class ProxyFileInfo : public FileInfo
{
public:
    explicit ProxyFileInfo(const QUrl &url);
    ~ProxyFileInfo() override;
};
}
#endif   // PROXYFILEINFO_H

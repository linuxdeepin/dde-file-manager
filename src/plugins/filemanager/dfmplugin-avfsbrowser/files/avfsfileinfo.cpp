// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avfsfileinfo.h"
#include "utils/avfsutils.h"

#include "dfm-base/base/schemefactory.h"

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE

AvfsFileInfo::AvfsFileInfo(const QUrl &url)
    : AbstractFileInfo(url)
{
    setProxy(InfoFactory::create<AbstractFileInfo>(AvfsUtils::avfsUrlToLocal(url)));
}

AvfsFileInfo::~AvfsFileInfo()
{
}

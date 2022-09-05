// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractsearcher.h"

AbstractSearcher::AbstractSearcher(const DUrl &url, const QString &key, QObject *parent)
    : QObject(parent),
      searchUrl(url),
      keyword(key)
{
}

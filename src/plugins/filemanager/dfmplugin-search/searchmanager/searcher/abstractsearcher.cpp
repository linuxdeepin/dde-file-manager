// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractsearcher.h"

DPSEARCH_USE_NAMESPACE

AbstractSearcher::AbstractSearcher(const QUrl &url, const QString &key, QObject *parent)
    : QObject(parent),
      searchUrl(url),
      keyword(key)
{
}

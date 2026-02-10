// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBSHAREITERATOR_P_H
#define SMBSHAREITERATOR_P_H

#include "dfmplugin_smbbrowser_global.h"
#include "typedefines.h"

#include <QUrl>

#include <dfm-io/denumerator.h>

BEGIN_IO_NAMESPACE
class DLocalEnumerator;
END_IO_NAMESPACE

DPSMBBROWSER_BEGIN_NAMESPACE

class SmbShareIterator;
class SmbShareIteratorPrivate
{
    friend class SmbShareIterator;

public:
    explicit SmbShareIteratorPrivate(const QUrl &url, SmbShareIterator *qq);
    ~SmbShareIteratorPrivate();

private:
    SmbShareIterator *q { nullptr };
    SmbShareNodes smbShares;
    QScopedPointer<DFMIO::DEnumerator> enumerator { nullptr };
    QUrl rootUrl;
};

DPSMBBROWSER_END_NAMESPACE

#endif   // SMBSHAREITERATOR_P_H

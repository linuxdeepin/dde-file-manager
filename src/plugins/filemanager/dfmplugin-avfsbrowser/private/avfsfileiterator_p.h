// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILEITERATOR_P_H
#define AVFSFILEITERATOR_P_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-base/file/local/localdiriterator.h>

#include <QUrl>

namespace dfmplugin_avfsbrowser {

class AvfsFileIterator;
class AvfsFileIteratorPrivate
{
    friend class AvfsFileIterator;

public:
    explicit AvfsFileIteratorPrivate(const QUrl &root, AvfsFileIterator *qq);
    ~AvfsFileIteratorPrivate();

private:
    AvfsFileIterator *q { nullptr };
    QUrl root;
    dfmbase::LocalDirIterator *proxy { nullptr };
};

}

#endif   // AVFSFILEITERATOR_P_H

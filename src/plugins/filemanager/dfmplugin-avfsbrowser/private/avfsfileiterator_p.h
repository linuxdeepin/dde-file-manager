// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSFILEITERATOR_P_H
#define AVFSFILEITERATOR_P_H

#include "dfmplugin_avfsbrowser_global.h"

#include <QUrl>

namespace dfmplugin_avfsbrowser {

class AvfsFileIterator;
class AvfsFileIteratorPrivate
{
    friend class AvfsFileIterator;

public:
    explicit AvfsFileIteratorPrivate(const QUrl &root, AvfsFileIterator *qq);

private:
    AvfsFileIterator *q { nullptr };
    QUrl root;
};

}

#endif   // AVFSFILEITERATOR_P_H

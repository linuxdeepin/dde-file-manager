// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHAREITERATOR_P_H
#define SHAREITERATOR_P_H

#include "dfmplugin_myshares_global.h"

namespace dfmplugin_myshares {

class ShareIterator;
class ShareIteratorPrivate
{
    friend class ShareIterator;

public:
    explicit ShareIteratorPrivate(ShareIterator *qq);
    ~ShareIteratorPrivate();

private:
    ShareIterator *q { nullptr };
    ShareInfoList shares;
    ShareInfo currentInfo;
};

}

#endif   // SHAREITERATOR_P_H

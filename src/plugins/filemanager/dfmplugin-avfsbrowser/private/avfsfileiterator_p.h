/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef AVFSFILEITERATOR_P_H
#define AVFSFILEITERATOR_P_H

#include "dfmplugin_avfsbrowser_global.h"

#include <QUrl>

DPAVFSBROWSER_BEGIN_NAMESPACE

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

DPAVFSBROWSER_END_NAMESPACE

#endif   // AVFSFILEITERATOR_P_H

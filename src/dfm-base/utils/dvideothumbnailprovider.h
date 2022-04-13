/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#ifndef DFM_DVIDEOTHUMBNAILPROVIDER_H
#define DFM_DVIDEOTHUMBNAILPROVIDER_H

#include "dfm_base_global.h"

#include <QScopedPointer>

class QImage;
class QString;

DFMBASE_BEGIN_NAMESPACE

class DVideoThumbnailProviderPrivate;
class DVideoThumbnailProvider
{
public:
    DVideoThumbnailProvider();
    ~DVideoThumbnailProvider();

    bool hasKey(const QString &key) const;
    QImage createThumbnail(const QString &size, const QString &path);

private:
    QScopedPointer<DVideoThumbnailProviderPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // DFM_DVIDEOTHUMBNAILPROVIDER_H

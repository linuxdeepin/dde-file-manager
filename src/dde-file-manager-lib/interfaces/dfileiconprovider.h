/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef DFILEICONPROVIDER_H
#define DFILEICONPROVIDER_H

#include <QFileIconProvider>

#include "dfmglobal.h"

class DFileInfo;
class DGvfsFileInfo;

DFM_BEGIN_NAMESPACE

class DFileIconProviderPrivate;
class DFileIconProvider : public QFileIconProvider
{
public:
    DFileIconProvider();
    ~DFileIconProvider() override;

    static DFileIconProvider *globalProvider();

    QIcon icon(const QFileInfo &info) const override;
    QIcon icon(const QFileInfo &info, const QIcon &feedback) const ;
    QIcon icon(const DFileInfo &info, const QIcon &feedback = QIcon()) const;
    QIcon icon(const DGvfsFileInfo &info, const QIcon &feedback = QIcon()) const;

private:
    QScopedPointer<DFileIconProviderPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFileIconProvider)
    Q_DISABLE_COPY(DFileIconProvider)
};

DFM_END_NAMESPACE

#endif // DFILEICONPROVIDER_H

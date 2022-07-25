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

#ifndef LOCALFILEICONPROVIDER_H
#define LOCALFILEICONPROVIDER_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QFileIconProvider>
#include <QScopedPointer>

namespace dfmbase {

class LocalFileIconProviderPrivate;
class LocalFileIconProvider : public QFileIconProvider
{
public:
    LocalFileIconProvider();
    ~LocalFileIconProvider() override;

    static LocalFileIconProvider *globalProvider();

    QIcon icon(const QFileInfo &info) const override;
    QIcon icon(const QString &path) const;
    QIcon icon(const QFileInfo &info, const QIcon &feedback) const;
    QIcon icon(const QString &path, const QIcon &feedback) const;
    QIcon icon(AbstractFileInfo *info, const QIcon &feedback = QIcon());

private:
    QScopedPointer<LocalFileIconProviderPrivate> d;

    Q_DISABLE_COPY(LocalFileIconProvider)
};

}

#endif   // LOCALFILEICONPROVIDER_H

/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
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

#ifndef HIDEFILEHELPER_H
#define HIDEFILEHELPER_H

#include "dfm-base/dfm_base_global.h"

#include <QUrl>
#include <QScopedPointer>

DFMBASE_BEGIN_NAMESPACE
class HideFileHelperPrivate;
class HideFileHelper
{
public:
    HideFileHelper(const QUrl &dir);
    ~HideFileHelper();

    QUrl dirUrl() const;
    QUrl fileUrl() const;
    bool save() const;
    bool insert(const QString &name);
    bool remove(const QString &name);
    bool contains(const QString &name);
    QSet<QString> hideFileList() const;

private:
    QScopedPointer<HideFileHelperPrivate> d;
};
DFMBASE_END_NAMESPACE

#endif   // HIDEFILEHELPER_H

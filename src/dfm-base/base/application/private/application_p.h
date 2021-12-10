/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef APPLICATION_P_H
#define APPLICATION_P_H

#include "dfm-base/dfm_base_global.h"

#include <QString>
#include <QVariant>

DFMBASE_BEGIN_NAMESPACE
class Application;
class ApplicationPrivate
{
public:
    explicit ApplicationPrivate(Application *qq);

    void _q_onSettingsValueChanged(const QString &group, const QString &key, const QVariant &value, bool edited = false);
    void _q_onSettingsValueEdited(const QString &group, const QString &key, const QVariant &value);

    static Application *self;
};
DFMBASE_END_NAMESPACE
#endif // APPLICATION_P_H

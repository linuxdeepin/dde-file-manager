/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#ifndef TAGHANDLEPLUGIN_H
#define TAGHANDLEPLUGIN_H

#include <dasplugin.h>

#include "taghandle.h"

using namespace DAS_NAMESPACE;

class TagHandlePlugin : public DASPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DASFactoryInterface_iid FILE "dde-anythingmonitor.json")

public:
    explicit TagHandlePlugin(QObject *const parent = nullptr);
    virtual ~TagHandlePlugin() = default;

    TagHandlePlugin(const TagHandlePlugin &other) = delete;
    TagHandlePlugin &operator=(const TagHandlePlugin &other) = delete;

    DASInterface *create(const QString &key);
};

#endif // TAGHANDLEPLUGIN_H

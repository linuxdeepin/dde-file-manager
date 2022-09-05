// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taghandleplugin.h"

TagHandlePlugin::TagHandlePlugin(QObject * const parent)
: DASPlugin{ parent }
{

}

DASInterface *TagHandlePlugin::create(const QString &key)
{
    (void)key;
    return (new TagHandle{});
}

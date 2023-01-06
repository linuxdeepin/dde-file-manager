// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENULOADER_P_H
#define DFMEXTMENULOADER_P_H

#include <QString>
#include <QLibrary>

class DFMExtPluginLoader;
class DFMExtPluginLoaderPrivate
{
    friend class DFMExtPluginLoader;
    DFMExtPluginLoader *const q;
    QLibrary qlib;
    QString errorString;
    explicit DFMExtPluginLoaderPrivate(DFMExtPluginLoader *qq)
        : q(qq)
    {

    }
};

#endif // DFMEXTMENULOADER_P_H

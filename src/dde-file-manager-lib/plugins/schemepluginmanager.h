/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author:      Xiao Zhiguo <xiaozhiguo@uniontech.com>
* Maintainer:  Xiao Zhiguo <xiaozhiguo@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// NOTE [XIAO]

#ifndef SCHEMEPLUGINMANAGER_H
#define SCHEMEPLUGINMANAGER_H

#include "schemeplugininterface.h"
#include <QObject>

typedef QList<QPair<QString, SchemePluginInterface *>> SchemePluginList;
typedef QList<QPair<QString, QString>> SchemePluginPathList;

class SchemePluginManager : public QObject
{
    Q_OBJECT
public:
    explicit SchemePluginManager(QObject *parent = nullptr);

    static SchemePluginManager *instance();
    QString schemePluginDir();
    void loadSchemePlugin();
    void unloadSchemePlugin(QString schemeName);
    SchemePluginList schemePlugins();

private:
    Q_DISABLE_COPY(SchemePluginManager)

    SchemePluginList schemePluginList;
    SchemePluginPathList schemePluginPahtList;
};

#endif // SCHEMEPLUGINMANAGER_H

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTPLUGINLOADER_H
#define DFMEXTPLUGINLOADER_H

#include <menu/dfmextmenuplugin.h>
#include <emblemicon/dfmextemblemiconplugin.h>
#include <QString>
#include <QSharedPointer>

class DFMExtPluginLoaderPrivate;
class DFMExtPluginLoader
{
    Q_DISABLE_COPY(DFMExtPluginLoader)
    DFMExtPluginLoaderPrivate *const d;
public:
    explicit DFMExtPluginLoader(const QString &filaName);
    virtual ~DFMExtPluginLoader();
    static bool hasSymbol(const QString &fileName);
    bool loadPlugin();
    bool initialize();
    void shutdown();
    QString fileName() const;
    QString errorString() const;
    QSharedPointer<DFMEXT::DFMExtMenuPlugin> extMenuPlugin();
    QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin> extEmbleIconPlugin();
};

typedef QSharedPointer<DFMExtPluginLoader> DFMExtPluginLoaderPointer;

#endif // DFMEXTPLUGINLOADER_H

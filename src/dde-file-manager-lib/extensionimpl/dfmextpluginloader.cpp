/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             huangyu<zhangyub@uniontech.com>
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
#include "dfmextpluginloader.h"
#include "dfmextmenuimplproxy.h"
#include "private/dfmextpluginloader_p.h"

#include "dfmextmenuimpl.h"

#include <QDebug>
#include <QMenu>
#include <QApplication>

#include <functional>

USING_DFMEXT_NAMESPACE

typedef void (*extension_initiliaze)();
typedef DFMExtMenuPlugin *(*extension_menu)();
typedef void (*extension_shutdown)();
typedef DFMExtEmblemIconPlugin *(*extension_emblem)();

static extension_initiliaze initFunc = nullptr;
static extension_shutdown shutdownFunc = nullptr;
static extension_menu menuFunc = nullptr;
static extension_emblem emblemFunc = nullptr;

DFMExtPluginLoader::DFMExtPluginLoader(const QString &filaName)
    : d(new DFMExtPluginLoaderPrivate(this))
{
    d->qlib.setFileName(filaName);
}

DFMExtPluginLoader::~DFMExtPluginLoader()
{
    if (d) {
        delete d;
    }
}

bool DFMExtPluginLoader::hasSymbol(const QString &fileName)
{
    QLibrary lib(fileName);
    if (!lib.load()) {
        return false;
    }

    auto initFuncTemp = reinterpret_cast<extension_initiliaze>(lib.resolve("dfm_extension_initiliaze"));
    auto shutdownFuncTemp = reinterpret_cast<extension_shutdown>(lib.resolve("dfm_extension_shutdown"));

    if (!initFuncTemp || !shutdownFuncTemp) {
        return false;
    }

    if (lib.isLoaded()) {
        lib.unload();
    }

    return true;
}

bool DFMExtPluginLoader::loadPlugin()
{
    if (d->qlib.fileName().isEmpty()) {
        d->errorString = "Failed, plugin file name is empty";
        qDebug() << d->errorString;
        return false;
    }

    bool ret = d->qlib.load();
    if (!ret) {
        d->errorString = d->qlib.errorString();
    }
    return ret;
}

bool DFMExtPluginLoader::initialize()
{
    if (!d->qlib.isLoaded()) {
        return false;
    }

    initFunc = reinterpret_cast<extension_initiliaze>(d->qlib.resolve("dfm_extension_initiliaze"));
    if (!initFunc) {
        d->errorString = "Failed, get 'dfm_extension_initiliaze' import function";
        qInfo() << d->errorString;
        return false;
    }

    initFunc();
    return true;
}

QSharedPointer<DFMExtMenuPlugin> DFMExtPluginLoader::extMenuPlugin()
{
    if (!d->qlib.isLoaded()) {
        d->errorString = "Failed, called 'extMenuPlugin' get interface, "
                         "need call 'initialize' function befor that";
        return nullptr;
    }

    menuFunc = reinterpret_cast<extension_menu>(d->qlib.resolve("dfm_extension_menu"));

    if (!menuFunc) {
        d->errorString = "Failed, get 'dfm_extension_menu' import function";
        return nullptr;
    }

    return QSharedPointer<DFMExtMenuPlugin>(menuFunc());
}

QSharedPointer<DFMExtEmblemIconPlugin> DFMExtPluginLoader::extEmbleIconPlugin()
{
    if (!d->qlib.isLoaded()) {
        d->errorString = "Failed, called 'extEmbleIconPlugin' get interface, "
                         "need call 'initialize' function befor that";
        return nullptr;
    }

    emblemFunc = reinterpret_cast<extension_emblem>(d->qlib.resolve("dfm_extension_emblem"));

    if (!emblemFunc) {
        d->errorString = "Failed, get 'dfm_extension_emblem' import function: "
                + d->qlib.fileName();
        return nullptr;
    }

    return QSharedPointer<DFMExtEmblemIconPlugin>(emblemFunc());
}

void DFMExtPluginLoader::shutdown()
{
    shutdownFunc = reinterpret_cast<extension_shutdown>(d->qlib.resolve("dfm_extension_shutdown"));
    if (!shutdownFunc) {
        d->errorString = "Failed, get 'dfm_extension_shutdown' import function: "
                + d->qlib.fileName();
        qInfo() << Q_FUNC_INFO << d->errorString;
        return;
    }

    //! delete interaface
    shutdownFunc();

    if (!d->qlib.isLoaded())
        return;

    //! 卸载插件so文件
    if(!d->qlib.unload()) {
        d->errorString = d->qlib.errorString();
        qInfo() << Q_FUNC_INFO << d->errorString;
    }
}

QString DFMExtPluginLoader::fileName() const
{
    return d->qlib.fileName();
}

QString DFMExtPluginLoader::errorString() const
{
    return d->qlib.errorString();
}


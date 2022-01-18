/*
* Copyright (C) 2021 Deepin Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "pluginemblemmanagerprivate.h"
#include "dfmextpluginmanager.h"
#include "emblemicon/dfmextemblem.h"
#include "plugins/pluginemblemmanager.h"
#include "dfmglobal.h"

#include <QDebug>
#include <QIcon>
#include <QTimerEvent>
#include <QApplication>
#include <QTimer>

static const int kEmblemUpdateTime { 300 };

USING_DFMEXT_NAMESPACE

PluginEmblemManagerPrivate::PluginEmblemManagerPrivate(PluginEmblemManager *qq)
    : q(qq)
{
    // 开启获取插件角标线程
    startWork();
    // 运行时增加插件，恢复线程刷新角标
    connect(&DFMExtPluginManager::instance(), &DFMExtPluginManager::extensionPluginCreated, this, [this]() {
        this->startWork();
    });
    connect(&DFMExtPluginManager::instance(), &DFMExtPluginManager::extensionPluginEnable, this, [this]() {
        emit q->updatePluginEmblem();
    });
    connect(&DFMExtPluginManager::instance(), &DFMExtPluginManager::extensionPluginDisbale, this, [this]() {
        clearEmblemIconsMap();
        emit q->updatePluginEmblem();
    });
}

PluginEmblemManagerPrivate::~PluginEmblemManagerPrivate()
{
    if (updateTimer) {
        updateTimer->stop();
        updateTimer->deleteLater();
        updateTimer = nullptr;
    }

    if (isRunning()) {
        stopWork();
    }
}

void PluginEmblemManagerPrivate::getPluginEmblemIconsFromMap(const DUrl &fileUrl, int systemIconCount, QList<QIcon> &icons)
{
    // 如果插件中没有实现角标类，直接返回
    if (!bHaveEmblemObj)
        return;

    // 获取插件角标前，更新下缓存的角标
    asyncUpdateEmblemIconsFromPlugin(fileUrl, systemIconCount);

    QString strFilePath = fileUrl.toLocalFile();
    if (mapIcons.contains(strFilePath)) {
        QList<QIcon> newIcons = {QIcon(), QIcon(), QIcon(), QIcon()};
        for (int i = 0; i < qMin(kMaxEmblemCount, icons.size()); ++i) {
            newIcons[i] = icons.at(i);
        }
        QList<QString> listPath = mapIcons[strFilePath];
        for (int i = systemIconCount; i < kMaxEmblemCount; ++i) {
            if (!listPath[i].isEmpty()) {
                // 优先从主题中拿角标，如果主题中没有，则认为是本地路径
                QIcon icon = QIcon::fromTheme(listPath[i]);
                if (!icon.isNull()) {
                    newIcons[i] = icon;
                }
            }
        }
        icons = newIcons;
    }
}

void PluginEmblemManagerPrivate::clearEmblemIconsMap()
{
    mutexMap.lock();
    mapIcons.clear();
    mutexMap.unlock();
}

void PluginEmblemManagerPrivate::asyncUpdateEmblemIconsFromPlugin(const DUrl &fileUrl, int systemIconCount)
{
    QPair<QString, int> pair(fileUrl.toLocalFile(), systemIconCount);
    if (!filePathQueue.contains(pair)) {
        filePathQueue.enqueue(pair);
        smt.release();
    }
}

void PluginEmblemManagerPrivate::run()
{
    if (!bInitPlugin) {
        bInitPlugin = true;
        // 加载外部插件(由于加载插件可能导致卡顿，故放到子线程)
        DFMGlobal::autoInitExtPluginManager();
        // 判断是否存在角标对象，如果不存在，线程退出
        if (DFMExtPluginManager::instance().emblemIcons().isEmpty()) {
            bInitPlugin = false;
            return;
        }
        bHaveEmblemObj = true;
    }

    while (bWork) {
        smt.acquire();
        QPair<QString, int> data = filePathQueue.dequeue();
        // 角标总数小于kMaxEmblemCount时，加载插件角标
        if (data.second < kMaxEmblemCount) {

            QStringList newIcons = {QString(), QString(), QString(), QString()};
            bool bHaveIcon = false;

            // 遍历角标插件对象
            for (auto plugin : DFMExtPluginManager::instance().emblemIcons()) {
                if (!plugin)
                    continue;
                // 获得插件中通过emblemIcons接口设置的角标
                getEmblemIcons(plugin, data, bHaveIcon, newIcons);
                // 获取插件中通过locationEmblemIcons接口设置的角标
                getLocationEmblemIcons(plugin, data, bHaveIcon, newIcons);
            }

            // 将从插件中拿到的角标缓存到容器
            cacheEmblemToMap(bHaveIcon, data, newIcons);
        }
    }
}

void PluginEmblemManagerPrivate::startWork()
{
    bWork = true;
    if (!isRunning())
        start();
}

void PluginEmblemManagerPrivate::stopWork()
{
    bWork = false;
    wait();
}

void PluginEmblemManagerPrivate::getEmblemIcons(QSharedPointer<DFMExtEmblemIconPlugin> plugin, const QPair<QString, int> &data,
                                                   bool &bHaveIcon, QStringList &newIcons)
{
    DFMExtEmblemIconPlugin::IconsType emblems = plugin->emblemIcons(data.first.toStdString());
    if (!emblems.empty()) {
        bHaveIcon = true;
        size_t len = emblems.size() < kMaxEmblemCount ? emblems.size() : kMaxEmblemCount;
        for (int i = 0, pos = 0; i < static_cast<int>(len); ++i) {
            pos = data.second + i;
            if (pos < newIcons.size()) {
                newIcons[data.second + i] = QString::fromStdString(emblems[static_cast<size_t>(i)]);
            }
        }
    }
}

void PluginEmblemManagerPrivate::getLocationEmblemIcons(QSharedPointer<DFMExtEmblemIconPlugin> plugin, const QPair<QString, int> &data, bool &bHaveIcon, QStringList &newIcons)
{
    DFMExtEmblem icon = plugin->locationEmblemIcons(data.first.toStdString(), data.second);
    std::vector<DFMExtEmblemIconLayout> veIcon = icon.emblems();
    if (!veIcon.empty()) { // 拿到了角标
        bHaveIcon = true;
        size_t len = veIcon.size();
        for (size_t i = 0; i < len; ++i) {
            DFMExtEmblemIconLayout iconLayout = veIcon[i];
            int nType = static_cast<int>(iconLayout.locationType());
            if (nType < newIcons.size()) {
                newIcons[nType] = QString::fromStdString(iconLayout.iconPath());
            }
        }
    }
}

void PluginEmblemManagerPrivate::cacheEmblemToMap(bool &bHaveIcon, const QPair<QString, int> &data, const QStringList &newIcons)
{
    if (bHaveIcon) {
        bool bSame = true;
        mutexMap.lock();
        if (mapIcons.contains(data.first)) {
            QStringList oldIcons = mapIcons[data.first];
            int size = oldIcons.size();
            for (int i = 0; i < size; ++i) {
                if (oldIcons[i] != newIcons[i]) {
                    bSame = false;
                    mapIcons.insert(data.first, newIcons);
                    break;
                }
            }
        } else {
            mapIcons.insert(data.first, newIcons);
            bSame = false;
        }
        mutexMap.unlock();

        // 插件角标有变化，通知更新
        if (!bSame) {
            if (updateTimer) {
                QMetaObject::invokeMethod(updateTimer, "start", Qt::QueuedConnection);
            } else {
                updateTimer = new QTimer(this);
                updateTimer->moveToThread(qApp->thread());
                updateTimer->setSingleShot(true);
                updateTimer->setInterval(kEmblemUpdateTime);
                connect(updateTimer, &QTimer::timeout, this, &PluginEmblemManagerPrivate::updateTimerTimeout);
                QMetaObject::invokeMethod(updateTimer, "start", Qt::QueuedConnection);
            }
        }
    }
}

void PluginEmblemManagerPrivate::updateTimerTimeout()
{
    emit q->updatePluginEmblem();
}

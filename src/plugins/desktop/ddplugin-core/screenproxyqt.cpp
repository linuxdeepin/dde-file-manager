/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "screenproxyqt.h"
#include "screenqt.h"
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QtConcurrent>
#include <QDebug>
#include <QThreadPool>

ScreenProxyQt::ScreenProxyQt(QObject *parent)
    : dfmbase::AbstractScreenProxy (parent)
{

}

QList<dfmbase::AbstractScreen *> ScreenProxyQt::allScreen()
{
    if (!screenList.isEmpty()) //提前退出
        return screenList;

    QList<QScreen*> srcScreen{qApp->screens()};
    for(auto val: srcScreen) {
        auto ascreen = new ScreenQt(val);
        screenList.append(ascreen);
    }

    QObject::connect(qApp, &QGuiApplication::screenAdded, this, [=](QScreen *screen) {
        for (auto val: screenList) {
            auto screenPointer = qobject_cast<ScreenQt*>(val);
            if (screenPointer->qScreen() == screen) {
                emit AbstractScreenProxy::screenAdded(screenPointer);
            }
        }
    }, Qt::UniqueConnection); //找到抽象节点并转发

    QObject::connect(qApp, &QGuiApplication::screenRemoved, this, [=](QScreen *screen) {
        for (auto val: screenList) {
            auto screenPointer = qobject_cast<ScreenQt*>(val);
            if (screenPointer->qScreen() == screen) {
                emit AbstractScreenProxy::screenRemoved(screenPointer);
            }
        }
    }, Qt::UniqueConnection); //找到抽象节点并转发

    return screenList;
}

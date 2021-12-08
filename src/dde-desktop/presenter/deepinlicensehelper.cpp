/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "deepinlicensehelper.h"

#include "dbus/licenceInterface.h"

#include <QApplication>
#include <QtConcurrent>

class DeepinLicenseHelperGlobal : public DeepinLicenseHelper {};
Q_GLOBAL_STATIC(DeepinLicenseHelperGlobal, deepinLicenseHelperGlobal)

DeepinLicenseHelper *DeepinLicenseHelper::instance()
{
    return deepinLicenseHelperGlobal;
}

DeepinLicenseHelper::DeepinLicenseHelper(QObject *parent) : QObject(parent)
{
    Q_ASSERT(qApp->thread() == thread());
    m_reqTimer.setInterval(500);
    m_reqTimer.setSingleShot(true);

    connect(&m_reqTimer, &QTimer::timeout, this, &DeepinLicenseHelper::requetLicenseState);
}

DeepinLicenseHelper::~DeepinLicenseHelper()
{
    m_work.waitForFinished();
    delete m_licenseInterface;
    m_licenseInterface = nullptr;
}

void DeepinLicenseHelper::init()
{
    std::call_once(initFlag, [this](){
        m_work = QtConcurrent::run(&DeepinLicenseHelper::createInterface);
    });
}

void DeepinLicenseHelper::delayGetState()
{
    m_reqTimer.start();
}

void DeepinLicenseHelper::requetLicenseState()
{
    if (m_work.isRunning() || !m_licenseInterface) {
        qWarning() << "requetLicenseState: interface is invalid.";
        return;
    }

    m_work = QtConcurrent::run(&DeepinLicenseHelper::getLicenseState, this);
}

void DeepinLicenseHelper::initFinshed(void *interface)
{
    Q_ASSERT(interface);
    Q_ASSERT(!m_licenseInterface);
    m_licenseInterface = static_cast<ComDeepinLicenseInterface *>(interface);
    connect(m_licenseInterface, &ComDeepinLicenseInterface::LicenseStateChange,
                     this, &DeepinLicenseHelper::licenseStateChanged);
    m_work.waitForFinished();
    qInfo() << "interface inited";

    requetLicenseState();
}

void DeepinLicenseHelper::createInterface()
{
    qInfo() << "create ComDeepinLicenseInterface...";
    auto licenseInterface = new ComDeepinLicenseInterface(
            "com.deepin.license",
            "/com/deepin/license/Info",
            QDBusConnection::systemBus());
    licenseInterface->moveToThread(qApp->thread());
    qInfo() << "create /com/deepin/license/Info org.freedesktop.DBus.Properties...";
    QMetaObject::invokeMethod(DeepinLicenseHelper::instance(), "initFinshed", Q_ARG(void *, licenseInterface));
}

void DeepinLicenseHelper::getLicenseState(DeepinLicenseHelper *self)
{
    Q_ASSERT(self);
    Q_ASSERT(self->m_licenseInterface);
    qInfo() << "get active state from com.deepin.license.Info";
    int state = self->m_licenseInterface->AuthorizationState();
    qInfo() << "Get AuthorizationState" << state;
    emit self->postLicenseState(state);
}

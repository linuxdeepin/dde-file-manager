// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deepinlicensehelper.h"

#include "licenceInterface.h"

#include <QApplication>
#include <QtConcurrent>

using namespace ddplugin_canvas;

class DeepinLicenseHelperGlobal : public DeepinLicenseHelper {};
Q_GLOBAL_STATIC(DeepinLicenseHelperGlobal, deepinLicenseHelperGlobal)

DeepinLicenseHelper *DeepinLicenseHelper::instance()
{
    return deepinLicenseHelperGlobal;
}

DeepinLicenseHelper::DeepinLicenseHelper(QObject *parent) : QObject(parent)
{
    Q_ASSERT(qApp->thread() == thread());
    reqTimer.setInterval(500);
    reqTimer.setSingleShot(true);

    connect(&reqTimer, &QTimer::timeout, this, &DeepinLicenseHelper::requestLicenseState);
}

DeepinLicenseHelper::~DeepinLicenseHelper()
{
    work.waitForFinished();
    delete licenseInterface;
    licenseInterface = nullptr;
}

void DeepinLicenseHelper::init()
{
    std::call_once(initFlag, [this](){
        work = QtConcurrent::run(&DeepinLicenseHelper::createInterface);
    });
}

void DeepinLicenseHelper::delayGetState()
{
    reqTimer.start();
}

void DeepinLicenseHelper::requestLicenseState()
{
    if (work.isRunning() || !licenseInterface) {
        qWarning() << "requetLicenseState: interface is invalid.";
        return;
    }

    work = QtConcurrent::run(&DeepinLicenseHelper::getLicenseState, this);
}

void DeepinLicenseHelper::initFinshed(void *interface)
{
    Q_ASSERT(interface);
    Q_ASSERT(!licenseInterface);
    licenseInterface = static_cast<ComDeepinLicenseInterface *>(interface);
    connect(licenseInterface, &ComDeepinLicenseInterface::LicenseStateChange,
                     this, &DeepinLicenseHelper::requestLicenseState);
    work.waitForFinished();
    qInfo() << "interface inited";

    requestLicenseState();
}

void DeepinLicenseHelper::createInterface()
{
    qInfo() << "create ComDeepinLicenseInterface...";
    auto licenseInterface = new ComDeepinLicenseInterface(
            "com.deepin.license",
            "/com/deepin/license/Info",
            QDBusConnection::systemBus());

    licenseInterface->moveToThread(qApp->thread());
    qInfo() << "create /com/deepin/license/Info ...";

    QMetaObject::invokeMethod(DeepinLicenseHelper::instance(), "initFinshed", Q_ARG(void *, licenseInterface));
}

void DeepinLicenseHelper::getLicenseState(DeepinLicenseHelper *self)
{
    Q_ASSERT(self);
    Q_ASSERT(self->licenseInterface);
    qInfo() << "get active state from com.deepin.license.Info";
    int state = self->licenseInterface->authorizationState();
    LicenseProperty prop = self->getServiceProperty();

    if (prop == LicenseProperty::Noproperty) {
        qInfo() << "no service property obtained,try to get AuthorizetionProperty";
        prop = self->getAuthorizationProperty();
    }

    qInfo() << "Get AuthorizationState" << state << prop;
    emit self->postLicenseState(state, prop);
}

DeepinLicenseHelper::LicenseProperty DeepinLicenseHelper::getServiceProperty()
{
    // 不直接使用serviceProperty接口，需要通过QVariant是否有效判断接口是否存在
    LicenseProperty prop = LicenseProperty::Noproperty;
    QVariant servProp = licenseInterface->property("ServiceProperty");
    if (!servProp.isValid()) {
        qInfo() << "no such property: ServiceProperty in license";
    } else {
        bool ok = false;
        prop = servProp.toInt(&ok) ? LicenseProperty::Secretssecurity : LicenseProperty::Noproperty;
        if (!ok) {
            qWarning() << "invalid value of serviceProperty" << servProp;
            prop = LicenseProperty::Noproperty;
        }
    }
    return prop;
}

DeepinLicenseHelper::LicenseProperty DeepinLicenseHelper::getAuthorizationProperty()
{
    // 不直接使用AuthorizationProperty接口，需要通过QVariant是否有效判断接口是否存在
    LicenseProperty prop = LicenseProperty::Noproperty;
    QVariant authprop = licenseInterface->property("AuthorizationProperty");
    if (!authprop.isValid()) {
        qInfo() << "no such property: AuthorizationProperty in license.";
    } else {
        bool ok = false;
        if (authprop.toInt(&ok) == 2)
            prop = LicenseProperty::Enterprise;
        else if (authprop.toInt(&ok) == 1)
            prop = LicenseProperty::Government;
        else
            prop = LicenseProperty::Noproperty;

        if (!ok) {
            qWarning() << "invalid value of AuthorizationProperty" << authprop;
            prop = LicenseProperty::Noproperty;
        }
    }
    return prop;
}

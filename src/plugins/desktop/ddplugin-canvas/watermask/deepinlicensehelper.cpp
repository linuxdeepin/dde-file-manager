// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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
        fmDebug() << "Starting background interface creation";
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
        fmWarning() << "requetLicenseState: interface is invalid.";
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
    fmInfo() << "License interface initialized and connected successfully";

    requestLicenseState();
}

void DeepinLicenseHelper::createInterface()
{
    fmInfo() << "Creating ComDeepinLicenseInterface for service: com.deepin.license";
    auto licenseInterface = new ComDeepinLicenseInterface(
            "com.deepin.license",
            "/com/deepin/license/Info",
            QDBusConnection::systemBus());

    licenseInterface->moveToThread(qApp->thread());
    fmInfo() << "License interface created successfully, moved to main thread";

    QMetaObject::invokeMethod(DeepinLicenseHelper::instance(), "initFinshed", Q_ARG(void *, licenseInterface));
}

void DeepinLicenseHelper::getLicenseState(DeepinLicenseHelper *self)
{
    Q_ASSERT(self);
    Q_ASSERT(self->licenseInterface);
    fmDebug() << "Starting license state retrieval from com.deepin.license.Info";
    int state = self->licenseInterface->authorizationState();
    LicenseProperty prop = self->getServiceProperty();

    if (prop == LicenseProperty::Noproperty) {
        fmInfo() << "No service property obtained, trying to get AuthorizationProperty";
        prop = self->getAuthorizationProperty();
    }

    fmInfo() << "License state retrieved - AuthorizationState:" << state << ", Property:" << static_cast<int>(prop);
    emit self->postLicenseState(state, prop);
}

DeepinLicenseHelper::LicenseProperty DeepinLicenseHelper::getServiceProperty()
{
    // 不直接使用serviceProperty接口，需要通过QVariant是否有效判断接口是否存在
    LicenseProperty prop = LicenseProperty::Noproperty;
    QVariant servProp = licenseInterface->property("ServiceProperty");
    if (!servProp.isValid()) {
        fmDebug() << "ServiceProperty not available in license interface";
    } else {
        bool ok = false;
        prop = servProp.toInt(&ok) ? LicenseProperty::Secretssecurity : LicenseProperty::Noproperty;
        if (!ok) {
            fmWarning() << "Invalid ServiceProperty value - cannot convert to integer:" << servProp;
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
        fmDebug() << "AuthorizationProperty not available in license interface";
    } else {
        bool ok = false;
        int value = authprop.toInt(&ok);
        static QMap<int, LicenseProperty> licenseProperty = {
            {1, LicenseProperty::Government},
            {2, LicenseProperty::Enterprise},
            {5, LicenseProperty::Office},
            {6, LicenseProperty::BusinessSystem},
            {7, LicenseProperty::Equipment},
        };

        prop = licenseProperty.value(value, LicenseProperty::Noproperty);
        if (!ok) {
            fmWarning() << "Unknown AuthorizationProperty value:" << value << "- defaulting to Noproperty";
            prop = LicenseProperty::Noproperty;
        }
    }
    return prop;
}

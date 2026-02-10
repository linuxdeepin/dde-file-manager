// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEEPINLICENSEHELPER_H
#define DEEPINLICENSEHELPER_H

#include "ddplugin_canvas_global.h"

#include <QObject>
#include <QFuture>
#include <QTimer>

#include <mutex>

class ComDeepinLicenseInterface;

namespace ddplugin_canvas {

class DeepinLicenseHelper : public QObject
{
    Q_OBJECT
public:
    enum LicenseState {
        Unauthorized = 0,
        Authorized,
        AuthorizedLapse,
        TrialAuthorized,
        TrialExpired
    };
    enum LicenseProperty{
        Noproperty = 0,
        Secretssecurity,
        Government,
        Enterprise,
        Office,
        BusinessSystem,
        Equipment
    };

    static DeepinLicenseHelper *instance();
    void init();
    void delayGetState();
signals:
    void postLicenseState(int state, int prop);

protected:
    explicit DeepinLicenseHelper(QObject *parent = nullptr);
    ~DeepinLicenseHelper();

private slots:
    void requestLicenseState();
    void initFinshed(void *interface);

private:
    static void createInterface();
    static void getLicenseState(DeepinLicenseHelper *);
    LicenseProperty getServiceProperty();
    LicenseProperty getAuthorizationProperty();
private:
    std::once_flag initFlag;
    QFuture<void> work;
    QTimer reqTimer;
    ComDeepinLicenseInterface *licenseInterface = nullptr;
};

}
#endif // DEEPINLICENSEHELPER_H

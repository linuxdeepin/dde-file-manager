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
#ifndef DEEPINLICENSEHELPER_H
#define DEEPINLICENSEHELPER_H

#include "dfm_desktop_service_global.h"

#include <QObject>
#include <QFuture>
#include <QTimer>

#include <mutex>

class ComDeepinLicenseInterface;

DSB_D_BEGIN_NAMESPACE

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
public:
    static DeepinLicenseHelper *instance();
    void init();
    void delayGetState();
signals:
    void licenseStateChanged(int);

public slots:

protected:
    explicit DeepinLicenseHelper(QObject *parent = nullptr);
    ~DeepinLicenseHelper();

private slots:
    void requetLicenseState();
    void initFinshed(void *interface);

private:
    static void createInterface();
    static void getLicenseState(DeepinLicenseHelper *);
private:
    std::once_flag initFlag;
    QFuture<void> work;
    QTimer reqTimer;
    ComDeepinLicenseInterface *licenseInterface = nullptr;
};

DSB_D_END_NAMESPACE

#endif // DEEPINLICENSEHELPER_H

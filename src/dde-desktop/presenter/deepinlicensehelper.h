// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEEPINLICENSEHELPER_H
#define DEEPINLICENSEHELPER_H

#include <QObject>
#include <QFuture>
#include <QTimer>

#include <mutex>

class ComDeepinLicenseInterface;
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
private:
    std::once_flag initFlag;
    QFuture<void> m_work;
    QTimer m_reqTimer;
    ComDeepinLicenseInterface *m_licenseInterface = nullptr;
};

#endif // DEEPINLICENSEHELPER_H

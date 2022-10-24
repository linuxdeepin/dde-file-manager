// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SMBINTEGRATIONSWITCHER_H
#define SMBINTEGRATIONSWITCHER_H

#include <QObject>
#include <dfmglobal.h>

class SmbIntegrationSwitcher : public QObject
{
    Q_OBJECT
public:
    static SmbIntegrationSwitcher *instance();
    void switchIntegrationMode(bool value);
    bool isIntegrationMode();
protected:
    explicit SmbIntegrationSwitcher(QObject *parent = nullptr);

signals:
    void smbIntegrationModeChanged(bool smbIntegration);
public slots:
private:
    bool smbIntegrationMode;
};

#endif // SMBINTEGRATIONSWITCHER_H

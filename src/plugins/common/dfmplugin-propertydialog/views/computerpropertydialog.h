// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERDETAILVIEW_H
#define COMPUTERDETAILVIEW_H

#include "dfmplugin_propertydialog_global.h"
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>

#include <DAbstractDialog>
#include <DDialog>

#include <QFrame>
#include <QGroupBox>
#include <QThread>

namespace dfmplugin_propertydialog {
class ComputerInfoThread : public QThread
{
    Q_OBJECT
public:
    enum ActiceState {
        kInactivated = 0,
        kActivated
    };
    enum ServiceInfo {
        kNoService = 0,
        kSecretsSecurity
    };
    enum AuthorizedInfo {
        kUnauthorized = 0,
        kGovernment,
        kEnterprise
    };

    explicit ComputerInfoThread(QObject *parent = nullptr);
    virtual ~ComputerInfoThread() override;

    void startThread();

    void stopThread();

protected:
    virtual void run() override;

private:
    void computerProcess();
    QString computerName() const;
    QString versionNum() const;
    QString edition() const;
    QString osBuild() const;
    QString systemType() const;
    QString cpuInfo() const;
    QString memoryInfo() const;

signals:
    void sigSendComputerInfo(QMap<ComputerInfoItem, QString> computerInfo);

private:
    QMap<ComputerInfoItem, QString> computerData {};
    bool threadStop { false };
};

class ComputerPropertyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit ComputerPropertyDialog(QWidget *parent = nullptr);
    ~ComputerPropertyDialog() override;

private:
    void iniUI();
    void iniThread();

signals:

public slots:
    void computerProcess(QMap<ComputerInfoItem, QString> computerInfo);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    DTK_WIDGET_NAMESPACE::DLabel *computer { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *computerIcon { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *basicInfo { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerName { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerVersionNum { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerEdition { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerOSBuild { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerCpu { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerMemory { nullptr };
    ComputerInfoThread *thread { nullptr };
};
}
#endif   // COMPUTERDETAILVIEW_H

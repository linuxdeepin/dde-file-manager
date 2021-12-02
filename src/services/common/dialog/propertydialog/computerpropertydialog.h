/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef COMPUTERDETAILVIEW_H
#define COMPUTERDETAILVIEW_H

#include "dfm_common_service_global.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"

#include <DAbstractDialog>
#include <DDialog>

#include <QFrame>
#include <QGroupBox>
#include <QThread>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_BEGIN_NAMESPACE

class ComputerInfoThread : public QThread
{
    Q_OBJECT
public:
    explicit ComputerInfoThread(QObject *parent = nullptr);
    virtual ~ComputerInfoThread() override;

    void startThread();

    void stopThread();

protected:
    virtual void run() override;

private:
    void computerProcess();

signals:
    void sigSendComputerInfo(QStringList computerInfo);

private:
    QStringList computerData {};
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
    void computerProcess(QStringList computerInfo);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    DLabel *computer { nullptr };
    DLabel *computerIcon { nullptr };
    DLabel *basicInfo { nullptr };
    KeyValueLabel *computerName { nullptr };
    KeyValueLabel *computerEdition { nullptr };
    KeyValueLabel *computerVersionNum { nullptr };
    KeyValueLabel *computerType { nullptr };
    KeyValueLabel *computerCpu { nullptr };
    KeyValueLabel *computerMemory { nullptr };
    ComputerInfoThread *thread { nullptr };
};
DSC_END_NAMESPACE
#endif   // COMPUTERDETAILVIEW_H

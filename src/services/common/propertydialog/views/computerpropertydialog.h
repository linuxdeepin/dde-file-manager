/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "propertydialog/property_defines.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"

#include <DAbstractDialog>
#include <DDialog>

#include <QFrame>
#include <QGroupBox>
#include <QThread>

CPY_BEGIN_NAMESPACE
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
    DTK_WIDGET_NAMESPACE::DLabel *computer { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *computerIcon { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *basicInfo { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerName { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerEdition { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerVersionNum { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerCpu { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *computerMemory { nullptr };
    ComputerInfoThread *thread { nullptr };
};
CPY_END_NAMESPACE
#endif   // COMPUTERDETAILVIEW_H

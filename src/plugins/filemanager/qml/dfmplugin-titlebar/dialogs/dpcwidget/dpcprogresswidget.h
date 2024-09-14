// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPCPROGRESSWIDGET_H
#define DPCPROGRESSWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <DWidget>

class QDBusInterface;

DWIDGET_BEGIN_NAMESPACE
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {

class DPCProgressWidget : public DTK_WIDGET_NAMESPACE::DWidget
{
    Q_OBJECT
public:
    explicit DPCProgressWidget(QWidget *parent = nullptr);
    void start();

Q_SIGNALS:
    void sigCompleted(bool success, const QString &msg);

private Q_SLOTS:
    void changeProgressValue();
    void onDiskPwdChanged(int result);

private:
    void initUI();
    void initConnect();

private:
    DTK_WIDGET_NAMESPACE::DWaterProgress *changeProgress { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *titleLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *msgLabel { nullptr };

    QTimer *progressTimer { nullptr };
    QSharedPointer<QDBusInterface> accessControlInter { nullptr };
};

}

#endif   // DPCPROGRESSWIDGET_H

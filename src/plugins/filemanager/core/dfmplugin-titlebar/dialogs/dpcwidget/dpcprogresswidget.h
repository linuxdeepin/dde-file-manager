/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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

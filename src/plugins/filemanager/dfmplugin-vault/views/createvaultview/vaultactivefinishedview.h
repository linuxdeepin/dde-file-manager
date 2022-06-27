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

#ifndef VAULTACTIVEFINISHEDVIEW_H
#define VAULTACTIVEFINISHEDVIEW_H

#include "dfmplugin_vault_global.h"

#include <dtkwidget_global.h>

#include <polkit-qt5-1/PolkitQt1/Authority>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QGridLayout;
class QVBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DWaterProgress;
class DLabel;
DWIDGET_END_NAMESPACE

namespace dfmplugin_vault {
class VaultActiveFinishedView : public QWidget
{
    Q_OBJECT
public:
    explicit VaultActiveFinishedView(QWidget *parent = nullptr);

    void setFinishedBtnEnabled(bool b);

signals:
    void sigAccepted();

public slots:
    //! 连接创建保险箱返回信号
    void slotEncryptComplete(int nState);

private slots:
    void slotEncryptVault();
    void slotTimeout();
    //! 异步授权时，此函数接收授权完成的结果
    void slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result);

protected:
    void showEvent(QShowEvent *event) override;

private:
    QWidget *widgetOne { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *encryVaultImageLabel { nullptr };

    QWidget *widgetTow { nullptr };
    DTK_WIDGET_NAMESPACE::DWaterProgress *waterProgress { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsLabelone { nullptr };

    QWidget *widgetThree { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *encryptFinishedImageLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *tipsThree { nullptr };

    QPushButton *finishedBtn { nullptr };

    QVBoxLayout *vLayout { nullptr };

    //! 辅助进度条，实现加密过程效果
    QTimer *timer { nullptr };
};
}
#endif   // VAULTACTIVEFINISHEDVIEW_H

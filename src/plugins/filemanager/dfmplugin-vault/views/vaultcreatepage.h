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

#ifndef VAULTACTIVEVIEW_H
#define VAULTACTIVEVIEW_H

#include "dfmplugin_vault_global.h"

#include "vaultpagebase.h"

class QStackedWidget;
class QMouseEvent;

namespace dfmplugin_vault {
class VaultActiveStartView;
class VaultActiveSetUnlockMethodView;
class VaultActiveFinishedView;
class VaultActiveSaveKeyFileView;
class VaultActiveView : public VaultPageBase
{
public:
    explicit VaultActiveView(QWidget *parent = nullptr);

protected:
    //! 重写基类关闭事件
    void closeEvent(QCloseEvent *event) override;

    void showEvent(QShowEvent *event) override;

private slots:
    void slotNextWidget();

private:
    //! 窗口容器
    QStackedWidget *stackedWidget { nullptr };

    VaultActiveStartView *startVaultWidget { nullptr };
    VaultActiveSetUnlockMethodView *setUnclockMethodWidget { nullptr };
    VaultActiveFinishedView *activeVaultFinishedWidget { nullptr };
    VaultActiveSaveKeyFileView *saveKeyFileWidget { nullptr };
};
}
#endif   //! VAULTACTIVEVIEW_H

/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef DFMVAULTACTIVEVIEW_H
#define DFMVAULTACTIVEVIEW_H

#include "dfmvaultpagebase.h"

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QMouseEvent;
QT_END_NAMESPACE

class DFMVaultActiveStartView;
class DFMVaultActiveSetUnlockMethodView;
class DFMVaultActiveSaveKeyView;
class DFMVaultActiveFinishedView;
class DFMVaultActiveSaveKeyFileView;

DWIDGET_USE_NAMESPACE

class DFMVaultActiveView : public DFMVaultPageBase
{
public:
    static DFMVaultActiveView *getInstance()
    {
        static DFMVaultActiveView dlg;
        return &dlg;
    }

protected:
    // 重写基类关闭事件
    void closeEvent(QCloseEvent *event) override;

    void showEvent(QShowEvent *event) override;

private slots:
    void slotNextWidget();

private:
    explicit DFMVaultActiveView(QWidget *parent = nullptr);

    // 复原操作
    void setBeginingState();

private:
    // 窗口容器
    QStackedWidget                          *m_pStackedWidget;

    DFMVaultActiveStartView                 *m_pStartVaultWidget;
    DFMVaultActiveSetUnlockMethodView       *m_pSetUnclockMethodWidget;
    DFMVaultActiveSaveKeyView               *m_SaveKeyWidget;   //! 弃用
    DFMVaultActiveFinishedView              *m_ActiveVaultFinishedWidget;
    DFMVaultActiveSaveKeyFileView           *m_SaveKeyFileWidget;
};

#endif // DFMVAULTACTIVEVIEW_H

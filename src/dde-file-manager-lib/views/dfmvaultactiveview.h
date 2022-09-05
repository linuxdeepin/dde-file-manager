// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    explicit DFMVaultActiveView(QWidget *parent = nullptr);
    ~DFMVaultActiveView();

protected:
    // 重写基类关闭事件
    void closeEvent(QCloseEvent *event) override;

    void showEvent(QShowEvent *event) override;

private slots:
    void slotNextWidget();

private:
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

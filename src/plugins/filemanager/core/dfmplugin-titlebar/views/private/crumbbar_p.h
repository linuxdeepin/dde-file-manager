// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CrumbBar_P_H
#define CrumbBar_P_H

#include "dfmplugin_titlebar_global.h"
#include "utils/crumbinterface.h"

#include <DPushButton>
#include <DListView>

#include <QPushButton>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

namespace dfmplugin_titlebar {
class UrlPushButton;
class CrumbBar;
class CrumbModel;
class CrumbBarPrivate
{
    friend class CrumbBar;
    CrumbBar *const q;

    QList<UrlPushButton *> navButtons;
    QHBoxLayout *crumbBarLayout;
    QPoint clickedPos;
    bool clickableAreaEnabled { false };
    QUrl lastUrl;
    bool hoverFlag { false };   // 鼠标是否悬停在按钮上
    bool popupVisible { false };

    CrumbInterface *crumbController { nullptr };

public:
    explicit CrumbBarPrivate(CrumbBar *qq);
    virtual ~CrumbBarPrivate();

    void clearCrumbs();
    void updateController(const QUrl &url);
    void setClickableAreaEnabled(bool enabled);
    void writeUrlToClipboard(const QUrl &url);
    UrlPushButton *buttonAt(QPoint pos) const;

private:
    void initUI();
    void initData();
    void initConnections();

    void appendWidget(QWidget *widget, int stretch = 0);
    void updateButtonVisibility();
};
}

#endif   // CrumbBar_P_H

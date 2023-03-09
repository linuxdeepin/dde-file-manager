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

// for first icon item icon AlignCenter...
class IconItemDelegate : public DStyledItemDelegate
{
public:
    explicit IconItemDelegate(QAbstractItemView *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class CrumbBar;
class CrumbModel;
class CrumbBarPrivate
{
    friend class CrumbBar;
    CrumbBar *const q;

    DPushButton leftArrow;
    DPushButton rightArrow;
    DListView crumbView;
    CrumbModel *crumbModel { nullptr };
    QHBoxLayout *crumbBarLayout;
    QPoint clickedPos;
    bool clickableAreaEnabled { false };
    QUrl lastUrl;

    CrumbInterface *crumbController { nullptr };

public:
    explicit CrumbBarPrivate(CrumbBar *qq);
    virtual ~CrumbBarPrivate();

    void clearCrumbs();
    void checkArrowVisiable();
    void updateController(const QUrl &url);
    void setClickableAreaEnabled(bool enabled);
    void writeUrlToClipboard(const QUrl &url);

private:
    void initUI();
    void initData();
    void initConnections();
};
}

#endif   // CrumbBar_P_H

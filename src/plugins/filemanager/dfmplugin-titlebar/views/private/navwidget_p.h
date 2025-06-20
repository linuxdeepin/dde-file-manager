// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVWIDGET_P_H
#define NAVWIDGET_P_H

#include "dfmplugin_titlebar_global.h"
#include "utils/historystack.h"

#include <dtkwidget_global.h>

#include <QObject>
#include <QHBoxLayout>

#include <memory>

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
DWIDGET_END_NAMESPACE

namespace dfmplugin_titlebar {
class NavWidget;
class NavWidgetPrivate : public QObject
{
    Q_OBJECT
    friend class NavWidget;

public:
    static constexpr int kMaxStackCount = UINT16_MAX;

    explicit NavWidgetPrivate(NavWidget *qq);
    void updateBackForwardButtonsState();

private:
    NavWidget *const q;
    DTK_WIDGET_NAMESPACE::DIconButton *navBackButton { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *navForwardButton { nullptr };
    QHBoxLayout *hboxLayout { nullptr };
    QUrl currentUrl {};
    std::shared_ptr<HistoryStack> curNavStack { nullptr };
    QList<std::shared_ptr<HistoryStack>> allNavStacks;
};
}

#endif   // NAVWIDGET_P_H

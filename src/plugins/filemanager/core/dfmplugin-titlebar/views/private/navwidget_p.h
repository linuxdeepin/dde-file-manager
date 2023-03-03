// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NAVWIDGET_P_H
#define NAVWIDGET_P_H

#include "dfmplugin_titlebar_global.h"
#include "utils/historystack.h"

#include <DButtonBox>
#include <QObject>
#include <QHBoxLayout>

#include <memory>

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
    Dtk::Widget::DButtonBox *buttonBox { nullptr };
    Dtk::Widget::DButtonBoxButton *navBackButton { nullptr };
    Dtk::Widget::DButtonBoxButton *navForwardButton { nullptr };
    QHBoxLayout *hboxLayout { nullptr };
    QUrl currentUrl {};
    std::shared_ptr<HistoryStack> curNavStack { nullptr };
    QList<std::shared_ptr<HistoryStack>> allNavStacks;
};
}

#endif   // NAVWIDGET_P_H

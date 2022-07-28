/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

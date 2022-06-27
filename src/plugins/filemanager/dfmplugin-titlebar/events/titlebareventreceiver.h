/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef TITLEBAREVENTRECEIVER_H
#define TITLEBAREVENTRECEIVER_H

#include "dfmplugin_titlebar_global.h"

#include <QObject>

namespace dfmplugin_titlebar {

class TitleBarEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TitleBarEventReceiver)

public:
    static TitleBarEventReceiver *instance();

public slots:
    void handleTabAdded(quint64 windowId);
    void handleTabChanged(quint64 windowId, int index);
    void handleTabMoved(quint64 windowId, int from, int to);
    void handleTabRemovd(quint64 windowId, int index);
    void handleStartSpinner(quint64 windowId);
    void handleStopSpinner(quint64 windowId);
    void handleShowFilterButton(quint64 windowId, bool visible);
    void handleViewModeChanged(quint64 windowId, int mode);

private:
    explicit TitleBarEventReceiver(QObject *parent = nullptr);
};

}

#endif   // TITLEBAREVENTRECEIVER_H

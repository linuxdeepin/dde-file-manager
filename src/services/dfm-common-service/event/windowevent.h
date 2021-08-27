/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#ifndef DFMWINDOWEVENT_H
#define DFMWINDOWEVENT_H

#include "dfmevent.h"

class DFMWindowEventData
{
    int m_winIdx;
    QUrl m_target;

public:

    DFMWindowEventData(int winIdx, const QUrl &target);

    DFMWindowEventData(const DFMWindowEventData &data);

    int winIdx() const;
    void setWinIdx(int winIdx);

    QUrl target() const;
    void setTarget(const QUrl &target);
};

class DFMWindowEvent: public DFMEvent
{
    //操作的数据列表
    QList<DFMWindowEventData> m_data;

public:
    enum Type{
        ChangeCurrentUrl = DFMEvent::WindowEvent + 1,
        OpenNewWindow,
        OpenNewTab,
        OpenUrl,
        MenuAction,
        NavBack,
        NavForward,
        SaveOperator,
        CleanSaveOperator,
        Revocation,
    }; Q_ENUM(Type)

    explicit DFMWindowEvent(int type);

    DFMWindowEvent &operator = (const DFMWindowEvent &event);

    void setData(QList<DFMWindowEventData> data);

    QList<DFMWindowEventData> data() const;
};

typedef QSharedPointer<DFMWindowEvent> DFMWindowEventPointer;

#endif // DFMWINDOWEVENT_H

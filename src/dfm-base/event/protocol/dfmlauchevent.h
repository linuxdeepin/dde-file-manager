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
#ifndef DFMLAUCHEVENT_H
#define DFMLAUCHEVENT_H

#include "event/protocol/dfmevent.h"

#include <QUrl>

class DFMLauchEventData {
    QUrl source;
    QUrl target;
public:

    DFMLauchEventData(const DFMLauchEventData &data)
    {
        source = data.source;
        target = data.target;
    }
};

class DFMLauchEvent: public DFMEvent
{
public:
    enum Type{
        OpenFile = DFMEvent::LauchEvent + 1,
        OpenFileByApp,
        OpenUrlByTerminal,
        CompressFile,
        DecompressFile,
        DecompressFileHere,
        WriteUrlToClipboard,
    }; Q_ENUM(Type)

    DFMLauchEvent(int type);

    QList<DFMLauchEventData> getDatas() const{
        return m_datas;
    }

private:
    QList<DFMLauchEventData> m_datas; //操作的数据列表
};

typedef QSharedPointer<DFMLauchEvent> DFMLauchEventPointer;
#endif // DFMLAUCHEVENT_H

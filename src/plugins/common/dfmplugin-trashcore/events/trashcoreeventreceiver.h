/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHCOREEVENTRECEIVER_H
#define TRASHCOREEVENTRECEIVER_H

#include "dfmplugin_trashcore_global.h"

#include <QObject>

namespace dfmplugin_trashcore {

class TrashCoreEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashCoreEventReceiver)

public:
    static TrashCoreEventReceiver *instance();

public slots:
    void handleEmptyTrash(const quint64 windowId = 0);

private:
    explicit TrashCoreEventReceiver(QObject *parent = nullptr);
};

}

#endif   // COREEVENTRECEIVER_H

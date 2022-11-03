/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef TRASHCOREEVENTSENDER_H
#define TRASHCOREEVENTSENDER_H

#include "dfmplugin_trashcore_global.h"
#include "dfm_base_global.h"

#include <QObject>
#include <QSharedPointer>

namespace dfmbase {
class AbstractFileWatcher;
}

namespace dfmplugin_trashcore {

class TrashCoreEventSender final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashCoreEventSender)

public:
    static TrashCoreEventSender *instance();

private slots:
    void sendTrashStateChangedDel();
    void sendTrashStateChangedAdd();

private:
    explicit TrashCoreEventSender(QObject *parent = nullptr);
    void initTrashWatcher();

private:
    QSharedPointer<DFMBASE_NAMESPACE::AbstractFileWatcher> trashFileWatcher = nullptr;
    bool isEmpty { false };
};

}

#endif   // TRASHCOREEVENTSENDER_H

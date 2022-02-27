/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef TRASHCOREUNICASTRECEIVER_H
#define TRASHCOREUNICASTRECEIVER_H

#include "dfmplugin_trashcore_global.h"
#include "services/filemanager/titlebar/titlebar_defines.h"

#include <QObject>

DPTRASHCORE_BEGIN_NAMESPACE

class TrashCoreUnicastReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashCoreUnicastReceiver)

public:
    static TrashCoreUnicastReceiver *instance();
    void connectService();

public slots:
    bool invokeIsEmpty();

private:
    explicit TrashCoreUnicastReceiver(QObject *parent = nullptr);
};

DPTRASHCORE_END_NAMESPACE

#endif   // TRASHCOREUNICASTRECEIVER_H

/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef ABSTRACTCANVASMANAGER_H
#define ABSTRACTCANVASMANAGER_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE
class AbstractCanvasManager : public QObject
{
    Q_OBJECT
public:
    explicit AbstractCanvasManager(QObject *parent = nullptr);

signals:

public slots:
};

DFMBASE_END_NAMESPACE
#endif   // ABSTRACTCANVASMANAGER_H

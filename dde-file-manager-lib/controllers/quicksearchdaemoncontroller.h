/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     shihua <tangtong@deepin.com>
 *
 * Maintainer: shihua <tangtong@deepin.com>
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
#ifndef QUICKSEARCHDAEMONCONTROLLER_H
#define QUICKSEARCHDAEMONCONTROLLER_H

#include <memory>


#include <QObject>
#include <QVariant>

#include "../interfaces/durl.h"
#include "interface/quicksearchdaemon_interface.h"

class QuickSearchDaemonController final : public QObject
{
    Q_OBJECT

public:
    QuickSearchDaemonController(QObject *const parent = nullptr);
    virtual ~QuickSearchDaemonController() = default;

    QuickSearchDaemonController(const QuickSearchDaemonController &) = delete;
    QuickSearchDaemonController &operator=(const QuickSearchDaemonController &) = delete;

    QList<QString> search(const DUrl &path_for_searching, const QString &key);

private:
    std::unique_ptr<QuickSearchDaemonInterface> interface_ptr{ nullptr };
};


#endif // QUICKSEARCHDAEMONCONTROLLER_H

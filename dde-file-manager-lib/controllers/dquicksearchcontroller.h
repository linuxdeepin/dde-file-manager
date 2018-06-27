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
#ifndef DQUICKSEARCHCONTROLLER_H
#define DQUICKSEARCHCONTROLLER_H

#include <memory>

#include "interface/quicksearchdaemon_interface.h"


#include <QObject>


class DQuickSearchController final : public QObject
{
    Q_OBJECT

public:
    DQuickSearchController(QObject *const parent = nullptr);
    virtual ~DQuickSearchController() = default;

    DQuickSearchController(const DQuickSearchController &) = delete;
    DQuickSearchController &operator=(const DQuickSearchController &) = delete;

    QList<QString> search(const QString &local_path, const QString &key_words);


    static inline DQuickSearchController *instance()
    {
        static DQuickSearchController *const controllter{ new DQuickSearchController };
        return controllter;
    }

private:
    std::unique_ptr<QuickSearchDaemonInterface> m_interface{ nullptr };
};






#endif // DQUICKSEARCHCONTROLLER_H

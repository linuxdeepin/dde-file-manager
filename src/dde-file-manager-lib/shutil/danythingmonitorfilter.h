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

#ifndef DANYTHINGMONITORFILTER_H
#define DANYTHINGMONITORFILTER_H


#include <QObject>
#include <QScopedPointer>

#include "durl.h"


class DAnythingMonitorFilterPrivate;
class DAnythingMonitorFilter final : public QObject
{
    Q_OBJECT
public:
    explicit DAnythingMonitorFilter(QObject *const parent = nullptr);
    virtual ~DAnythingMonitorFilter();

    DAnythingMonitorFilter(const DAnythingMonitorFilter &) = delete;
    DAnythingMonitorFilter &operator=(const DAnythingMonitorFilter &) = delete;

    bool whetherFilterCurrentPath(const QByteArray &local_path);

    static inline DAnythingMonitorFilter *instance()
    {
        static DAnythingMonitorFilter *singleton{ new DAnythingMonitorFilter };
        return singleton;
    }

private:
    QScopedPointer<DAnythingMonitorFilterPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DAnythingMonitorFilter)
};

#endif // DANYTHINGMONITORFILTER_H

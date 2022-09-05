// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

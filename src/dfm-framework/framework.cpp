/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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
#include "framework.h"
#include "backtrace/backtrace.h"
#include "dfm-framework/log/frameworklog.h"
#include "dfm-framework/lifecycle/lifecycle.h"

DPF_BEGIN_NAMESPACE

class FrameworkPrivate
{
    Q_DECLARE_PUBLIC(Framework)

    explicit FrameworkPrivate(Framework *dd);

    // Plugin lifeCycle manager.
    QScopedPointer<LifeCycle> lifeCycle;

    bool bInitialized = false;

    Framework *const q_ptr = nullptr;
};

FrameworkPrivate::FrameworkPrivate(Framework *dd)
    : q_ptr(dd)
{

}

Framework &Framework::instance()
{
    static Framework ins;
    return ins;
}


bool Framework::initialize()
{
    if (d->bInitialized) {
        qDebug() << "Frame work has been initialized!";
        return true;
    }

    backtrace::initbacktrace();

    FrameworkLog::initialize();

    // It will be true after all inner moudules initialized
    // successfully.
    d->bInitialized = true;

    return true;
}

bool Framework::start()
{
    // TODO(anyone):Start plugin after initialized,
    // thus plugin logic will be run
    return true;
}

const LifeCycle &Framework::lifeCycle() const
{
    return *d->lifeCycle;
}

Framework::Framework() :
    d(new FrameworkPrivate(this))
{
    d->lifeCycle.reset(new LifeCycle());
}

DPF_END_NAMESPACE

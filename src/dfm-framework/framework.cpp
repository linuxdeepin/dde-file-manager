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

DPF_BEGIN_NAMESPACE

/*!
 * \brief Framework::Framework
 * include all inner modules, all interfaces fetch form here.
 * \param parent
 */
Framework::Framework(QObject *parent) : QObject(parent)
{

}

/*!
 * \brief Framework::initialize
 * initialize inner modules.
 * \return bool
 */
bool Framework::initialize()
{
    // TODO(mozart): do more init here.
    backtrace::initbacktrace();

    return true;
}

DPF_END_NAMESPACE

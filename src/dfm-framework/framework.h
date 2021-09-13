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
#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/lifecycle/lifecycle.h"

#include <QObject>

DPF_BEGIN_NAMESPACE

class FrameworkPrivate;


/*!
 * \brief The Framework class
 */
class DPF_EXPORT Framework
{
    Q_DISABLE_COPY(Framework)
public:
    /*!
     * \brief Get framework instance.
     * \return
     */
    static Framework &instance();

    /*!
     * \brief Framework inner modules will be initialized
     * when it invoked,same for plugins.
     * \return
     */
    bool initialize();

    /*!
     * \brief Start framework after initialized.
     * \return
     */
    bool start();

    /*!
     * \brief Get plugin life cycle manager
     * \return
     */
    const LifeCycle &lifeCycle() const;

private:
    Framework();

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d), Framework)
    QScopedPointer<FrameworkPrivate> d;
};

#endif // FRAMEWORK_H

DPF_END_NAMESPACE

#define dpfInstance ::dpf::Framework::instance()

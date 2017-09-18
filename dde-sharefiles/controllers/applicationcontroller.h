/*
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

#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <TActionController>
#include "applicationhelper.h"


class T_CONTROLLER_EXPORT ApplicationController : public TActionController
{
    Q_OBJECT
public:
    ApplicationController();
    ApplicationController(const ApplicationController &other);
    virtual ~ApplicationController();

public slots:
    void staticInitialize();
    void staticRelease();

protected:
    virtual bool preFilter();
};

T_DECLARE_CONTROLLER(ApplicationController, applicationcontroller)

#endif // APPLICATIONCONTROLLER_H

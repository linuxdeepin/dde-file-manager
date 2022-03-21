/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef MENUSERVICE_P_H
#define MENUSERVICE_P_H
#include "dfm_common_service_global.h"

#include <dfm-base/interfaces/abstractscenecreator.h>
#include <services/common/menu/menuservice.h>

DSC_BEGIN_NAMESPACE

class MenuServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit MenuServicePrivate(MenuService *parent);
    void createSubscene(DFMBASE_NAMESPACE::AbstractSceneCreator *creator, DFMBASE_NAMESPACE::AbstractMenuScene *parent) const;

public:
    QHash<QString, DFMBASE_NAMESPACE::AbstractSceneCreator *> creators;

private:
    MenuService *q;
};

DSC_END_NAMESPACE
#endif   // MENUSERVICE_P_H

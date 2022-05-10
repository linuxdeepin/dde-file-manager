/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "custom/custommode_p.h"
#include "models/fileproxymodel.h"

DDP_ORGANIZER_USE_NAMESPACE

CustomModePrivate::CustomModePrivate(CustomMode *qq) : q(qq)
{

}

CustomMode::CustomMode(QObject *parent)
    : CanvasOrganizer(parent)
    , d(new CustomModePrivate(this))
{

}

int CustomMode::mode() const
{
    return OrganizerCreator::kCustom;
}

bool CustomMode::initialize(FileProxyModel *)
{
    return true;
}



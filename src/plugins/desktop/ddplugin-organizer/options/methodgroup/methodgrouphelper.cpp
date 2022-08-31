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
#include "methodgrouphelper.h"
#include "typemethodgroup.h"

using namespace ddplugin_organizer;

MethodGroupHelper::MethodGroupHelper(QObject *parent) : QObject(parent)
{

}

MethodGroupHelper *MethodGroupHelper::create(Classifier id)
{
    MethodGroupHelper *ret = nullptr;
    switch (id) {
    case kType:
        ret = new TypeMethodGroup();
        break;
    default:
        break;
    }

    return ret;
}

MethodGroupHelper::~MethodGroupHelper()
{
    release();
}

void MethodGroupHelper::release()
{

}

bool MethodGroupHelper::build()
{
    return false;
}

/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef ACCESSIBLE_INTERNAL_FUNCTIONS_H
#define ACCESSIBLE_INTERNAL_FUNCTIONS_H

//#define AC_SET_OBJECT_NAME(classObj,objectname)  classObj->setObjectName(objectname);

#ifdef ENABLE_ACCESSIBILITY
    #define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename) classObj->setAccessibleName(accessiblename);
    #define AC_SET_OBJECT_NAME(classObj,objectname)
#else
    #define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename)
    #define AC_SET_OBJECT_NAME(classObj,objectname)
#endif

#endif // ACCESSIBLE_INTERNAL_FUNCTIONS_H

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

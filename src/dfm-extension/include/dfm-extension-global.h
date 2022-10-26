/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef DDEFILEMANAGEREXTENSION_GLOBAL_H
#define DDEFILEMANAGEREXTENSION_GLOBAL_H

#define DFMEXT dfmext
#define BEGEN_DFMEXT_NAMESPACE namespace DFMEXT {
#define USING_DFMEXT_NAMESPACE using namespace DFMEXT;
#define END_DFMEXT_NAMESPACE }

#define DFM_FAKE_VIRTUAL
#define DFM_FAKE_OVERRIDE

/*
   Some classes do not permit copies to be made of an object. These
   classes contains a private copy constructor and assignment
   operator to disable copying (the compiler gives an error message).
*/
#define DFM_DISABLE_COPY(Class)    \
    Class(const Class &) = delete; \
    Class &operator=(const Class &) = delete;

#endif   // DDEFILEMANAGEREXTENSION_GLOBAL_H

// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

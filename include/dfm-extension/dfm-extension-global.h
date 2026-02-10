// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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

#define DFMEXT_VERSION_MAJOR 6
#define DFMEXT_VERSION_MINOR 0
#define DFMEXT_VERSION_PATCH 0

/*
   DFMEXT_VERSION is (major << 16) + (minor << 8) + patch.
*/
#define DFMEXT_VERSION DFMEXT_VERSION_CHECK(DFMEXT_VERSION_MAJOR, DFMEXT_VERSION_MINOR, DFMEXT_VERSION_PATCH)
/*
   can be used like #if (DFMEXT_VERSION >= DFMEXT_VERSION_CHECK(6, 0, 0))
*/
#define DFMEXT_VERSION_CHECK(major, minor, patch) ((major << 16) | (minor << 8) | (patch))

// TODO:  auto-generate
#define DFMEXT_INTERFACE_Menu
#define DFMEXT_INTERFACE_Emblem
#define DFMEXT_INTERFACE_Window
#define DFMEXT_INTERFACE_File
#endif   // DDEFILEMANAGEREXTENSION_GLOBAL_H

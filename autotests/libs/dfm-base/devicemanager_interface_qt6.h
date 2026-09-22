// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @brief Forwarder shim so that
 *        dfm-base/base/device/private/deviceproxymanager_p.h can be included
 *        from the autotest translation units: the private header does a
 *        quote-include of the qdbusxml2cpp-generated
 *        devicemanager_interface_qt6.h which lives in the dfm-base build
 *        directory.  Angle brackets are required here so the include does
 *        not resolve back to this shim itself; the generated header is found
 *        through the dfm6-base target's exported include directories.
 */
#ifndef UT_DEVICEMANAGER_INTERFACE_SHIM_H
#define UT_DEVICEMANAGER_INTERFACE_SHIM_H

#include <devicemanager_interface_qt6.h>

#endif   // UT_DEVICEMANAGER_INTERFACE_SHIM_H

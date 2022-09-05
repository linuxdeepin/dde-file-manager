// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UI_UTILS_XUTIL_H_
#define UI_UTILS_XUTIL_H_

class QWidget;

namespace QtX11 {
namespace utils {

// Restores the widget after it has been maximized or minimized.
void ShowNormalWindow(QWidget* widget);

}  // namespace utils
}  // namespace QtX11

#endif  // UI_UTILS_XUTIL_H_

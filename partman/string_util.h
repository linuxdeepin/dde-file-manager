// Copyright (c) 2016 Deepin Ltd. All rights reserved.
// Use of this source is governed by General Public License that can be found
// in the LICENSE file.

#ifndef INSTALLER_BASE_STRING_UTIL_H
#define INSTALLER_BASE_STRING_UTIL_H

#include <QString>

namespace PartMan {

// Parse a pattern |pattern| from string |str| and returns matched substring
// or an empty string.
// Note that a match group shall be specified in |pattern|.
QString RegexpLabel(const QString& pattern, const QString& str);

}  // namespace installer

#endif  // INSTALLER_BASE_STRING_UTIL_H

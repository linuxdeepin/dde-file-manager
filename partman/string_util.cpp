// Copyright (c) 2016 Deepin Ltd. All rights reserved.
// Use of this source is governed by General Public License that can be found
// in the LICENSE file.

#include "string_util.h"

#include <QRegularExpression>

namespace PartMan {

QString RegexpLabel(const QString& pattern, const QString& str) {
  QRegularExpression reg(pattern, QRegularExpression::MultilineOption);
  QRegularExpressionMatch match = reg.match(str);
  if (match.hasMatch()) {
    return match.captured(1);
  } else {
    return QString();
  }
}

}  // namespace installer

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMREGULAREXPRESSION_H
#define DFMREGULAREXPRESSION_H

#include <QString>

// Backport some static function.
// This class should be removed after dropping support for old Qt version.

class DFMRegularExpression
{
public:
    static QString checkWildcardAndToRegularExpression(const QString &pattern);
    static QString wildcardToRegularExpression(const QString &pattern);
    static inline QString anchoredPattern(const QString &expression) {
        return QLatin1String("\\A(?:")
               + expression
               + QLatin1String(")\\z");
    }
};

#endif // DFMREGULAREXPRESSION_H

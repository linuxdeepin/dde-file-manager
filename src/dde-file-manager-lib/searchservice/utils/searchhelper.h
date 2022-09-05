// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REGULAREXPRESSION_H
#define REGULAREXPRESSION_H

#include <QString>

class RegularExpression
{
public:
    static QString checkWildcardAndToRegularExpression(const QString &pattern);
    static QString wildcardToRegularExpression(const QString &pattern);
    static inline QString anchoredPattern(const QString &expression)
    {
        return QLatin1String("\\A(?:")
                + expression
                + QLatin1String(")\\z");
    }
};

class SearchHelper
{
public:
    static bool isHiddenFile(const QString &fileName, QHash<QString, QSet<QString>> &filters, const QString &searchPath);
};

#endif   // REGULAREXPRESSION_H

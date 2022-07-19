/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searchhelper.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QSet>

QString RegularExpression::checkWildcardAndToRegularExpression(const QString &pattern)
{
    if (!pattern.contains('*') && !pattern.contains('?'))
        return wildcardToRegularExpression('*' + pattern + '*');

    return wildcardToRegularExpression(pattern);
}

QString RegularExpression::wildcardToRegularExpression(const QString &pattern)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    return QRegularExpression::wildcardToRegularExpression(pattern);
#endif   // (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))

    const int wclen = pattern.length();
    QString rx;
    rx.reserve(wclen + wclen / 16);
    int i = 0;
    const QChar *wc = pattern.unicode();
#ifdef Q_OS_WIN
    const QLatin1Char nativePathSeparator('\\');
    const QLatin1String starEscape("[^/\\\\]*");
    const QLatin1String questionMarkEscape("[^/\\\\]");
#else
    const QLatin1Char nativePathSeparator('/');
    const QLatin1String starEscape("[^/]*");
    const QLatin1String questionMarkEscape("[^/]");
#endif
    while (i < wclen) {
        const QChar c = wc[i++];
        switch (c.unicode()) {
        case '*':
            rx += starEscape;
            break;
        case '?':
            rx += questionMarkEscape;
            break;
        case '\\':
#ifdef Q_OS_WIN
        case '/':
            rx += QLatin1String("[/\\\\]");
            break;
#endif
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
            rx += QLatin1Char('\\');
            rx += c;
            break;
        case '[':
            rx += c;
            // Support for the [!abc] or [!a-c] syntax
            if (i < wclen) {
                if (wc[i] == QLatin1Char('!')) {
                    rx += QLatin1Char('^');
                    ++i;
                }
                if (i < wclen && wc[i] == QLatin1Char(']'))
                    rx += wc[i++];
                while (i < wclen && wc[i] != QLatin1Char(']')) {
                    // The '/' appearing in a character class invalidates the
                    // regular expression parsing. It also concerns '\\' on
                    // Windows OS types.
                    if (wc[i] == QLatin1Char('/') || wc[i] == nativePathSeparator)
                        return rx;
                    if (wc[i] == QLatin1Char('\\'))
                        rx += QLatin1Char('\\');
                    rx += wc[i++];
                }
            }
            break;
        default:
            rx += c;
            break;
        }
    }
    return anchoredPattern(rx);
}

bool SearchHelper::isHiddenFile(const QString &fileName, QHash<QString, QSet<QString>> &filters, const QString &searchPath)
{
    if (!fileName.startsWith(searchPath) || fileName == searchPath)
        return false;

    QFileInfo fileInfo(fileName);
    if (fileInfo.isHidden())
        return true;

    const auto &fileParentPath = fileInfo.absolutePath();
    const auto &hiddenFileConfig = fileParentPath + "/.hidden";

    // 判断.hidden文件是否存在，不存在说明该路径下没有隐藏文件
    if (!QFile::exists(hiddenFileConfig))
        return isHiddenFile(fileParentPath, filters, searchPath);

    if (filters[fileParentPath].isEmpty()) {
        QFile file(hiddenFileConfig);
        if (!file.open(QFile::ReadOnly))
            return false;

        // 判断.hidden文件中的内容是否为空，空则表示该路径下没有隐藏文件
        if (file.isReadable() && file.size() > 0) {
            QByteArray data = file.readAll();
            file.close();

            const auto &hiddenFiles = QSet<QString>::fromList(QString(data).split('\n', QString::SkipEmptyParts));
            filters[fileParentPath] = hiddenFiles;
        } else {
            return isHiddenFile(fileParentPath, filters, searchPath);
        }
    }

    return filters[fileParentPath].contains(fileInfo.fileName())
            ? true
            : isHiddenFile(fileParentPath, filters, searchPath);
}

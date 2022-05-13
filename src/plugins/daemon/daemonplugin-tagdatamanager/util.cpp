/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "util.h"

static constexpr const char *const escaped_skim_str { "\\039" };
static constexpr const char skim_char { '\'' };

QString DAEMONPTAGDM_NAMESPACE::escapingEnSkim(const QString &source)
{
    if (source.isEmpty()) {
        return source;
    }

    QByteArray local8bits_str { source.toLocal8Bit() };
    QByteArray::const_iterator cbeg { local8bits_str.cbegin() };
    QByteArray::const_iterator cend { local8bits_str.cend() };

    QByteArray::const_iterator pos { std::find(cbeg, cend, skim_char) };

    while (pos != cend) {
        QByteArray::difference_type index { pos - cbeg };
        local8bits_str.remove(index, 1);

        if (pos == local8bits_str.cbegin()) {
            local8bits_str.push_front(escaped_skim_str);
            cbeg = local8bits_str.cbegin();
            cend = local8bits_str.cend();
            pos = std::find(cbeg, cend, skim_char);
        } else {
            local8bits_str.insert(index, escaped_skim_str);
            cbeg = local8bits_str.cbegin();
            cend = local8bits_str.cend();
            pos = std::find(cbeg, cend, skim_char);
        }
    }

    return QString::fromLocal8Bit(local8bits_str);
}

QString DAEMONPTAGDM_NAMESPACE::restoreEscapedEnSkim(const QString &source)
{
    if (source.isEmpty()) {
        return source;
    }

    QByteArray local8bits_str { source.toLocal8Bit() };

    while (local8bits_str.contains(escaped_skim_str)) {
        int index { local8bits_str.indexOf(escaped_skim_str) };
        local8bits_str.remove(index, 4);

        if (index != 0) {
            local8bits_str.insert(index, skim_char);
        } else {
            local8bits_str.push_front(skim_char);
        }
    }

    return QString::fromLocal8Bit(local8bits_str);
}

QUrl DAEMONPTAGDM_NAMESPACE::parentUrl(const QUrl &source)
{
    const QString &path = source.path();

    if (path == "/")
        return QUrl();
    QUrl reUrl;
    reUrl.setScheme(source.scheme());
    QStringList paths = path.split("/");
    paths.removeAt(0);
    if (!paths.isEmpty() && paths.last().isEmpty())
        paths.removeLast();

    if (!paths.isEmpty())
        paths.removeLast();

    QString curPath;
    curPath = std::accumulate(paths.begin(), paths.end(), QString(""), [](QString a, QString b) {
        return a + "/" + b;
    });

    if (curPath.isEmpty())
        curPath += "/";

    reUrl.setPath(curPath);

    return reUrl;
}

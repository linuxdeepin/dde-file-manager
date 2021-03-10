/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "tagutil.h"


#include <QDebug>

namespace Tag
{

const QMap<QString, QString> ColorsWithNames{
    { "#ffa503", "Orange"},
    { "#ff1c49", "Red"},
    { "#9023fc", "Purple"},
    { "#3468ff", "Navy-blue"},
    { "#00b5ff", "Azure"},
    { "#58df0a", "Grass-green"},
    { "#fef144", "Yellow"},
    { "#cccccc", "Gray" }
};

const QMap<QString, QColor> NamesWithColors{
    {"Orange", "#ffa503"},
    {"Red", "#ff1c49"},
    {"Purple", "#9023fc"},
    {"Navy-blue", "#3468ff"},
    {"Azure", "#00b5ff"},
    {"Grass-green", "#58df0a"},
    {"Yellow", "#fef144"},
    {"Gray", "#cccccc"}
};

// <TagColorName, IconName>
const  QMap<QString, QString> ColorNameWithIconName {
    {"Orange", "dfm_tag_orange"},
    {"Red", "dfm_tag_red"},
    {"Purple", "dfm_tag_purple"},
    {"Navy-blue", "dfm_tag_deepblue"},
    {"Azure", "dfm_tag_lightblue"},
    {"Grass-green", "dfm_tag_green"},
    {"Yellow", "dfm_tag_yellow"},
    {"Gray", "dfm_tag_gray"}
};

std::vector<QString> ColorName{
    "Orange",
    "Red",
    "Purple",
    "Navy-blue",
    "Azure",
    "Grass-green",
    "Yellow",
    "Gray"
};

const QMap<QString, QString> &ActualAndFakerName()
{
    ///###: Why has a faker name?
    ///###: pair.second represent the faker name,
    ///###: and them are used in translation.
    const static QMap<QString, QString> ActualAndFakerName{
        {"Orange", QObject::tr("Orange")},
        {"Red", QObject::tr("Red")},
        {"Purple", QObject::tr("Purple")},
        {"Navy-blue", QObject::tr("Navy-blue")},
        {"Azure", QObject::tr("Azure")},
        {"Grass-green", QObject::tr("Green")},
        {"Yellow", QObject::tr("Yellow")},
        {"Gray", QObject::tr("Gray")}
    };

    return ActualAndFakerName;
}






static constexpr const char *const escaped_skim_str{ "\\039" };
static constexpr const char skim_char{'\''};

QString escaping_en_skim(const QString &source) noexcept
{
    if (source.isEmpty()) {
        return source;
    }

    QByteArray local8bits_str{ source.toLocal8Bit() };
    QByteArray::const_iterator cbeg{ local8bits_str.cbegin() };
    QByteArray::const_iterator cend{ local8bits_str.cend() };

    QByteArray::const_iterator pos{ std::find(cbeg, cend, skim_char) };

    while (pos != cend) {
        QByteArray::difference_type index{ pos - cbeg };
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

#ifdef QT_DEBUG
    qDebug() << local8bits_str;
#endif //QT_DEBUG

    return QString::fromLocal8Bit(local8bits_str);
}

QString restore_escaped_en_skim(const QString &source) noexcept
{
    if (source.isEmpty()) {
        return source;
    }

    QByteArray local8bits_str{ source.toLocal8Bit() };

    while (local8bits_str.contains(escaped_skim_str)) {
        int index{ local8bits_str.indexOf(escaped_skim_str) };
        local8bits_str.remove(index, 4);

        if (index != 0) {
            local8bits_str.insert(index, skim_char);
        } else {
            local8bits_str.push_front(skim_char);
        }
    }

#ifdef QT_DEBUG
    qDebug() << local8bits_str;
#endif //QT_DEBUG

    return QString::fromLocal8Bit(local8bits_str);
}

}


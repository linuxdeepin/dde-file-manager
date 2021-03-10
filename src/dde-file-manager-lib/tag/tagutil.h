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

#ifndef TAGUTIL_H
#define TAGUTIL_H


#include <QMap>
#include <QColor>
#include <QObject>
#include <QString>

#include <vector>


///###: ODR!
namespace Tag
{
enum class ActionType : std::size_t {
    MakeFilesTags = 1,
    GetFilesThroughTag,
    GetTagsThroughFile,
    RemoveTagsOfFiles,
    DeleteTags,
    ChangeTagName,
    DeleteFiles,
    MakeFilesTagThroughColor,
    ChangeFilesName,
    GetAllTags = 10,
    BeforeMakeFilesTags,
    GetTagsColor,
    ChangeTagColor
};

extern const QMap<QString, QString> ColorsWithNames;
extern const QMap<QString, QColor> NamesWithColors;
extern const QMap<QString, QString> ColorNameWithIconName; // <TagColorName, IconName>

const QMap<QString, QString> &ActualAndFakerName();

extern std::vector<QString> ColorName;


extern QString escaping_en_skim(const QString &source) noexcept;
extern QString restore_escaped_en_skim(const QString &source) noexcept;



}


#endif // TAGUTIL_H

/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "customactiondata.h"

CustomActionData::CustomActionData()
    : actPosition(0),
      actNameArg(CustomActionDefines::kNoneArg),
      actCmdArg(CustomActionDefines::kNoneArg),
      actSeparator(CustomActionDefines::kNone)
{
}

CustomActionData::CustomActionData(const CustomActionData &other)
    : actComboPos(other.actComboPos),
      actPosition(other.actPosition),
      actNameArg(other.actNameArg),
      actCmdArg(other.actCmdArg),
      actName(other.actName),
      actIcon(other.actIcon),
      actCommand(other.actCommand),
      actSeparator(other.actSeparator),
      actChildrenActions(other.actChildrenActions)
{
}

CustomActionData &CustomActionData::operator=(const CustomActionData &other)
{
    if (this == &other)
        return *this;
    actNameArg = other.actNameArg;
    actCmdArg = other.actCmdArg;
    actName = other.actName;
    actComboPos = other.actComboPos;
    actPosition = other.actPosition;
    actSeparator = other.actSeparator;
    actIcon = other.actIcon;
    actCommand = other.actCommand;
    actChildrenActions = other.actChildrenActions;
    return *this;
}

bool CustomActionData::isMenu() const
{
    return !actChildrenActions.isEmpty();
}

bool CustomActionData::isAction() const
{
    return actChildrenActions.isEmpty();
}

QString CustomActionData::name() const
{
    return actName;
}

int CustomActionData::position(CustomActionDefines::ComboType combo) const
{
    auto it = actComboPos.find(combo);
    if (it != actComboPos.end())
        return it.value();
    else
        return actPosition;
}

int CustomActionData::position() const
{
    return actPosition;
}

QString CustomActionData::icon() const
{
    return actIcon;
}

QString CustomActionData::command() const
{
    return actCommand;
}

CustomActionDefines::Separator CustomActionData::separator() const
{
    return actSeparator;
}

QList<CustomActionData> CustomActionData::acitons() const
{
    return actChildrenActions;
}

CustomActionDefines::ActionArg CustomActionData::nameArg() const
{
    return actNameArg;
}

CustomActionDefines::ActionArg CustomActionData::commandArg() const
{
    return actCmdArg;
}

CustomActionEntry::CustomActionEntry()
{
}

CustomActionEntry::CustomActionEntry(const CustomActionEntry &other)
    : filePackage(other.filePackage),
      fileVersion(other.fileVersion),
      fileComment(other.fileComment),
      fileSign(other.fileSign),
      actFileCombo(other.actFileCombo),
      actMimeTypes(other.actMimeTypes),
      actExcludeMimeTypes(other.actExcludeMimeTypes),
      actSupportSchemes(other.actSupportSchemes),
      actNotShowIn(other.actNotShowIn),
      actSupportSuffix(other.actSupportSuffix),
      actData(other.actData)
{
}

CustomActionEntry &CustomActionEntry::operator=(const CustomActionEntry &other)
{
    if (this == &other)
        return *this;
    filePackage = other.filePackage;
    fileVersion = other.fileVersion;
    fileComment = other.fileComment;
    actFileCombo = other.actFileCombo;
    actMimeTypes = other.actMimeTypes;
    actExcludeMimeTypes = other.actExcludeMimeTypes;
    actSupportSchemes = other.actSupportSchemes;
    actNotShowIn = other.actNotShowIn;
    actSupportSuffix = other.actSupportSuffix;
    fileSign = other.fileSign;
    actData = other.actData;
    return *this;
}

QString CustomActionEntry::package() const
{
    return filePackage;
}

QString CustomActionEntry::version() const
{
    return fileVersion;
}

QString CustomActionEntry::comment() const
{
    return fileComment;
}

CustomActionDefines::ComboTypes CustomActionEntry::fileCombo() const
{
    return actFileCombo;
}

QStringList CustomActionEntry::mimeTypes() const
{
    return actMimeTypes;
}

QStringList CustomActionEntry::excludeMimeTypes() const
{
    return actExcludeMimeTypes;
}

QStringList CustomActionEntry::surpportSchemes() const
{
    return actSupportSchemes;
}

QStringList CustomActionEntry::notShowIn() const
{
    return actNotShowIn;
}

QStringList CustomActionEntry::supportStuffix() const
{
    return actSupportSuffix;
}

CustomActionData CustomActionEntry::data() const
{
    return actData;
}

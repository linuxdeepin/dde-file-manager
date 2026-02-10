// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcustomactiondata.h"

using namespace dfmplugin_menu;

DCustomActionData::DCustomActionData()
    : actionPosition(0),
      actionNameArg(DCustomActionDefines::kNoneArg),
      actionCmdArg(DCustomActionDefines::kNoneArg),
      actionSeparator(DCustomActionDefines::kNone)
{
}

DCustomActionData::DCustomActionData(const DCustomActionData &other)
    : comboPos(other.comboPos),
      actionPosition(other.actionPosition),
      actionNameArg(other.actionNameArg),
      actionCmdArg(other.actionCmdArg),
      actionName(other.actionName),
      actionIcon(other.actionIcon),
      actionCommand(other.actionCommand),
      actionSeparator(other.actionSeparator),
      childrenActions(other.childrenActions),
      actionParentPath(other.actionParentPath)
{
}

DCustomActionData &DCustomActionData::operator=(const DCustomActionData &other)
{
    if (this == &other)
        return *this;
    actionNameArg = other.actionNameArg;
    actionCmdArg = other.actionCmdArg;
    actionName = other.actionName;
    comboPos = other.comboPos;
    actionPosition = other.actionPosition;
    actionSeparator = other.actionSeparator;
    actionIcon = other.actionIcon;
    actionCommand = other.actionCommand;
    childrenActions = other.childrenActions;
    actionParentPath = other.actionParentPath;
    return *this;
}

bool DCustomActionData::isMenu() const
{
    return !childrenActions.isEmpty();
}

bool DCustomActionData::isAction() const
{
    return childrenActions.isEmpty();
}

QString DCustomActionData::name() const
{
    return actionName;
}

int DCustomActionData::position(DCustomActionDefines::ComboType combo) const
{
    auto it = comboPos.find(combo);
    if (it != comboPos.end())
        return it.value();
    else
        return actionPosition;
}

int DCustomActionData::position() const
{
    return actionPosition;
}

QString DCustomActionData::icon() const
{
    return actionIcon;
}

QString DCustomActionData::command() const
{
    return actionCommand;
}

QString DCustomActionData::parentPath() const
{
    return actionParentPath;
}

DCustomActionDefines::Separator DCustomActionData::separator() const
{
    return actionSeparator;
}

QList<DCustomActionData> DCustomActionData::acitons() const
{
    return childrenActions;
}

DCustomActionDefines::ActionArg DCustomActionData::nameArg() const
{
    return actionNameArg;
}

DCustomActionDefines::ActionArg DCustomActionData::commandArg() const
{
    return actionCmdArg;
}

DCustomActionEntry::DCustomActionEntry()
{
}

DCustomActionEntry::DCustomActionEntry(const DCustomActionEntry &other)
    : packageName(other.packageName), packageVersion(other.packageVersion), packageComment(other.packageComment), packageSign(other.packageSign), actionFileCombo(other.actionFileCombo), actionMimeTypes(other.actionMimeTypes), actionExcludeMimeTypes(other.actionExcludeMimeTypes), actionSupportSchemes(other.actionSupportSchemes), actionNotShowIn(other.actionNotShowIn), actionSupportSuffix(other.actionSupportSuffix), actionData(other.actionData)
{
}

DCustomActionEntry &DCustomActionEntry::operator=(const DCustomActionEntry &other)
{
    if (this == &other)
        return *this;
    packageName = other.packageName;
    packageVersion = other.packageVersion;
    packageComment = other.packageComment;
    actionFileCombo = other.actionFileCombo;
    actionMimeTypes = other.actionMimeTypes;
    actionExcludeMimeTypes = other.actionExcludeMimeTypes;
    actionSupportSchemes = other.actionSupportSchemes;
    actionNotShowIn = other.actionNotShowIn;
    actionSupportSuffix = other.actionSupportSuffix;
    packageSign = other.packageSign;
    actionData = other.actionData;
    return *this;
}

QString DCustomActionEntry::package() const
{
    return packageName;
}

QString DCustomActionEntry::version() const
{
    return packageVersion;
}

QString DCustomActionEntry::comment() const
{
    return packageComment;
}

DCustomActionDefines::ComboTypes DCustomActionEntry::fileCombo() const
{
    return actionFileCombo;
}

QStringList DCustomActionEntry::mimeTypes() const
{
    return actionMimeTypes;
}

QStringList DCustomActionEntry::excludeMimeTypes() const
{
    return actionExcludeMimeTypes;
}

QStringList DCustomActionEntry::surpportSchemes() const
{
    return actionSupportSchemes;
}

QStringList DCustomActionEntry::notShowIn() const
{
    return actionNotShowIn;
}

QStringList DCustomActionEntry::supportStuffix() const
{
    return actionSupportSuffix;
}

DCustomActionData DCustomActionEntry::data() const
{
    return actionData;
}

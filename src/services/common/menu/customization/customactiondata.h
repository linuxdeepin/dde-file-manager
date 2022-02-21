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
#ifndef CUSTOMACTIONDATA_H
#define CUSTOMACTIONDATA_H

#include "customactiondefine.h"

#include <QObject>

class CustomActionData
{
    friend class CustomActionParser;
    friend class CustomActionBuilder;

public:
    explicit CustomActionData();
    CustomActionData(const CustomActionData &other);
    CustomActionData &operator=(const CustomActionData &other);
    bool isMenu() const;
    bool isAction() const;
    QString name() const;
    int position(CustomActionDefines::ComboType) const;
    int position() const;
    QString icon() const;
    QString command() const;
    CustomActionDefines::Separator separator() const;
    QList<CustomActionData> acitons() const;
    CustomActionDefines::ActionArg nameArg() const;
    CustomActionDefines::ActionArg commandArg() const;

protected:
    QMap<CustomActionDefines::ComboType, int> actComboPos;   //一级菜单，不同的文件组合时的位置
    int actPosition;   //显示位置，二,三级菜单的位置，一级菜单文件组合的默认位置
    CustomActionDefines::ActionArg actNameArg;   //菜单名参数
    CustomActionDefines::ActionArg actCmdArg;   //命令参数
    QString actName;
    QString actIcon;
    QString actCommand;   //菜单执行动作
    CustomActionDefines::Separator actSeparator;
    QList<CustomActionData> actChildrenActions;   //当前action的子actions
};

//根项
class CustomActionEntry
{
    friend class CustomActionParser;
    friend class CustomActionBuilder;

public:
    explicit CustomActionEntry();
    explicit CustomActionEntry(const CustomActionEntry &other);
    CustomActionEntry &operator=(const CustomActionEntry &other);
    QString package() const;
    QString version() const;
    QString comment() const;
    CustomActionDefines::ComboTypes fileCombo() const;
    QStringList mimeTypes() const;
    QStringList excludeMimeTypes() const;
    QStringList surpportSchemes() const;
    QStringList notShowIn() const;
    QStringList supportStuffix() const;
    CustomActionData data() const;

protected:
    QString filePackage;   //配置文件名
    QString fileVersion;   //版本
    QString fileComment;   //描述
    QString fileSign;   //签名
    CustomActionDefines::ComboTypes actFileCombo;   //支持的选中项类型
    QStringList actMimeTypes;   //支持的文件类型
    QStringList actExcludeMimeTypes;   //不支持文件类型
    QStringList actSupportSchemes;   //支持协议：file、trash, tag...
    QStringList actNotShowIn;   //仅桌面或文管展示："Desktop", "Filemanager"
    QStringList actSupportSuffix;   //支持后缀: 归档管理器 *.7z.001,*.7z.002,*.7z.003...
    CustomActionData actData;   //一级菜单项的数据
};
#endif   // CUSTOMACTIONDATA_H

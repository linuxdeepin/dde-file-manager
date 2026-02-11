// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCUSTOMACTIONDATA_H
#define DCUSTOMACTIONDATA_H

#include "dfmplugin_menu_global.h"
#include "dcustomactiondefine.h"

#include <QObject>

namespace dfmplugin_menu {

class DCustomActionData
{
    friend class DCustomActionParser;
    friend class DCustomActionBuilder;

public:
    explicit DCustomActionData();
    DCustomActionData(const DCustomActionData &other);
    DCustomActionData &operator=(const DCustomActionData &other);
    bool isMenu() const;
    bool isAction() const;
    QString name() const;
    int position(DCustomActionDefines::ComboType) const;
    int position() const;
    QString icon() const;
    QString command() const;
    QString parentPath() const;
    DCustomActionDefines::Separator separator() const;
    QList<DCustomActionData> acitons() const;
    DCustomActionDefines::ActionArg nameArg() const;
    DCustomActionDefines::ActionArg commandArg() const;
signals:

public slots:
protected:
    QMap<DCustomActionDefines::ComboType, int> comboPos;   //一级菜单，不同的文件组合时的位置
    int actionPosition;   //显示位置，二,三级菜单的位置，一级菜单文件组合的默认位置
    DCustomActionDefines::ActionArg actionNameArg;   //菜单名参数
    DCustomActionDefines::ActionArg actionCmdArg;   //命令参数
    QString actionName;
    QString actionIcon;
    QString actionCommand;   //菜单执行动作
    DCustomActionDefines::Separator actionSeparator;
    QList<DCustomActionData> childrenActions;   //当前action的子actions
    QString actionParentPath;
};

//根项
class DCustomActionEntry
{
    friend class DCustomActionParser;
    friend class DCustomActionBuilder;

public:
    explicit DCustomActionEntry();
    explicit DCustomActionEntry(const DCustomActionEntry &other);
    DCustomActionEntry &operator=(const DCustomActionEntry &other);
    QString package() const;
    QString version() const;
    QString comment() const;
    DCustomActionDefines::ComboTypes fileCombo() const;
    QStringList mimeTypes() const;
    QStringList excludeMimeTypes() const;
    QStringList surpportSchemes() const;
    QStringList notShowIn() const;
    QStringList supportStuffix() const;
    DCustomActionData data() const;

protected:
    QString packageName;   //配置文件名
    QString packageVersion;   //版本
    QString packageComment;   //描述
    QString packageSign;   //签名
    DCustomActionDefines::ComboTypes actionFileCombo;   //支持的选中项类型
    QStringList actionMimeTypes;   //支持的文件类型
    QStringList actionExcludeMimeTypes;   //不支持文件类型
    QStringList actionSupportSchemes;   //支持协议：file、trash, tag...
    QStringList actionNotShowIn;   //仅桌面或文管展示："Desktop", "Filemanager"
    QStringList actionSupportSuffix;   //支持后缀: 归档管理器 *.7z.001,*.7z.002,*.7z.003...
    DCustomActionData actionData;   //一级菜单项的数据
};

}

#endif   // DCUSTOMACTIONDATA_H

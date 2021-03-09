/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#ifndef DCUSTOMACTIONDATA_H
#define DCUSTOMACTIONDATA_H


#include "dcustomactiondefine.h"

#include <QObject>

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
    DCustomActionDefines::Separator separator() const;
    QList<DCustomActionData> acitons() const;
    DCustomActionDefines::ActionArg nameArg() const;
    DCustomActionDefines::ActionArg commandArg() const;
signals:

public slots:
protected:
    QMap<DCustomActionDefines::ComboType, int> m_comboPos; //一级菜单，不同的文件组合时的位置
    int m_position;     //显示位置，二,三级菜单的位置，一级菜单文件组合的默认位置
    DCustomActionDefines::ActionArg m_nameArg;  //菜单名参数
    DCustomActionDefines::ActionArg m_cmdArg;   //命令参数
    QString m_name;
    QString m_icon;
    QString m_command;     //菜单执行动作
    DCustomActionDefines::Separator m_separator;
    QList<DCustomActionData> m_childrenActions;     //当前action的子actions
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
    QString m_package;  //配置文件名
    QString m_version;  //版本
    QString m_comment;  //描述
    QString m_sign;     //签名
    DCustomActionDefines::ComboTypes m_fileCombo;     //支持的选中项类型
    QStringList m_mimeTypes;    //支持的文件类型
    QStringList m_excludeMimeTypes; //不支持文件类型
    QStringList m_supportSchemes;   //支持协议：file、trash, tag...
    QStringList m_notShowIn;    //仅桌面或文管展示："Desktop", "Filemanager"
    QStringList m_supportSuffix; //支持后缀: 归档管理器 *.7z.001,*.7z.002,*.7z.003...
    DCustomActionData m_data;    //一级菜单项的数据
};
#endif // DCUSTOMACTIONDATA_H

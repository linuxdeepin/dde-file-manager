/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zhangyu <zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu <zhangyub@uniontech.com>
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

#include <QObject>

#include "dcustomactiondefine.h"

class DCustomActionData : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionData(QObject *parent = nullptr);

signals:

public slots:
protected:
    QString m_name;
    QString m_genericName;
    int m_position;     //显示位置
    DCustomActionDefines::Separator m_separator;
    QString m_command;     //菜单执行动作
};

class DCustomActionFile : public QObject
{
    Q_OBJECT
public:
    explicit DCustomActionFile(QObject *parent = nullptr);
protected:
    QString m_package;  //配置文件名
    QString m_version;  //版本
    QString m_comment;  //描述
    DCustomActionDefines::FileComboTypes m_fileCombo;
    QStringList m_mimeTypes;    //支持的文件类型，目前需求不判断
    QList<DCustomActionData> m_rootActions;    //一级菜单
};
#endif // DCUSTOMACTIONDATA_H

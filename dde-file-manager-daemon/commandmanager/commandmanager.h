/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>

class CommandManagerAdaptor;

class CommandManager : public QObject
{
    Q_OBJECT
public:
    static QString ObjectPath;
    explicit CommandManager(QObject *parent = 0);

signals:

public slots:
    bool process(const QString& cmd, const QStringList& args, QString &output, QString &error);
    bool startDetached(const QString &cmd, const QStringList &args);

private:
    CommandManagerAdaptor* m_commandManagerAdaptor = NULL;
};

#endif // COMMANDMANAGER_H

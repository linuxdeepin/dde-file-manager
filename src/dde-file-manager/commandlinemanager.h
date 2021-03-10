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

#ifndef COMMANDLINEMANAGER_H
#define COMMANDLINEMANAGER_H

#include <QCommandLineOption>
#include <QJsonObject>
#include <QMap>
#include <QList>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
QT_END_NAMESPACE

class DUrl;

class CommandLineManager
{
public:
    static CommandLineManager *instance();

    bool isSet(const QString &name) const;
    QString value(const QString &name) const;

    void process();
    void process(const QStringList &arguments);

    void addOption(const QCommandLineOption &option);
    void addOptions(const QList<QCommandLineOption> &options);
    QStringList positionalArguments() const;
    QStringList unknownOptionNames() const;

    void processCommand();

private:
    explicit CommandLineManager();
    CommandLineManager(CommandLineManager &) = delete;
    CommandLineManager &operator=(CommandLineManager &) = delete;
    void initOptions();

    ~CommandLineManager();
    QCommandLineParser *m_commandParser;
};

#endif // COMMANDLINEMANAGER_H

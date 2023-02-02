/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
class QCommandLineOption;
QT_END_NAMESPACE

class CommandParser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CommandParser)

public:
    static CommandParser &instance();

    void bindEvents();
    bool isSet(const QString &name) const;
    QString value(const QString &name) const;
    void processCommand();
    void process();
    void process(const QStringList &arguments);

private:
    void initialize();
    void initOptions();
    void addOption(const QCommandLineOption &option);

    QStringList positionalArguments() const;
    QStringList unknownOptionNames() const;
    void showPropertyDialog();
    void openWithDialog();
    void openInHomeDirectory();
    void openInUrls();
    void openWindowWithUrl(const QUrl &url);

private:
    explicit CommandParser(QObject *parent = nullptr);

private:
    QCommandLineParser *commandParser;
};

#endif   // COMMANDPARSER_H

// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
class QCommandLineOption;
QT_END_NAMESPACE

class CommandParserPrivate;
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
    void openSession();
    void openWindowWithUrl(const QUrl &url);
    void processEvent();

private:
    explicit CommandParser(QObject *parent = nullptr);
    ~CommandParser();

private:
    QCommandLineParser *commandParser;
    QScopedPointer<CommandParserPrivate> d;
};

#endif   // COMMANDPARSER_H

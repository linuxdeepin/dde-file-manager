// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDPARSER_P_H
#define COMMANDPARSER_P_H

#include <QVariantMap>

class CommandParserPrivate
{
public:
    struct EventArgsInfo
    {
        QString action;
        QVariantMap params;
    };

    explicit CommandParserPrivate();
    ~CommandParserPrivate();

    EventArgsInfo parseEventArgs(const QByteArray &eventArg);
};

#endif   // COMMANDPARSER_P_H

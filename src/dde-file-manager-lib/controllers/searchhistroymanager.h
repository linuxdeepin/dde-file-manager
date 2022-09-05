// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHHISTROYMANAGER_H
#define SEARCHHISTROYMANAGER_H

#include <QStringList>

class SearchHistroyManager
{
public:
    explicit SearchHistroyManager();
    ~SearchHistroyManager();

    QStringList toStringList();

    void writeIntoSearchHistory(QString keyword);
    bool removeSearchHistory(QString keyword);

    void clearHistory();
};

#endif // SEARCHHISTROYMANAGER_H

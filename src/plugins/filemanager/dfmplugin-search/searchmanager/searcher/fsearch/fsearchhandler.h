// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSEARCHHANDLER_H
#define FSEARCHHANDLER_H

#include "dfmplugin_search_global.h"

extern "C" {
#include "fsearch/fsearch.h"
}

#include <QFlags>
#include <QMutex>

#include <functional>

#define DEFAULT_MAX_RESULTS 50000

DPSEARCH_BEGIN_NAMESPACE

class FSearchHandler
{
public:
    using FSearchCallbackFunc = std::function<void(const QString &, bool)>;
    enum FSearchFlag {
        FSEARCH_FLAG_NONE = 0,
        FSEARCH_FLAG_FILTER_HIDDEN_FILE = 1,
        FSEARCH_FLAG_PINYIN = 1 << 1,
        FSEARCH_FLAG_REGEX = 1 << 2,
        FSEARCH_FLAG_ALL = FSEARCH_FLAG_FILTER_HIDDEN_FILE | FSEARCH_FLAG_REGEX | FSEARCH_FLAG_PINYIN
    };
    Q_DECLARE_FLAGS(FSearchFlags, FSearchFlag)

    explicit FSearchHandler();
    ~FSearchHandler();

    static bool checkPathSearchable(const QString &path);

    void init();
    void reset();
    bool loadDatabase(const QString &path, const QString &dbLocation);
    bool updateDatabase();
    bool saveDatabase(const QString &savePath);
    bool search(const QString &keyword, FSearchCallbackFunc callback);
    void stop();
    void setMaxResults(uint32_t max);
    void setFlags(FSearchFlags flags);
    long dbTimeStamp();

private:
    void releaseApp();
    static void reveiceResultsCallback(void *data, void *sender);

private:
    bool isStop = false;
    FsearchApplication *app = nullptr;
    uint32_t maxResults = DEFAULT_MAX_RESULTS;
    FSearchCallbackFunc callbackFunc = nullptr;
    QMutex syncMutex;
};

DPSEARCH_END_NAMESPACE

Q_DECLARE_OPERATORS_FOR_FLAGS(DPSEARCH_NAMESPACE::FSearchHandler::FSearchFlags)

#endif   // FSEARCHHANDLER_H

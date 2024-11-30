// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXDBUS_P_H
#define TEXTINDEXDBUS_P_H

#include "service_textindex_global.h"
#include "task/taskmanager.h"
#include "textindexadaptor.h"

#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>
#include <FSDirectory.h>

#include <docparser.h>

#include <exception>

#include <dirent.h>

class TextIndexDBus;

SERVICETEXTINDEX_BEGIN_NAMESPACE

class TextIndexDBusPrivate
{
    friend class ::TextIndexDBus;

public:
    explicit TextIndexDBusPrivate(TextIndexDBus *qq)
        : q(qq), adapter(new TextIndexAdaptor(qq)), taskManager(new TaskManager(qq))
    {
    }
    ~TextIndexDBusPrivate() { }

    void initConnect();
    QString indexStorePath() const;

private:
    TextIndexDBus *q { nullptr };
    TextIndexAdaptor *adapter { nullptr };
    TaskManager *taskManager { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // TEXTINDEXDBUS_P_H

// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXDBUS_P_H
#define TEXTINDEXDBUS_P_H

#include "service_textindex_global.h"
#include "core/indexruntime.h"
#include "textindexadaptor.h"
#include "utils/textindexconfig.h"

#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>
#include <FSDirectory.h>

#include <dirent.h>

class TextIndexDBus;

SERVICETEXTINDEX_BEGIN_NAMESPACE

class TextIndexDBusPrivate
{
    friend class ::TextIndexDBus;

public:
    explicit TextIndexDBusPrivate(TextIndexDBus *qq)
        : q(qq),
          adapter(new TextIndexAdaptor(qq)),
          runtime(new IndexRuntime(IndexProfile::content(), qq))
    {
        initialize();
        initConnect();
    }
    ~TextIndexDBusPrivate() { }

    void initialize();
    void initConnect();
    void handleMonitoring(bool start);
    void handleSlientStart();
    bool canSilentlyRefreshIndex(const QString &path) const;

private:
    void handleConfigChanged();
    void initializeSupportedExtensions();

private:
    TextIndexDBus *q { nullptr };
    TextIndexAdaptor *adapter { nullptr };
    IndexRuntime *runtime { nullptr };
    
    // Store current supported file extensions for comparison
    QStringList m_currentSupportedExtensions;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // TEXTINDEXDBUS_P_H

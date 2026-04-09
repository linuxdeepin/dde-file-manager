// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCRINDEXDBUS_P_H
#define OCRINDEXDBUS_P_H

#include "service_textindex_global.h"
#include "core/indexruntime.h"
#include "ocrindexadaptor.h"

class OcrIndexDBus;

SERVICETEXTINDEX_BEGIN_NAMESPACE

class OcrIndexDBusPrivate
{
    friend class ::OcrIndexDBus;

public:
    explicit OcrIndexDBusPrivate(OcrIndexDBus *qq)
        : q(qq),
          adapter(new OcrIndexAdaptor(qq)),
          runtime(new IndexRuntime(IndexProfile::ocr(), qq))
    {
        initialize();
        initConnect();
    }

    ~OcrIndexDBusPrivate() { }

    void initialize();
    void initConnect();
    void handleMonitoring(bool start);
    void handleSlientStart();
    bool canSilentlyRefreshIndex(const QString &path) const;

private:
    void handleConfigChanged();
    void initializeSupportedExtensions();

private:
    OcrIndexDBus *q { nullptr };
    OcrIndexAdaptor *adapter { nullptr };
    IndexRuntime *runtime { nullptr };
    QStringList m_currentSupportedExtensions;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // OCRINDEXDBUS_P_H

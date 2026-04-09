// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXPROFILE_H
#define INDEXPROFILE_H

#include "service_textindex_global.h"

#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexProfile
{
public:
    enum class Type {
        Content,
        Ocr
    };

    using DirectoryProvider = std::function<QString()>;
    using AvailabilityChecker = std::function<bool()>;
    using ScopeChecker = std::function<bool(const QString &)>;
    using CandidateChecker = std::function<bool(const QString &)>;
    using AnythingTypeProvider = std::function<QStringList()>;

    IndexProfile() = default;
    IndexProfile(Type type,
                 QString id,
                 QString statusFileName,
                 QString versionKey,
                 int runtimeVersion,
                 DirectoryProvider indexDirectoryProvider,
                 AvailabilityChecker availabilityChecker,
                 ScopeChecker scopeChecker,
                 CandidateChecker candidateChecker,
                 AnythingTypeProvider anythingTypeProvider = {});

    Type type() const;
    const QString &id() const;
    QString indexDirectory() const;
    QString statusFilePath() const;
    const QString &versionKey() const;
    bool isIndexAvailable() const;
    int runtimeIndexVersion() const;
    bool isPathInScope(const QString &path) const;
    bool isCandidateFile(const QString &path) const;
    QStringList anythingFileTypes() const;
    bool supportsAnything() const;

    static IndexProfile content();
    static IndexProfile ocr();

private:
    Type m_type { Type::Content };
    QString m_id;
    QString m_statusFileName { QStringLiteral("index_status.json") };
    QString m_versionKey;
    int m_runtimeVersion { -1 };
    DirectoryProvider m_indexDirectoryProvider;
    AvailabilityChecker m_availabilityChecker;
    ScopeChecker m_scopeChecker;
    CandidateChecker m_candidateChecker;
    AnythingTypeProvider m_anythingTypeProvider;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXPROFILE_H

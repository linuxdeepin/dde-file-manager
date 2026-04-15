// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexprofile.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"

#include <QFileInfo>

SERVICETEXTINDEX_BEGIN_NAMESPACE

IndexProfile::IndexProfile(Type type,
                           QString id,
                           QString statusFileName,
                           QString versionKey,
                           int runtimeVersion,
                           DirectoryProvider indexDirectoryProvider,
                           AvailabilityChecker availabilityChecker,
                           ScopeChecker scopeChecker,
                           CandidateChecker candidateChecker,
                           AnythingSearchOptionsProvider anythingSearchOptionsProvider)
    : m_type(type),
      m_id(std::move(id)),
      m_statusFileName(std::move(statusFileName)),
      m_versionKey(std::move(versionKey)),
      m_runtimeVersion(runtimeVersion),
      m_indexDirectoryProvider(std::move(indexDirectoryProvider)),
      m_availabilityChecker(std::move(availabilityChecker)),
      m_scopeChecker(std::move(scopeChecker)),
      m_candidateChecker(std::move(candidateChecker)),
      m_anythingSearchOptionsProvider(std::move(anythingSearchOptionsProvider))
{
}

IndexProfile::Type IndexProfile::type() const
{
    return m_type;
}

const QString &IndexProfile::id() const
{
    return m_id;
}

QString IndexProfile::indexDirectory() const
{
    return m_indexDirectoryProvider ? m_indexDirectoryProvider() : QString();
}

QString IndexProfile::statusFilePath() const
{
    const QString indexDir = indexDirectory();
    if (indexDir.isEmpty()) {
        return QString();
    }

    return indexDir + QLatin1Char('/') + m_statusFileName;
}

const QString &IndexProfile::versionKey() const
{
    return m_versionKey;
}

bool IndexProfile::isIndexAvailable() const
{
    return m_availabilityChecker ? m_availabilityChecker() : false;
}

int IndexProfile::runtimeIndexVersion() const
{
    return m_runtimeVersion;
}

bool IndexProfile::isPathInScope(const QString &path) const
{
    return m_scopeChecker ? m_scopeChecker(path) : false;
}

bool IndexProfile::isCandidateFile(const QString &path) const
{
    return m_candidateChecker ? m_candidateChecker(path) : false;
}

IndexProfile::AnythingSearchOptions IndexProfile::anythingSearchOptions() const
{
    return m_anythingSearchOptionsProvider ? m_anythingSearchOptionsProvider() : AnythingSearchOptions {};
}

bool IndexProfile::supportsAnything() const
{
    return !anythingSearchOptions().isEmpty();
}

IndexProfile IndexProfile::content()
{
    return IndexProfile {
        Type::Content,
        QStringLiteral("content"),
        QStringLiteral("index_status.json"),
        Defines::kTextVersionKey,
        Defines::kTextIndexVersion,
        []() { return DFMSEARCH::Global::contentIndexDirectory(); },
        []() { return DFMSEARCH::Global::isContentIndexAvailable(); },
        [](const QString &path) { return DFMSEARCH::Global::isPathInContentIndexDirectory(path); },
        [](const QString &path) { return IndexUtility::isSupportedTextFile(path); },
        []() {
            return AnythingSearchOptions {
                { Defines::kAnythingDocType },
                {}
            };
        }
    };
}

IndexProfile IndexProfile::ocr()
{
    return IndexProfile {
        Type::Ocr,
        QStringLiteral("ocr"),
        QStringLiteral("index_status.json"),
        Defines::kOcrVersionKey,
        Defines::kOcrIndexVersion,
        []() { return DFMSEARCH::Global::ocrTextIndexDirectory(); },
        []() { return DFMSEARCH::Global::isOcrTextIndexAvailable(); },
        [](const QString &path) { return DFMSEARCH::Global::isPathInOcrTextIndexDirectory(path); },
        [](const QString &path) { return IndexUtility::isSupportedOCRFile(path); },
        []() {
            return AnythingSearchOptions {
                { Defines::kAnythingPicType },
                TextIndexConfig::instance().supportedOcrImageExtensions()
            };
        }
    };
}

SERVICETEXTINDEX_END_NAMESPACE

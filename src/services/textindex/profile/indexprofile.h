// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXPROFILE_H
#define INDEXPROFILE_H

#include "service_textindex_global.h"

#include <boost/shared_ptr.hpp>

#include <functional>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexProfile
{
public:
    struct AnythingSearchOptions {
        QStringList fileTypes;
        QStringList fileExtensions;

        bool isEmpty() const
        {
            return fileTypes.isEmpty() && fileExtensions.isEmpty();
        }
    };

    enum class Type {
        Content,
        Ocr
    };

    using DirectoryProvider = std::function<QString()>;
    using AvailabilityChecker = std::function<bool()>;
    using ScopeChecker = std::function<bool(const QString &)>;
    using CandidateChecker = std::function<bool(const QString &)>;
    using AnythingSearchOptionsProvider = std::function<AnythingSearchOptions()>;
    using ChecksumProvider = std::function<QString(const QString &filePath)>;
    using TextCacheLookup = std::function<QString(const QString &checksum)>;
    using AnalyzerProvider = std::function<boost::shared_ptr<void>()>;

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
                 AnythingSearchOptionsProvider anythingSearchOptionsProvider = {},
                 ChecksumProvider checksumProvider = {},
                 TextCacheLookup textCacheLookup = {},
                 AnalyzerProvider analyzerProvider = {});

    Type type() const;
    const QString &id() const;
    QString indexDirectory() const;
    QString statusFilePath() const;
    const QString &versionKey() const;
    bool isIndexAvailable() const;
    int runtimeIndexVersion() const;
    bool isPathInScope(const QString &path) const;
    bool isCandidateFile(const QString &path) const;
    AnythingSearchOptions anythingSearchOptions() const;
    bool supportsAnything() const;

    /**
     * @brief Compute a checksum for the given file, if the profile supports it
     * @return Checksum string, or empty if the profile does not support checksumming
     */
    QString computeChecksum(const QString &filePath) const;

    /**
     * @brief Look up cached extraction text by checksum, if the profile supports it
     * @return Cached text, or empty if no cache hit or not supported
     */
    QString lookupCachedText(const QString &checksum) const;

    bool supportsChecksum() const;

    const wchar_t *pathField() const;
    const wchar_t *ancestorPathsField() const;
    const wchar_t *modifyTimeField() const;
    bool supportsModifiedTimestampCheck() const;
    boost::shared_ptr<void> createAnalyzer() const;
    int maxFileTruncationSizeMB() const;

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
    AnythingSearchOptionsProvider m_anythingSearchOptionsProvider;
    ChecksumProvider m_checksumProvider;
    TextCacheLookup m_textCacheLookup;
    AnalyzerProvider m_analyzerProvider;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXPROFILE_H

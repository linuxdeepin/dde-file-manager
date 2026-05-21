// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexprofile.h"
#include "extractor/ocrdeduplication.h"
#include "utils/filehash.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"

#include <dfm-search/field_names.h>
#include <dfm-search/lucene++/ngramanalyzer.h>
#include <lucene++/LuceneHeaders.h>

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
                           AnythingSearchOptionsProvider anythingSearchOptionsProvider,
                           ChecksumProvider checksumProvider,
                           TextCacheLookup textCacheLookup,
                           AnalyzerProvider analyzerProvider)
    : m_type(type),
      m_id(std::move(id)),
      m_statusFileName(std::move(statusFileName)),
      m_versionKey(std::move(versionKey)),
      m_runtimeVersion(runtimeVersion),
      m_indexDirectoryProvider(std::move(indexDirectoryProvider)),
      m_availabilityChecker(std::move(availabilityChecker)),
      m_scopeChecker(std::move(scopeChecker)),
      m_candidateChecker(std::move(candidateChecker)),
      m_anythingSearchOptionsProvider(std::move(anythingSearchOptionsProvider)),
      m_checksumProvider(std::move(checksumProvider)),
      m_textCacheLookup(std::move(textCacheLookup)),
      m_analyzerProvider(std::move(analyzerProvider))
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

QString IndexProfile::computeChecksum(const QString &filePath) const
{
    return m_checksumProvider ? m_checksumProvider(filePath) : QString();
}

QString IndexProfile::lookupCachedText(const QString &checksum) const
{
    return m_textCacheLookup ? m_textCacheLookup(checksum) : QString();
}

bool IndexProfile::supportsChecksum() const
{
    return static_cast<bool>(m_checksumProvider);
}

const wchar_t *IndexProfile::pathField() const
{
    switch (m_type) {
    case Type::Ocr:
        return DFMSEARCH::LuceneFieldNames::OcrText::kPath;
    case Type::Content:
    default:
        return DFMSEARCH::LuceneFieldNames::Content::kPath;
    }
}

const wchar_t *IndexProfile::ancestorPathsField() const
{
    switch (m_type) {
    case Type::Ocr:
        return DFMSEARCH::LuceneFieldNames::OcrText::kAncestorPaths;
    case Type::Content:
    default:
        return DFMSEARCH::LuceneFieldNames::Content::kAncestorPaths;
    }
}

const wchar_t *IndexProfile::modifyTimeField() const
{
    switch (m_type) {
    case Type::Ocr:
        return DFMSEARCH::LuceneFieldNames::OcrText::kModifyTime;
    case Type::Content:
    default:
        return DFMSEARCH::LuceneFieldNames::Content::kModifyTime;
    }
}

bool IndexProfile::supportsModifiedTimestampCheck() const
{
    switch (m_type) {
    case Type::Ocr:
    case Type::Content:
        return true;
    default:
        return false;
    }
}

boost::shared_ptr<void> IndexProfile::createAnalyzer() const
{
    return m_analyzerProvider ? m_analyzerProvider() : nullptr;
}

int IndexProfile::maxFileTruncationSizeMB() const
{
    const TextIndexConfig &config = TextIndexConfig::instance();
    switch (m_type) {
    case Type::Ocr:
        return config.maxOcrImageSizeMB();
    case Type::Content:
    default:
        return config.maxIndexFileTruncationSizeMB();
    }
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
        },
        {},
        {},
        []() -> boost::shared_ptr<void> {
            return Lucene::newLucene<Lucene::NGramAnalyzer>(2, 2);
        }
    };
}

IndexProfile IndexProfile::ocr()
{
    // Capture OCR index directory for use in the text cache lookup lambda.
    // This avoids repeatedly querying the global function on every lookup call.
    const QString ocrIndexDir = DFMSEARCH::Global::ocrTextIndexDirectory();

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
        },
        // ChecksumProvider: compute file MD5
        [](const QString &filePath) { return FileHash::computeMd5(filePath); },
        // TextCacheLookup: find existing OCR text by checksum
        [ocrIndexDir](const QString &checksum) {
            return OcrDeduplication::lookupByTextChecksum(checksum, ocrIndexDir);
        },
        // AnalyzerProvider: create NGramAnalyzer for OCR text
        []() -> boost::shared_ptr<void> {
            return Lucene::newLucene<Lucene::NGramAnalyzer>(2, 2);
        }
    };
}

SERVICETEXTINDEX_END_NAMESPACE

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexcontentmigrator.h"
#include "profile/indexprofile.h"

#include <dfm-search/field_names.h>

#include <LuceneException.h>
#include <FSDirectory.h>
#include <IndexReader.h>
#include <IndexSearcher.h>
#include <Term.h>
#include <TermQuery.h>
#include <TopDocs.h>

#include <QDir>
#include <QFileInfo>
#include <QDateTime>

SERVICETEXTINDEX_USE_NAMESPACE

using namespace Lucene;
DFM_SEARCH_USE_NS

bool IndexContentMigrator::prepare(const QString &indexDir, const IndexProfile &profile)
{
    if (indexDir.isEmpty()) {
        return false;
    }

    m_oldIndexDir = indexDir + QStringLiteral(".old");
    bool didRename = false;

    if (QDir(m_oldIndexDir).exists()) {
        // 上次 Create 崩溃残留的 .old，直接用作数据源（无需 rename）
        fmInfo() << "[IndexContentMigrator] Using existing .old residue as data source:" << m_oldIndexDir;
    } else {
        // 正常 Create 流程：rename 旧索引 → .old
        const QDir dir(indexDir);
        if (!dir.exists()) {
            m_oldIndexDir.clear();
            return false;
        }

        if (!QDir().rename(indexDir, m_oldIndexDir)) {
            fmWarning() << "[IndexContentMigrator] Failed to rename index directory:"
                        << indexDir << "→" << m_oldIndexDir;
            m_oldIndexDir.clear();
            return false;
        }
        didRename = true;
        fmInfo() << "[IndexContentMigrator] Renamed old index:" << indexDir << "→" << m_oldIndexDir;
    }

    // 以只读模式打开旧索引
    try {
        m_reader = IndexReader::open(
                FSDirectory::open(m_oldIndexDir.toStdWString()), true);
        m_searcher = newLucene<IndexSearcher>(m_reader);

        m_pathField = profile.pathField();
        m_contentField = profile.contentField();
        m_modifyTimeField = profile.modifyTimeField();

        fmInfo() << "[IndexContentMigrator] Migration prepared, reader opened with"
                 << m_reader->numDocs() << "documents";
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "[IndexContentMigrator] Failed to open old index reader:"
                    << QString::fromStdWString(e.getError());
        if (didRename) {
            QDir().rename(m_oldIndexDir, indexDir);
        }
        m_oldIndexDir.clear();
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[IndexContentMigrator] Failed to open old index reader:" << e.what();
        if (didRename) {
            QDir().rename(m_oldIndexDir, indexDir);
        }
        m_oldIndexDir.clear();
        return false;
    }
}

std::optional<QString> IndexContentMigrator::lookupContent(const QString &filePath) const
{
    if (!m_searcher || !m_pathField || !m_contentField || !m_modifyTimeField) {
        return std::nullopt;
    }

    try {
        // Step 1: 按路径查询旧索引
        TermQueryPtr query = newLucene<TermQuery>(
                newLucene<Term>(m_pathField, filePath.toStdWString()));
        TopDocsPtr topDocs = m_searcher->search(query, 1);

        if (topDocs->totalHits == 0) {
            return std::nullopt;
        }

        DocumentPtr doc = m_searcher->doc(topDocs->scoreDocs[0]->doc);

        // Step 2: 校验 modifyTime，确保文件未被修改
        const QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            return std::nullopt;
        }

        const QString currentMtime = QString::number(fileInfo.lastModified().toSecsSinceEpoch());
        const String storedMtime = doc->get(m_modifyTimeField);

        if (currentMtime.toStdWString() != storedMtime) {
            return std::nullopt;
        }

        // Step 3: 读取 content 字段
        // 返回值（无论空否）表示命中确认，调用方不应再走 extract 流程
        const String content = doc->get(m_contentField);
        return QString::fromStdWString(content);
    } catch (...) {
        // Best-effort：任何异常静默降级，回退到正常 extract 流程
        return std::nullopt;
    }
}

void IndexContentMigrator::cleanup()
{
    // 关闭 reader
    if (m_reader) {
        try {
            m_reader->close();
        } catch (...) {
        }
        m_reader.reset();
        m_searcher.reset();
    }

    // 删除 .old 目录
    if (!m_oldIndexDir.isEmpty()) {
        QDir dir(m_oldIndexDir);
        if (dir.exists()) {
            dir.removeRecursively();
            fmInfo() << "[IndexContentMigrator] Cleaned up old index:" << m_oldIndexDir;
        }
        m_oldIndexDir.clear();
    }
}

bool IndexContentMigrator::isActive() const
{
    return m_searcher && !m_oldIndexDir.isEmpty();
}

bool IndexContentMigrator::hasResidue(const QString &indexDir)
{
    const QString oldDir = indexDir + QStringLiteral(".old");
    return QDir(oldDir).exists();
}
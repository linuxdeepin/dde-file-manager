// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_moveprocessor_withindex.cpp
 * @brief Tests for FileMoveProcessor and DirectoryMoveProcessor with a real
 *        Lucene index. Documents are added directly, avoiding heavy
 *        dependencies like ProcessExtractor and IndexRuntime.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QString>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/moveprocessor.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexcontext.h"
#include "services/textindex/utils/taskstate.h"

#include <lucene++/LuceneHeaders.h>
#include <FSDirectory.h>
#include <IndexWriter.h>
#include <IndexReader.h>
#include <IndexSearcher.h>
#include <Document.h>
#include <Field.h>
#include <StandardAnalyzer.h>

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace Lucene;

namespace {

struct MoveProcIndexTest : public testing::Test
{
    QTemporaryDir tmp;
    QString indexDir;
    std::unique_ptr<IndexContext> ctx;

    static IndexProfile buildProfile(const QString &idxDir)
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "mpidx_test",
                            "mpidx_status.json",
                            "mpidx_version",
                            1,
                            [idxDir]() -> QString { return idxDir; },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &p) -> bool { return p.endsWith(".txt") || p.endsWith(".md"); },
                            {}, {}, {},
                            []() -> boost::shared_ptr<void> {
                                return newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
                            });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        indexDir = tmp.path() + "/index";
        QDir().mkpath(indexDir);

        auto profile = buildProfile(indexDir);
        ctx = std::make_unique<IndexContext>(
            std::move(profile), nullptr, nullptr, nullptr);

        // Populate index with test documents
        auto writer = newLucene<IndexWriter>(
                FSDirectory::open(indexDir.toStdWString()),
                boost::static_pointer_cast<Lucene::Analyzer>(ctx->profile().createAnalyzer()),
                true,
                IndexWriter::MaxFieldLengthUNLIMITED);

        DocumentPtr doc1 = newLucene<Document>();
        doc1->add(newLucene<Field>(ctx->profile().pathField(),
                                   (tmp.path() + "/hello.txt").toStdWString(),
                                   Field::STORE_YES,
                                   Field::INDEX_NOT_ANALYZED_NO_NORMS));
        doc1->add(newLucene<Field>(ctx->profile().contentField(),
                                   L"hello world content",
                                   Field::STORE_NO,
                                   Field::INDEX_ANALYZED));
        writer->addDocument(doc1);

        DocumentPtr doc2 = newLucene<Document>();
        doc2->add(newLucene<Field>(ctx->profile().pathField(),
                                   (tmp.path() + "/src/code.txt").toStdWString(),
                                   Field::STORE_YES,
                                   Field::INDEX_NOT_ANALYZED_NO_NORMS));
        doc2->add(newLucene<Field>(ctx->profile().contentField(),
                                   L"code content here",
                                   Field::STORE_NO,
                                   Field::INDEX_ANALYZED));
        writer->addDocument(doc2);

        writer->commit();
        writer->close();
    }

    void TearDown() override
    {
        ctx.reset();
    }

    IndexReaderPtr openReader()
    {
        return IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
    }

    IndexWriterPtr openWriter()
    {
        return newLucene<IndexWriter>(
                FSDirectory::open(indexDir.toStdWString()),
                boost::static_pointer_cast<Lucene::Analyzer>(ctx->profile().createAnalyzer()),
                false,
                IndexWriter::MaxFieldLengthUNLIMITED);
    }
};

TEST_F(MoveProcIndexTest, FileMoveProcessor_IsFileInIndex_ExistingFile)
{
    auto reader = openReader();
    auto searcher = newLucene<IndexSearcher>(reader);
    FileMoveProcessor proc(*ctx, searcher, nullptr);
    EXPECT_TRUE(proc.isFileInIndex(tmp.path() + "/hello.txt"));
}

TEST_F(MoveProcIndexTest, FileMoveProcessor_IsFileInIndex_NonExistentFile)
{
    auto reader = openReader();
    auto searcher = newLucene<IndexSearcher>(reader);
    FileMoveProcessor proc(*ctx, searcher, nullptr);
    EXPECT_FALSE(proc.isFileInIndex("/nonexistent/file.txt"));
}

TEST_F(MoveProcIndexTest, FileMoveProcessor_IsFileInIndex_InProcessedCache)
{
    auto reader = openReader();
    auto searcher = newLucene<IndexSearcher>(reader);
    FileMoveProcessor proc(*ctx, searcher, nullptr);
    proc.m_processedPaths.insert("/cached/path.txt");
    EXPECT_TRUE(proc.isFileInIndex("/cached/path.txt"));
}

TEST_F(MoveProcIndexTest, FileMoveProcessor_ProcessFileMove_Rename)
{
    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer);
    bool result = proc.processFileMove(tmp.path() + "/hello.txt", tmp.path() + "/renamed.txt");
    EXPECT_TRUE(result);
    EXPECT_TRUE(proc.hasChanges());

    writer->close();
    reader->close();
}

TEST_F(MoveProcIndexTest, FileMoveProcessor_ProcessFileMove_SourceNotInIndex)
{
    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer);
    bool result = proc.processFileMove("/nonexistent/old.txt", "/nonexistent/new.txt");
    EXPECT_TRUE(result);

    writer->close();
    reader->close();
}

TEST_F(MoveProcIndexTest, FileMoveProcessor_ProcessFileMove_TargetNotCandidate)
{
    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer);
    bool result = proc.processFileMove("/old.dat", tmp.path() + "/new.dat");
    EXPECT_TRUE(result);

    writer->close();
    reader->close();
}

TEST_F(MoveProcIndexTest, FileMoveProcessor_ProcessContentUpdate_NonExistent)
{
    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer);
    bool result = proc.processContentUpdate("/nonexistent/file.txt");
    EXPECT_FALSE(result);

    writer->close();
    reader->close();
}

TEST_F(MoveProcIndexTest, FileMoveProcessor_ProcessContentUpdateWithCache_Failure)
{
    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer);
    bool result = proc.processContentUpdateWithCache("/nonexistent/file.txt", "test failure");
    EXPECT_FALSE(result);
    EXPECT_FALSE(proc.hasChanges());

    writer->close();
    reader->close();
}

TEST_F(MoveProcIndexTest, DirectoryMoveProcessor_ProcessDirectoryMove_EmptyDir)
{
    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    DirectoryMoveProcessor proc(*ctx, searcher, writer, reader);
    TaskState state;
    bool result = proc.processDirectoryMove("/empty/dir", "/new/dir", state);
    EXPECT_TRUE(result);

    writer->close();
    reader->close();
}

TEST_F(MoveProcIndexTest, DirectoryMoveProcessor_UpdateSingleDocumentPath_SamePath)
{
    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    DirectoryMoveProcessor proc(*ctx, searcher, writer, reader);
    TermQueryPtr query = newLucene<TermQuery>(
        newLucene<Term>(ctx->profile().pathField(),
                       (tmp.path() + "/hello.txt").toStdWString()));
    TopDocsPtr topDocs = searcher->search(query, 1);
    ASSERT_GT(topDocs->totalHits, 0);
    DocumentPtr doc = searcher->doc(topDocs->scoreDocs[0]->doc);
    bool result = proc.updateSingleDocumentPath(doc, tmp.path() + "/", tmp.path() + "/");
    EXPECT_TRUE(result);

    writer->close();
    reader->close();
}

}  // namespace

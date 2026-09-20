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
#include <QFileInfo>
#include <QString>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/moveprocessor.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/core/indexcontext.h"
#include "services/textindex/document/indexdocumentbuilder.h"
#include "services/textindex/utils/indexutility.h"
#include "services/textindex/utils/taskstate.h"

#include <dfm-search/field_names.h>

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
using namespace DFMSEARCH::LuceneFieldNames;

namespace {

// 模拟 filename 语义的文档构建器：业务字段（fake_name）由路径派生，
// 用于验证 IndexProfile::MoveUpdatePolicy::RebuildDocument 下 rename 后派生字段随新路径重建
struct FakePathDerivedBuilder : public IndexDocumentBuilder
{
    static const wchar_t *kFakeName;

    Lucene::DocumentPtr build(const QString &filePath, const QString &text,
                              const BuilderOptions &options = {}) const override
    {
        Q_UNUSED(text)
        Q_UNUSED(options)
        Lucene::DocumentPtr doc = newLucene<Lucene::Document>();
        doc->add(newLucene<Lucene::Field>(kFakeName,
                                          QFileInfo(filePath).fileName().toLower().toStdWString(),
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_ANALYZED));
        doc->add(newLucene<Lucene::Field>(DFMSEARCH::LuceneFieldNames::Content::kPath,
                                          filePath.toStdWString(),
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));
        return doc;
    }
};

const wchar_t *FakePathDerivedBuilder::kFakeName = L"fake_name";

// 模拟 filename 语义（RebuildDocument 策略 + is_hidden 完整路径派生）：
// 用于验证目录改名走 copy + is_hidden 重算（不整体重建），且移入隐藏目录树时更新
struct FakeFilenameLikeBuilder : public FakePathDerivedBuilder
{
    Lucene::DocumentPtr build(const QString &filePath, const QString &text,
                              const BuilderOptions &options = {}) const override
    {
        Q_UNUSED(text)
        Q_UNUSED(options)
        Lucene::DocumentPtr doc = newLucene<Lucene::Document>();
        doc->add(newLucene<Lucene::Field>(DFMSEARCH::LuceneFieldNames::Content::kPath,
                                          filePath.toStdWString(),
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Lucene::Field>(kFakeName,
                                          QFileInfo(filePath).fileName().toLower().toStdWString(),
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Lucene::Field>(kFakeHidden,
                                          DFMSEARCH::Global::isHiddenPathOrInHiddenDir(filePath)
                                                  ? L"Y" : L"N",
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));
        for (const QString &ancestor : PathCalculator::extractAncestorPaths(filePath)) {
            doc->add(newLucene<Lucene::Field>(DFMSEARCH::LuceneFieldNames::Content::kAncestorPaths,
                                              ancestor.toStdWString(),
                                              Lucene::Field::STORE_NO, Lucene::Field::INDEX_NOT_ANALYZED));
        }
        return doc;
    }

    std::list<PathDerivedFieldSpec> pathDerivedFields() const override
    {
        return {
            { kFakeHidden, [](const QString &p) -> Lucene::FieldPtr {
                 return newLucene<Lucene::Field>(kFakeHidden,
                                                 DFMSEARCH::Global::isHiddenPathOrInHiddenDir(p) ? L"Y" : L"N",
                                                 Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED);
             } },
        };
    }

    static const wchar_t *kFakeHidden;
};

const wchar_t *FakeFilenameLikeBuilder::kFakeHidden = L"fake_hidden";

// 模拟 content/ocr 语义：文档含路径无关字段（content）+ 部分路径派生字段
// （fake_name/fake_ext），用于验证 UpdatePathOnly 下派生字段按新路径重算
struct FakePartialDerivedBuilder : public IndexDocumentBuilder
{
    static const wchar_t *kFakeName;
    static const wchar_t *kFakeExt;

    Lucene::DocumentPtr build(const QString &filePath, const QString &text,
                              const BuilderOptions &options = {}) const override
    {
        Q_UNUSED(text)
        Q_UNUSED(options)
        Lucene::DocumentPtr doc = newLucene<Lucene::Document>();
        doc->add(newLucene<Lucene::Field>(DFMSEARCH::LuceneFieldNames::Content::kPath,
                                          filePath.toStdWString(),
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));
        doc->add(newLucene<Lucene::Field>(DFMSEARCH::LuceneFieldNames::Content::kContents,
                                          L"keep me",
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_ANALYZED));
        doc->add(newLucene<Lucene::Field>(kFakeName,
                                          QFileInfo(filePath).fileName().toLower().toStdWString(),
                                          Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));
        const QString ext = QFileInfo(filePath).suffix().toLower();
        if (!ext.isEmpty()) {
            doc->add(newLucene<Lucene::Field>(kFakeExt, ext.toStdWString(),
                                              Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));
        }
        for (const QString &ancestor : PathCalculator::extractAncestorPaths(filePath)) {
            doc->add(newLucene<Lucene::Field>(DFMSEARCH::LuceneFieldNames::Content::kAncestorPaths,
                                              ancestor.toStdWString(),
                                              Lucene::Field::STORE_NO, Lucene::Field::INDEX_NOT_ANALYZED));
        }
        return doc;
    }

    std::list<PathDerivedFieldSpec> pathDerivedFields() const override
    {
        return {
            { kFakeName, [](const QString &p) -> Lucene::FieldPtr {
                 return newLucene<Lucene::Field>(kFakeName, QFileInfo(p).fileName().toLower().toStdWString(),
                                                 Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED);
             } },
            { kFakeExt, [](const QString &p) -> Lucene::FieldPtr {
                 const QString ext = QFileInfo(p).suffix().toLower();
                 if (ext.isEmpty())
                     return nullptr;   // 与 build() 一致：无扩展名则不加该字段
                 return newLucene<Lucene::Field>(kFakeExt, ext.toStdWString(),
                                                 Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED);
             } },
        };
    }
};

const wchar_t *FakePartialDerivedBuilder::kFakeName = L"fake_name";
const wchar_t *FakePartialDerivedBuilder::kFakeExt = L"fake_ext";

struct MoveProcIndexTest : public testing::Test
{
    QTemporaryDir tmp;
    QString indexDir;
    std::unique_ptr<IndexContext> ctx;

    static IndexProfile buildProfile(const QString &idxDir,
                                     IndexProfile::MoveUpdatePolicy policy = IndexProfile::MoveUpdatePolicy::UpdatePathOnly,
                                     bool requiresContentExtraction = true)
    {
        return IndexProfile({ IndexProfile::Type::Content,
                              "mpidx_test",
                              "mpidx_status.json",
                              "mpidx_version",
                              1 },
                            { [idxDir]() -> QString { return idxDir; },
                              []() -> bool { return true; },
                              [](const QString &) -> bool { return true; },
                              [](const QString &p) -> bool { return p.endsWith(".txt") || p.endsWith(".md"); },
                              {}, {}, {},
                              []() -> boost::shared_ptr<void> {
                                  return newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
                              } },
                            IndexProfile::RuntimePolicy{}, IndexProfile::FilterPolicy{},
                            { policy, requiresContentExtraction });
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
        // 与 ContentDocumentBuilder 一致：contents 为 STORE_YES + INDEX_ANALYZED
        doc1->add(newLucene<Field>(ctx->profile().contentField(),
                                   L"hello world content",
                                   Field::STORE_YES,
                                   Field::INDEX_ANALYZED));
        writer->addDocument(doc1);

        DocumentPtr doc2 = newLucene<Document>();
        doc2->add(newLucene<Field>(ctx->profile().pathField(),
                                   (tmp.path() + "/src/code.txt").toStdWString(),
                                   Field::STORE_YES,
                                   Field::INDEX_NOT_ANALYZED_NO_NORMS));
        doc2->add(newLucene<Field>(ctx->profile().contentField(),
                                   L"code content here",
                                   Field::STORE_YES,
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

// RebuildDocument 策略（模拟 filename 语义）：rename 后文档按新路径整体重建，
// 路径派生字段（fake_name）必须为新值，且旧路径文档无残留
TEST_F(MoveProcIndexTest, FileMoveProcessor_RebuildDocumentPolicy_RebuildsFromNewPath)
{
    FakePathDerivedBuilder builder;
    auto profile = buildProfile(indexDir, IndexProfile::MoveUpdatePolicy::RebuildDocument, false);
    ctx = std::make_unique<IndexContext>(std::move(profile), nullptr, nullptr, &builder);

    const QString from = tmp.path() + "/hello.txt";
    const QString to = tmp.path() + "/renamed.txt";

    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer);
    EXPECT_TRUE(proc.processFileMove(from, to));
    EXPECT_TRUE(proc.hasChanges());

    writer->commit();
    writer->close();
    reader->close();

    // 重新打开索引验证持久化结果
    auto reader2 = IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
    auto searcher2 = newLucene<IndexSearcher>(reader2);

    // 旧路径无残留
    TopDocsPtr oldHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), from.toStdWString())), 1);
    EXPECT_EQ(oldHits->totalHits, 0);

    // 新路径命中，且路径派生字段来自新路径
    TopDocsPtr newHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), to.toStdWString())), 1);
    ASSERT_EQ(newHits->totalHits, 1);
    DocumentPtr doc = searcher2->doc(newHits->scoreDocs[0]->doc);
    EXPECT_EQ(doc->get(FakePathDerivedBuilder::kFakeName), L"renamed.txt");

    reader2->close();
}

// UpdatePathOnly 策略（Content/Ocr 语义）：rename 仅更新 path，
// 其余字段原样保留（不触发文档重建）
TEST_F(MoveProcIndexTest, FileMoveProcessor_UpdatePathOnlyPolicy_KeepsExistingFields)
{
    const QString from = tmp.path() + "/hello.txt";
    const QString to = tmp.path() + "/renamed.txt";

    auto reader = openReader();
    auto writer = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer);
    EXPECT_TRUE(proc.processFileMove(from, to));

    writer->commit();
    writer->close();
    reader->close();

    auto reader2 = IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
    auto searcher2 = newLucene<IndexSearcher>(reader2);

    TopDocsPtr newHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), to.toStdWString())), 1);
    ASSERT_EQ(newHits->totalHits, 1);
    DocumentPtr doc = searcher2->doc(newHits->scoreDocs[0]->doc);
    // content 字段保留旧值（未重建；与 ContentDocumentBuilder 一致的 STORE_YES 字段）
    EXPECT_EQ(doc->get(ctx->profile().contentField()), L"hello world content");

    reader2->close();
}

// UpdatePathOnly + builder 声明的部分路径派生字段（content/ocr 的 kFilename/
// kIsHidden/kFileExt 语义）：move 后派生字段按新路径重算，路径无关字段保留
TEST_F(MoveProcIndexTest, FileMoveProcessor_UpdatePathOnlyPolicy_RecomputesDerivedFields)
{
    FakePartialDerivedBuilder builder;
    auto profile = buildProfile(indexDir);
    ctx = std::make_unique<IndexContext>(std::move(profile), nullptr, nullptr, &builder);

    const QString from = tmp.path() + "/hello.txt";
    const QString to = tmp.path() + "/renamed.md";

    // 预置初始文档（模拟已索引的旧路径）
    auto writer = newLucene<IndexWriter>(
            FSDirectory::open(indexDir.toStdWString()),
            newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
            true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->addDocument(builder.build(from, QString()));
    writer->commit();
    writer->close();

    auto reader = openReader();
    auto writer2 = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer2);
    EXPECT_TRUE(proc.processFileMove(from, to));
    EXPECT_TRUE(proc.hasChanges());

    writer2->commit();
    writer2->close();
    reader->close();

    auto reader2 = IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
    auto searcher2 = newLucene<IndexSearcher>(reader2);

    // 旧路径无残留
    TopDocsPtr oldHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), from.toStdWString())), 1);
    EXPECT_EQ(oldHits->totalHits, 0);

    // 新路径命中：路径无关字段保留，派生字段按新路径重算
    TopDocsPtr newHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), to.toStdWString())), 1);
    ASSERT_EQ(newHits->totalHits, 1);
    DocumentPtr doc = searcher2->doc(newHits->scoreDocs[0]->doc);
    EXPECT_EQ(doc->get(DFMSEARCH::LuceneFieldNames::Content::kContents), L"keep me");
    EXPECT_EQ(doc->get(FakePartialDerivedBuilder::kFakeName), L"renamed.md");
    EXPECT_EQ(doc->get(FakePartialDerivedBuilder::kFakeExt), L"md");

    reader2->close();
}

// 重命名去掉扩展名：派生字段重算返回空 FieldPtr → 字段从文档中消失
TEST_F(MoveProcIndexTest, FileMoveProcessor_UpdatePathOnlyPolicy_DropsFieldWhenDerivedEmpty)
{
    FakePartialDerivedBuilder builder;
    auto profile = buildProfile(indexDir);
    ctx = std::make_unique<IndexContext>(std::move(profile), nullptr, nullptr, &builder);

    const QString from = tmp.path() + "/hello.txt";
    const QString to = tmp.path() + "/noext";

    auto writer = newLucene<IndexWriter>(
            FSDirectory::open(indexDir.toStdWString()),
            newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
            true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->addDocument(builder.build(from, QString()));
    writer->commit();
    writer->close();

    auto reader = openReader();
    auto writer2 = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    FileMoveProcessor proc(*ctx, searcher, writer2);
    EXPECT_TRUE(proc.processFileMove(from, to));

    writer2->commit();
    writer2->close();
    reader->close();

    auto reader2 = IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
    auto searcher2 = newLucene<IndexSearcher>(reader2);

    TopDocsPtr newHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), to.toStdWString())), 1);
    ASSERT_EQ(newHits->totalHits, 1);
    DocumentPtr doc = searcher2->doc(newHits->scoreDocs[0]->doc);
    EXPECT_EQ(doc->get(FakePartialDerivedBuilder::kFakeName), L"noext");
    // 扩展名消失 → 派生字段被移除而非保留旧值
    EXPECT_TRUE(doc->get(FakePartialDerivedBuilder::kFakeExt).empty());

    reader2->close();
}

// 目录 move（UpdatePathOnly + 派生字段声明）：子文档 path 与派生字段均按新路径重算
TEST_F(MoveProcIndexTest, DirectoryMoveProcessor_UpdatePathOnlyPolicy_RecomputesDerivedFields)
{
    FakePartialDerivedBuilder builder;
    auto profile = buildProfile(indexDir);
    ctx = std::make_unique<IndexContext>(std::move(profile), nullptr, nullptr, &builder);

    const QString childFrom = tmp.path() + "/a/old/x.txt";

    auto writer = newLucene<IndexWriter>(
            FSDirectory::open(indexDir.toStdWString()),
            newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
            true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->addDocument(builder.build(childFrom, QString()));
    writer->commit();
    writer->close();

    auto reader = openReader();
    auto writer2 = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    DirectoryMoveProcessor proc(*ctx, searcher, writer2, reader);
    TaskState state;
    state.start();
    const QString fromDir = tmp.path() + "/a/old";
    const QString toDir = tmp.path() + "/b/new";
    EXPECT_TRUE(proc.processDirectoryMove(fromDir, toDir, state));
    EXPECT_TRUE(proc.hasChanges());

    writer2->commit();
    writer2->close();
    reader->close();

    auto reader2 = IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
    auto searcher2 = newLucene<IndexSearcher>(reader2);

    const QString childTo = tmp.path() + "/b/new/x.txt";
    TopDocsPtr newHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), childTo.toStdWString())), 1);
    ASSERT_EQ(newHits->totalHits, 1);
    DocumentPtr doc = searcher2->doc(newHits->scoreDocs[0]->doc);
    EXPECT_EQ(doc->get(DFMSEARCH::LuceneFieldNames::Content::kContents), L"keep me");
    EXPECT_EQ(doc->get(FakePartialDerivedBuilder::kFakeName), L"x.txt");
    EXPECT_EQ(doc->get(FakePartialDerivedBuilder::kFakeExt), L"txt");

    reader2->close();
}

// 目录 move（filename 语义 RebuildDocument 策略）：不整体重建，copy 保留
// basename 派生字段（fake_name），仅重算完整路径派生字段（is_hidden）——
// 移入隐藏目录树后子文档 is_hidden 必须更新为 Y，且无逐文档重建开销
TEST_F(MoveProcIndexTest, DirectoryMoveProcessor_RebuildDocumentPolicy_CopyWithHiddenRecompute)
{
    FakeFilenameLikeBuilder builder;
    auto profile = buildProfile(indexDir, IndexProfile::MoveUpdatePolicy::RebuildDocument, false);
    ctx = std::make_unique<IndexContext>(std::move(profile), nullptr, nullptr, &builder);

    const QString childFrom = tmp.path() + "/a/old/x.txt";

    auto writer = newLucene<IndexWriter>(
            FSDirectory::open(indexDir.toStdWString()),
            newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
            true, IndexWriter::MaxFieldLengthUNLIMITED);
    writer->addDocument(builder.build(childFrom, QString()));
    writer->commit();
    writer->close();

    auto reader = openReader();
    auto writer2 = openWriter();
    auto searcher = newLucene<IndexSearcher>(reader);

    DirectoryMoveProcessor proc(*ctx, searcher, writer2, reader);
    TaskState state;
    state.start();
    // 移入以 "." 开头的隐藏目录树：is_hidden 应由 N 重算为 Y
    const QString fromDir = tmp.path() + "/a/old";
    const QString toDir = tmp.path() + "/a/.hidden";
    EXPECT_TRUE(proc.processDirectoryMove(fromDir, toDir, state));
    EXPECT_TRUE(proc.hasChanges());

    writer2->commit();
    writer2->close();
    reader->close();

    auto reader2 = IndexReader::open(FSDirectory::open(indexDir.toStdWString()), true);
    auto searcher2 = newLucene<IndexSearcher>(reader2);

    const QString childTo = tmp.path() + "/a/.hidden/x.txt";
    TopDocsPtr newHits = searcher2->search(
            newLucene<TermQuery>(newLucene<Term>(ctx->profile().pathField(), childTo.toStdWString())), 1);
    ASSERT_EQ(newHits->totalHits, 1);
    DocumentPtr doc = searcher2->doc(newHits->scoreDocs[0]->doc);
    // basename 派生字段 copy 保留（未重建）
    EXPECT_EQ(doc->get(FakeFilenameLikeBuilder::kFakeName), L"x.txt");
    // 完整路径派生字段重算：is_hidden N → Y
    EXPECT_EQ(doc->get(FakeFilenameLikeBuilder::kFakeHidden), L"Y");

    reader2->close();
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

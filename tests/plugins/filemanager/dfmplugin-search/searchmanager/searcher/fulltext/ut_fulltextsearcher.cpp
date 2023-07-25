// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager/searcher/fulltext/fulltextsearcher.h"
#include "searchmanager/searcher/fulltext/fulltextsearcher_p.h"
#include "utils/searchhelper.h"

#include "stubext.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>
#include <docparser.h>
#include <DirectoryReader.h>

#include <QDir>

#include <dirent.h>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
using namespace Lucene;

class FullTextSearcherPrivateTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    }
    virtual void TearDown() override
    {
    }
};

// class FullTextSearcher
TEST(FullTextSearcherTest, ut_createIndex_1)
{
    stub_ext::StubExt st;
    st.set_lamda(IndexReader::indexExists, [] { __DBG_STUB_INVOKE__ return true; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    EXPECT_TRUE(searcher.createIndex("/home"));
}

TEST(FullTextSearcherTest, ut_createIndex_2)
{
    stub_ext::StubExt st;
    st.set_lamda(IndexReader::indexExists, [] { __DBG_STUB_INVOKE__ return false; });
    st.set_lamda(&FullTextSearcherPrivate::createIndex, [] { __DBG_STUB_INVOKE__ return true; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    EXPECT_TRUE(searcher.createIndex("/home"));
}

TEST(FullTextSearcherTest, ut_isSupport)
{
    UrlRoute::regScheme("recnet", "/", {}, true);
    UrlRoute::regScheme("file", "/");

    stub_ext::StubExt st;
    st.set_lamda(Application::genericAttribute, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_FALSE(FullTextSearcher::isSupport(QUrl("recent:///")));
    EXPECT_TRUE(FullTextSearcher::isSupport(QUrl::fromLocalFile("/home")));
}

TEST(FullTextSearcherTest, ut_search_1)
{
    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->isIndexCreating = true;

    EXPECT_FALSE(searcher.search());
    searcher.d->isIndexCreating = false;
}

TEST(FullTextSearcherTest, ut_search_2)
{
    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->status.storeRelease(AbstractSearcher::kRuning);

    EXPECT_FALSE(searcher.search());
}

TEST(FullTextSearcherTest, ut_search_3)
{
    stub_ext::StubExt st;
    st.set_lamda(&FullTextSearcherPrivate::dealKeyword, [] { __DBG_STUB_INVOKE__ return ""; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    EXPECT_FALSE(searcher.search());
    EXPECT_EQ(searcher.d->status.loadAcquire(), AbstractSearcher::kCompleted);
}

TEST(FullTextSearcherTest, ut_search_4)
{
    stub_ext::StubExt st;
    st.set_lamda(&FullTextSearcherPrivate::updateIndex, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&FullTextSearcherPrivate::doSearch, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(VADDR(FullTextSearcher, hasItem), [] { __DBG_STUB_INVOKE__ return true; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    EXPECT_TRUE(searcher.search());
    EXPECT_EQ(searcher.d->status.loadAcquire(), AbstractSearcher::kCompleted);
}

TEST(FullTextSearcherTest, ut_stop)
{
    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.stop();

    EXPECT_EQ(searcher.d->status.loadAcquire(), AbstractSearcher::kTerminated);
}

TEST(FullTextSearcherTest, ut_hasItem)
{
    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->allResults << QUrl::fromLocalFile("/home");

    EXPECT_TRUE(searcher.hasItem());
}

TEST(FullTextSearcherTest, ut_takeAll)
{
    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->allResults << QUrl::fromLocalFile("/home");

    auto results = searcher.takeAll();
    EXPECT_FALSE(results.isEmpty());
    EXPECT_TRUE(searcher.d->allResults.isEmpty());
}

// class FullTextSearcherPrivate
TEST_F(FullTextSearcherPrivateTest, ut_newIndexWriter)
{
    stub_ext::StubExt st;
    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });
    st.set_lamda(VADDR(IndexWriter, initialize), [] { __DBG_STUB_INVOKE__ });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    auto writer = searcher.d->newIndexWriter();

    EXPECT_TRUE(writer);
}

//TEST_F(FullTextSearcherPrivateTest, ut_newIndexReader)
//{
//    stub_ext::StubExt st;
//    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });

//    typedef IndexReaderPtr (*Open)(IndexReader *, const DirectoryPtr &, bool);
//    auto open = (Open)((IndexReaderPtr(*)(const DirectoryPtr &, bool))IndexReader::open);
//    st.set_lamda(open, [] {
//        __DBG_STUB_INVOKE__
//        return IndexReaderPtr();
//    });

//    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
//    auto reader = searcher.d->newIndexReader();

//    EXPECT_TRUE(reader);
//}

TEST_F(FullTextSearcherPrivateTest, ut_doIndexTask_1)
{
    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");

    searcher.d->doIndexTask(nullptr, nullptr, "/home", FullTextSearcherPrivate::kCreate);
    EXPECT_NE(searcher.d->status.loadAcquire(), AbstractSearcher::kRuning);
}

TEST_F(FullTextSearcherPrivateTest, ut_doIndexTask_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&DeviceUtils::fstabBindInfo, [] {
        __DBG_STUB_INVOKE__
        QMap<QString, QString> info { { "/data/home", "/home" } };
        return info;
    });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->status.storeRelease(AbstractSearcher::kRuning);

    searcher.d->doIndexTask(nullptr, nullptr, "/data/home", FullTextSearcherPrivate::kCreate);
    EXPECT_TRUE(searcher.d->bindPathTable.contains("/data/home"));
}

TEST_F(FullTextSearcherPrivateTest, ut_doIndexTask_3)
{
    stub_ext::StubExt st;
    st.set_lamda(&DeviceUtils::fstabBindInfo, [] {
        __DBG_STUB_INVOKE__
        QMap<QString, QString> info { { "/data/home", "/home" } };
        return info;
    });

    // stub readdir
    struct dirent dent;
    strcpy(dent.d_name, "test.txt");
    bool isFirst = true;
    st.set_lamda(readdir, [&] {
        __DBG_STUB_INVOKE__
        if (isFirst) {
            isFirst = false;
            return &dent;
        }
        return (dirent *)nullptr;
    });

    // stub lstat
    struct stat sta;
    sta.st_mode = __S_IFREG;
    typedef int (*LstatFunc)(const char *, struct stat *);
    auto lstatFunc = static_cast<LstatFunc>(lstat);
    st.set_lamda(lstatFunc, [&](const char *, struct stat *buf) {
        __DBG_STUB_INVOKE__
        buf = &sta;
        return 0;
    });

    st.set_lamda(&FullTextSearcherPrivate::indexDocs, [] { __DBG_STUB_INVOKE__ });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->status.storeRelease(AbstractSearcher::kRuning);

    searcher.d->doIndexTask(nullptr, nullptr, QDir::currentPath(), FullTextSearcherPrivate::kCreate);
    EXPECT_STREQ(dent.d_name, "test.txt");
}

TEST_F(FullTextSearcherPrivateTest, ut_doIndexTask_4)
{
    stub_ext::StubExt st;
    st.set_lamda(&DeviceUtils::fstabBindInfo, [] {
        __DBG_STUB_INVOKE__
        QMap<QString, QString> info { { "/data/home", "/home" } };
        return info;
    });

    // stub readdir
    struct dirent dent;
    strcpy(dent.d_name, "test.txt");
    bool isFirst = true;
    st.set_lamda(readdir, [&] {
        __DBG_STUB_INVOKE__
        if (isFirst) {
            isFirst = false;
            return &dent;
        }
        return (dirent *)nullptr;
    });

    // stub lstat
    struct stat sta;
    sta.st_mode = __S_IFREG;
    typedef int (*LstatFunc)(const char *, struct stat *);
    auto lstatFunc = static_cast<LstatFunc>(lstat);
    st.set_lamda(lstatFunc, [&](const char *, struct stat *buf) {
        __DBG_STUB_INVOKE__
        buf = &sta;
        return 0;
    });

    st.set_lamda(&FullTextSearcherPrivate::checkUpdate, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&FullTextSearcherPrivate::indexDocs, [] { __DBG_STUB_INVOKE__ });
    st.set_lamda(VADDR(SyncFileInfo, nameOf), [] { __DBG_STUB_INVOKE__ return "txt"; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->status.storeRelease(AbstractSearcher::kRuning);

    searcher.d->doIndexTask(nullptr, nullptr, QDir::currentPath(), FullTextSearcherPrivate::kUpdate);
    EXPECT_TRUE(searcher.d->isUpdated);
}

TEST_F(FullTextSearcherPrivateTest, ut_indexDocs_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });
    st.set_lamda(VADDR(IndexWriter, initialize), [] { __DBG_STUB_INVOKE__ });
    st.set_lamda(&FullTextSearcherPrivate::fileDocument, [] {
        __DBG_STUB_INVOKE__
        DocumentPtr doc = newLucene<Document>();
        return doc;
    });

    typedef void (*AddDoc)(IndexWriter *, const DocumentPtr &);
    auto add = (AddDoc)((void (IndexWriter::*)(const DocumentPtr &)) & IndexWriter::addDocument);

    bool success = false;
    st.set_lamda(add, [&] { __DBG_STUB_INVOKE__ success = true; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    auto writer = searcher.d->newIndexWriter();

    searcher.d->indexDocs(writer, "/home/test.txt", FullTextSearcherPrivate::kAddIndex);
    EXPECT_TRUE(success);
}

TEST_F(FullTextSearcherPrivateTest, ut_indexDocs_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });
    st.set_lamda(VADDR(IndexWriter, initialize), [] { __DBG_STUB_INVOKE__ });
    st.set_lamda(&FullTextSearcherPrivate::fileDocument, [] {
        __DBG_STUB_INVOKE__
        DocumentPtr doc = newLucene<Document>();
        return doc;
    });

    typedef void (*UpdateDoc)(IndexWriter *, const TermPtr &, const DocumentPtr &);
    auto update = (UpdateDoc)((void (IndexWriter::*)(const TermPtr &, const DocumentPtr &)) & IndexWriter::updateDocument);

    bool success = false;
    st.set_lamda(update, [&] { __DBG_STUB_INVOKE__ success = true; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    auto writer = searcher.d->newIndexWriter();

    searcher.d->indexDocs(writer, "/home/test.txt", FullTextSearcherPrivate::kUpdateIndex);
    EXPECT_TRUE(success);
}

TEST_F(FullTextSearcherPrivateTest, ut_indexDocs_3)
{
    stub_ext::StubExt st;
    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });
    st.set_lamda(VADDR(IndexWriter, initialize), [] { __DBG_STUB_INVOKE__ });

    typedef void (*DeleteDoc)(IndexWriter *, const TermPtr &);
    auto deleteDoc = (DeleteDoc)((void (IndexWriter::*)(const TermPtr &)) & IndexWriter::deleteDocuments);

    bool success = false;
    st.set_lamda(deleteDoc, [&] { __DBG_STUB_INVOKE__ success = true; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    auto writer = searcher.d->newIndexWriter();

    searcher.d->indexDocs(writer, "/home/test.txt", FullTextSearcherPrivate::kDeleteIndex);
    EXPECT_TRUE(success);
}

//TEST_F(FullTextSearcherPrivateTest, ut_checkUpdate_1)
//{
//    stub_ext::StubExt st;
//    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });

//    typedef TopDocsPtr (*Search)(Searcher *, const QueryPtr &, int32_t);
//    auto searchFunc = (Search)((TopDocsPtr(Searcher::*)(const QueryPtr &, int32_t)) & Searcher::search);
//    st.set_lamda(searchFunc, [] {
//        __DBG_STUB_INVOKE__
//        Collection<ScoreDocPtr> scoreDocs;
//        TopDocsPtr tdp = newLucene<TopDocs>(0, scoreDocs);
//        return tdp;
//    });

//    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
//    auto reader = searcher.d->newIndexReader();

//    FullTextSearcherPrivate::IndexType type = FullTextSearcherPrivate::kUpdateIndex;
//    bool result = searcher.d->checkUpdate(reader, "/home/test.txt", type);

//    EXPECT_TRUE(result);
//    EXPECT_EQ(type, FullTextSearcherPrivate::kAddIndex);
//}

//TEST_F(FullTextSearcherPrivateTest, ut_checkUpdate_2)
//{
//    stub_ext::StubExt st;
//    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });

//    typedef TopDocsPtr (*Search)(Searcher *, const QueryPtr &, int32_t);
//    auto searchFunc = (Search)((TopDocsPtr(Searcher::*)(const QueryPtr &, int32_t)) & Searcher::search);
//    st.set_lamda(searchFunc, [] {
//        __DBG_STUB_INVOKE__
//        Collection<ScoreDocPtr> scoreDocs = Collection<ScoreDocPtr>::newInstance(0);
//        ScoreDocPtr sd = newLucene<ScoreDoc>(1, 1);
//        scoreDocs.add(sd);
//        TopDocsPtr tdp = newLucene<TopDocs>(1, scoreDocs);
//        return tdp;
//    });

//    auto curTime = QDateTime::currentDateTime();
//    typedef DocumentPtr (*Doc)(IndexSearcher *, int32_t);
//    auto docFunc = (Doc)((DocumentPtr(IndexSearcher::*)(int32_t)) & IndexSearcher::doc);
//    st.set_lamda(docFunc, [&] {
//        __DBG_STUB_INVOKE__
//        DocumentPtr doc = newLucene<Document>();
//        auto strTime = curTime.toString("yyyyMMddHHmmss").toStdWString();
//        doc->add(newLucene<Field>(L"modified", strTime, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
//        return doc;
//    });

//    st.set_lamda(&QFileInfo::lastModified, [&] { __DBG_STUB_INVOKE__ return curTime.addDays(1); });

//    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
//    auto reader = searcher.d->newIndexReader();

//    FullTextSearcherPrivate::IndexType type = FullTextSearcherPrivate::kAddIndex;
//    bool result = searcher.d->checkUpdate(reader, "/home/test.txt", type);

//    EXPECT_TRUE(result);
//    EXPECT_EQ(type, FullTextSearcherPrivate::kUpdateIndex);
//}

TEST_F(FullTextSearcherPrivateTest, ut_tryNotify)
{
    stub_ext::StubExt st;
    st.set_lamda(&QTime::elapsed, [] { __DBG_STUB_INVOKE__ return 100; });
    st.set_lamda(VADDR(FullTextSearcher, hasItem), [] { __DBG_STUB_INVOKE__ return true; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    searcher.d->tryNotify();

    EXPECT_EQ(searcher.d->lastEmit, 100);
}

TEST_F(FullTextSearcherPrivateTest, ut_fileDocument)
{
    stub_ext::StubExt st;
    st.set_lamda(DocParser::convertFile, [] { __DBG_STUB_INVOKE__ return std::string("test"); });
    st.set_lamda(&QFileInfo::lastModified, [] { __DBG_STUB_INVOKE__ return QDateTime::currentDateTime(); });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    auto doc = searcher.d->fileDocument("/home/test.txt");

    auto value = doc->get(L"contents");
    EXPECT_EQ(value, L"test");
}

TEST_F(FullTextSearcherPrivateTest, ut_createIndex_1)
{
    stub_ext::StubExt st;
    typedef bool (QDir::*Exists)(const QString &) const;
    auto exists = static_cast<Exists>(&QDir::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return false; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    EXPECT_FALSE(searcher.d->createIndex("/tst"));
    EXPECT_EQ(searcher.d->status.loadAcquire(), AbstractSearcher::kCompleted);
}

TEST_F(FullTextSearcherPrivateTest, ut_createIndex_2)
{
    stub_ext::StubExt st;
    typedef bool (QDir::*Exists)(const QString &) const;
    auto exists = static_cast<Exists>(&QDir::exists);
    st.set_lamda(exists, [](QDir *&, const QString &path) { __DBG_STUB_INVOKE__ return path == "/tst"; });
    st.set_lamda(&QDir::mkpath, [] { __DBG_STUB_INVOKE__ return false; });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    EXPECT_FALSE(searcher.d->createIndex("/tst"));
    EXPECT_EQ(searcher.d->status.loadAcquire(), AbstractSearcher::kCompleted);
}

TEST_F(FullTextSearcherPrivateTest, ut_createIndex_3)
{
    stub_ext::StubExt st;
    typedef bool (QDir::*Exists)(const QString &) const;
    auto exists = static_cast<Exists>(&QDir::exists);
    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&FullTextSearcherPrivate::doIndexTask, [] { __DBG_STUB_INVOKE__ });
    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });
    st.set_lamda(VADDR(IndexWriter, initialize), [] { __DBG_STUB_INVOKE__ });

    st.set_lamda(VADDR(IndexWriter, deleteAll), [] { __DBG_STUB_INVOKE__ });

    typedef void (*Optimize)(IndexWriter *);
    auto optimize = (Optimize)((void (IndexWriter::*)()) & IndexWriter::optimize);
    st.set_lamda(optimize, [] { __DBG_STUB_INVOKE__ });

    typedef void (*Close)(IndexWriter *);
    auto close = (Close)((void (IndexWriter::*)()) & IndexWriter::close);
    st.set_lamda(close, [] { __DBG_STUB_INVOKE__ });

    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
    EXPECT_TRUE(searcher.d->createIndex("/tst"));
    EXPECT_EQ(searcher.d->status.loadAcquire(), AbstractSearcher::kCompleted);
}

//TEST_F(FullTextSearcherPrivateTest, ut_updateIndex)
//{
//    stub_ext::StubExt st;
//    st.set_lamda(&FullTextSearcherPrivate::doIndexTask, [] { __DBG_STUB_INVOKE__ });
//    st.set_lamda(FileUtils::bindPathTransform, [] { __DBG_STUB_INVOKE__ return "/tst"; });
//    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });
//    st.set_lamda(VADDR(IndexWriter, initialize), [] { __DBG_STUB_INVOKE__ });

//    typedef void (*Optimize)(IndexWriter *);
//    auto optimize = (Optimize)((void (IndexWriter::*)()) & IndexWriter::optimize);
//    st.set_lamda(optimize, [] { __DBG_STUB_INVOKE__ });

//    typedef void (*WriterClose)(IndexWriter *);
//    auto writerClose = (WriterClose)((void (IndexWriter::*)()) & IndexWriter::close);
//    st.set_lamda(writerClose, [] { __DBG_STUB_INVOKE__ });

//    typedef void (*ReaderClose)(IndexReader *);
//    auto readerClose = (ReaderClose)((void (IndexReader::*)()) & IndexReader::close);
//    st.set_lamda(readerClose, [] { __DBG_STUB_INVOKE__ });

//    FullTextSearcher searcher(QUrl::fromLocalFile("/home"), "test");
//    EXPECT_TRUE(searcher.d->updateIndex("/tst"));
//}

//TEST_F(FullTextSearcherPrivateTest, ut_doSearch)
//{
//    stub_ext::StubExt st;
//    st.set_lamda(FileUtils::bindPathTransform, [] { __DBG_STUB_INVOKE__ return "/tst"; });
//    st.set_lamda(&FullTextSearcherPrivate::indexStorePath, [] { __DBG_STUB_INVOKE__ return "/index"; });

//    st.set_lamda(VADDR(IndexWriter, initialize), [] { __DBG_STUB_INVOKE__ });

//    typedef TopDocsPtr (*Search)(Searcher *, const QueryPtr &, const FilterPtr &, int32_t);
//    auto searchFunc = (Search)((TopDocsPtr(Searcher::*)(const QueryPtr &, const FilterPtr &, int32_t)) & Searcher::search);
//    st.set_lamda(searchFunc, [] {
//        __DBG_STUB_INVOKE__
//        Collection<ScoreDocPtr> scoreDocs = Collection<ScoreDocPtr>::newInstance(0);
//        ScoreDocPtr sd = newLucene<ScoreDoc>(1, 1);
//        scoreDocs.add(sd);
//        TopDocsPtr tdp = newLucene<TopDocs>(1, scoreDocs);
//        return tdp;
//    });

//    auto curTime = QDateTime::currentDateTime();
//    typedef DocumentPtr (*Doc)(IndexSearcher *, int32_t);
//    auto docFunc = (Doc)((DocumentPtr(IndexSearcher::*)(int32_t)) & IndexSearcher::doc);
//    st.set_lamda(docFunc, [&] {
//        __DBG_STUB_INVOKE__
//        DocumentPtr doc = newLucene<Document>();
//        auto strTime = curTime.toString("yyyyMMddHHmmss").toStdWString();
//        doc->add(newLucene<Field>(L"modified", strTime, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
//        doc->add(newLucene<Field>(L"path", L"/tst", Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
//        return doc;
//    });

//    typedef bool (QFileInfo::*Exists)() const;
//    auto exists = static_cast<Exists>(&QFileInfo::exists);
//    st.set_lamda(exists, [] { __DBG_STUB_INVOKE__ return true; });
//    st.set_lamda(&QFileInfo::lastModified, [&] { __DBG_STUB_INVOKE__ return curTime; });
//    st.set_lamda(&SearchHelper::isHiddenFile, [] { __DBG_STUB_INVOKE__ return false; });
//}

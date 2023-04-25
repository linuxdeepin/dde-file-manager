// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/fileprovider.h"
#include "model/filefilter.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QList>
#include <QCoreApplication>

DDP_CANVAS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(FileProvider, setRoot)
{
    FileProvider fp;
    stub_ext::StubExt stub;
    bool startWatcher = false;
    stub.set_lamda(VADDR(AbstractFileWatcher,startWatcher), [&startWatcher](){
        startWatcher = true;
        return true;
    });

    ASSERT_TRUE(fp.setRoot(QUrl::fromLocalFile("/home")));
    EXPECT_EQ(fp.root(), QUrl("file:///home"));
    ASSERT_TRUE(startWatcher);

   //check signal
    bool re = false;
    stub.set_lamda(&FileProvider::remove, [&re](){
        re = true;
    });
    bool ins = false;
    stub.set_lamda(&FileProvider::insert, [&ins](){
        ins = true;
    });
    bool rn = false;
    stub.set_lamda(&FileProvider::rename, [&rn](){
        rn = true;
    });
    bool up = false;
    stub.set_lamda(&FileProvider::update, [&up](){
        up = true;
    });

    emit fp.watcher->fileDeleted(QUrl());
    EXPECT_FALSE(re);
    qApp->processEvents();
    EXPECT_TRUE(re);

    emit fp.watcher->subfileCreated(QUrl());
    EXPECT_FALSE(ins);
    qApp->processEvents();
    EXPECT_TRUE(ins);

    emit fp.watcher->fileRename(QUrl(),QUrl());
    EXPECT_FALSE(rn);
    qApp->processEvents();
    EXPECT_TRUE(rn);

    emit fp.watcher->fileAttributeChanged(QUrl());
    EXPECT_FALSE(up);
    qApp->processEvents();
    EXPECT_TRUE(up);
}

TEST(FileProvider, refresh)
{
    FileProvider fp;
    stub_ext::StubExt stub;
    bool start = false;
    stub.set_lamda(&TraversalDirThread::start, [&start](){
        start = true;
    });
    fp.refresh();
    EXPECT_TRUE(fp.isUpdating());
    EXPECT_TRUE(start);

    //check signal
    bool re = false;
    stub.set_lamda(&FileProvider::reset, [&re](){
        re = true;
    });
    bool fin = false;
    stub.set_lamda(&FileProvider::traversalFinished, [&fin](){
        fin = true;
    });
    bool up = false;
    stub.set_lamda(&FileProvider::preupdateData, [&up](){
        up = true;
    });

    emit fp.traversalThread->updateChildren({});
    EXPECT_TRUE(re);

    emit fp.traversalThread->updateChild(QUrl());
    EXPECT_TRUE(up);

    emit fp.traversalThread->finished({});
    EXPECT_TRUE(fin);
}

class TestReset : public FileFilter
{
public:
    bool fileTraversalFilter(QList<QUrl> &urls) override {
        urls.clear();
        ct = true;
        return false;
    }
    bool fileDeletedFilter(const QUrl &url) override {
        cd = true;
        return rd;
    }
    bool fileCreatedFilter(const QUrl &url) override {
        cc = true;
        return rc;
    }
    bool fileRenameFilter(const QUrl &oldUrl, const QUrl &newUrl) override {
        cr = true;
        return rr;
    }
    bool fileUpdatedFilter(const QUrl &url) override {
        cu = true;
        return ru;
    }
    bool ct = false;
    bool cd = false;
    bool cc = false;
    bool cr = false;
    bool cu = false;
    bool rd = false;
    bool rc = false;
    bool rr = false;
    bool ru = false;
};

TEST(FileProvider, reset)
{
    FileProvider fp;
    QList<QUrl> src = {QUrl(), QUrl()};
    QSharedPointer<FileFilter> filter(new TestReset);
    fp.installFileFilter(filter);
    ASSERT_EQ(fp.fileFilters.size(), 1);
    ASSERT_EQ(fp.fileFilters.first(), filter);

    QObject::connect(&fp, &FileProvider::refreshEnd, &fp, [&src](const QList<QUrl> &in){
        src = in;
    });

    fp.reset(src);

    EXPECT_TRUE(src.isEmpty());
    EXPECT_TRUE(dynamic_cast<TestReset *>(filter.get())->ct);

    fp.removeFileFilter(filter);

    EXPECT_TRUE(fp.fileFilters.isEmpty());
}

TEST(FileProvider, traversalFinished)
{
    FileProvider fp;
    fp.updateing = true;
    fp.traversalFinished();
    EXPECT_FALSE(fp.updateing);
}

class TestModelFileFilter: public testing::Test
{
public:
    void SetUp() override {
        QSharedPointer<FileFilter> filter1(new TestReset);
        f1 = dynamic_cast<TestReset *>(filter1.get());
        fp.fileFilters.append(filter1);
        QSharedPointer<FileFilter> filter2(new TestReset);
        f2 = dynamic_cast<TestReset *>(filter2.get());
        fp.fileFilters.append(filter2);
        QSharedPointer<FileFilter> filter3(new TestReset);
        f3 = dynamic_cast<TestReset *>(filter3.get());
        fp.fileFilters.append(filter3);
    }
    FileProvider fp;
    TestReset *f1;
    TestReset *f2;
    TestReset *f3;
};

TEST_F(TestModelFileFilter, insert)
{
    QUrl src;
    QObject::connect(&fp, &FileProvider::fileInserted, &fp, [&src](const QUrl &in){
        src = in;
    });

    QUrl in = QUrl::fromLocalFile("/home");

    {
        fp.insert(in);
        EXPECT_EQ(src, in);
        EXPECT_TRUE(f1->cc);
        EXPECT_TRUE(f2->cc);
        EXPECT_TRUE(f3->cc);
    }

    {
        src = QUrl();
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f1->rc = true;
        fp.insert(in);
        EXPECT_TRUE(src.isEmpty());
        EXPECT_TRUE(f1->cc);
        EXPECT_FALSE(f2->cc);
        EXPECT_FALSE(f3->cc);
    }

    {
        src = QUrl();
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f1->rc = false;
        f2->rc = true;
        fp.insert(in);
        EXPECT_TRUE(src.isEmpty());
        EXPECT_TRUE(f1->cc);
        EXPECT_TRUE(f2->cc);
        EXPECT_FALSE(f3->cc);
    }

    {
        src = QUrl();
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f2->rc = false;
        f3->rc = true;
        fp.insert(in);
        EXPECT_TRUE(src.isEmpty());
        EXPECT_TRUE(f1->cc);
        EXPECT_TRUE(f2->cc);
        EXPECT_TRUE(f3->cc);
    }
}

TEST_F(TestModelFileFilter, rename)
{
    QUrl oldUrl;
    QUrl newUrl;
    QObject::connect(&fp, &FileProvider::fileRenamed, &fp, [&oldUrl, &newUrl](const QUrl &o, const QUrl &n){
        oldUrl = o;
        newUrl = n;
    });

    QUrl in = QUrl::fromLocalFile("/home");
    QUrl in2 = QUrl::fromLocalFile("/home2");

    {
        fp.rename(in, in2);
        EXPECT_EQ(oldUrl, in);
        EXPECT_EQ(newUrl, in2);
        EXPECT_TRUE(f1->cr);
        EXPECT_TRUE(f2->cr);
        EXPECT_TRUE(f3->cr);
    }

    {
        oldUrl = QUrl();
        newUrl = QUrl();
        f1->cr = false;
        f2->cr = false;
        f3->cr = false;
        f1->rr = true;
        fp.rename(in, in2);
        EXPECT_EQ(oldUrl, in);
        EXPECT_TRUE(newUrl.isEmpty());
        EXPECT_TRUE(f1->cr);
        EXPECT_FALSE(f2->cr);
        EXPECT_FALSE(f3->cr);
    }

    {
        oldUrl = QUrl();
        newUrl = QUrl();
        f1->cr = false;
        f2->cr = false;
        f3->cr = false;
        f1->rr = false;
        f2->rr = true;
        fp.rename(in, in2);
        EXPECT_EQ(oldUrl, in);
        EXPECT_TRUE(newUrl.isEmpty());
        EXPECT_TRUE(f1->cr);
        EXPECT_TRUE(f2->cr);
        EXPECT_FALSE(f3->cr);
    }

    {
        oldUrl = QUrl();
        newUrl = QUrl();
        f1->cc = false;
        f2->cc = false;
        f3->cc = false;
        f2->rr = false;
        f3->rr = true;
        fp.rename(in, in2);
        EXPECT_EQ(oldUrl, in);
        EXPECT_TRUE(newUrl.isEmpty());
        EXPECT_TRUE(f1->cr);
        EXPECT_TRUE(f2->cr);
        EXPECT_TRUE(f3->cr);
    }
}

TEST_F(TestModelFileFilter, remove)
{
    QUrl src;
    QObject::connect(&fp, &FileProvider::fileRemoved, &fp, [&src](const QUrl &in){
        src = in;
    });

    QUrl in = QUrl::fromLocalFile("/home");

    {
        fp.remove(in);
        EXPECT_EQ(src, in);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }

    {
        src = QUrl();
        f1->cd = false;
        f2->cd = false;
        f3->cd = false;
        f1->rd = true;
        fp.remove(in);
        EXPECT_EQ(src, in);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }

    {
        src = QUrl();
        f1->cd = false;
        f2->cd = false;
        f3->cd = false;
        f1->rd = false;
        f2->rd = true;
        fp.remove(in);
        EXPECT_EQ(src, in);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }

    {
        src = QUrl();
        f1->cd = false;
        f2->cd = false;
        f3->cd = false;
        f2->rd = false;
        f3->rd = true;
        fp.remove(in);
        EXPECT_EQ(src, in);
        EXPECT_TRUE(f1->cd);
        EXPECT_TRUE(f2->cd);
        EXPECT_TRUE(f3->cd);
    }
}

TEST_F(TestModelFileFilter, update)
{
    QUrl src;
    QObject::connect(&fp, &FileProvider::fileUpdated, &fp, [&src](const QUrl &in){
        src = in;
    });

    fp.rootUrl = QUrl::fromLocalFile("/home");
    QUrl in = QUrl::fromLocalFile("/home/test/sss");

    {
        fp.update(in);
        EXPECT_TRUE(src.isEmpty());
        EXPECT_FALSE(f1->cu);
        EXPECT_FALSE(f2->cu);
        EXPECT_FALSE(f3->cu);
    }

    in = QUrl::fromLocalFile("/home/test");

    {
        src = QUrl();
        f1->cu = false;
        f2->cu = false;
        f3->cu = false;
        fp.update(in);
        EXPECT_EQ(src, in);
        EXPECT_TRUE(f1->cu);
        EXPECT_TRUE(f2->cu);
        EXPECT_TRUE(f3->cu);
    }

    {
        src = QUrl();
        f1->cu = false;
        f2->cu = false;
        f3->cu = false;
        f1->ru = true;
        fp.update(in);
        EXPECT_TRUE(src.isEmpty());
        EXPECT_TRUE(f1->cu);
        EXPECT_FALSE(f2->cu);
        EXPECT_FALSE(f3->cu);
    }

    {
        src = QUrl();
        f1->cu = false;
        f2->cu = false;
        f3->cu = false;
        f1->ru = false;
        f2->ru = true;
        fp.update(in);
        EXPECT_TRUE(src.isEmpty());
        EXPECT_TRUE(f1->cu);
        EXPECT_TRUE(f2->cu);
        EXPECT_FALSE(f3->cu);
    }

    {
        src = QUrl();
        f1->cu = false;
        f2->cu = false;
        f3->cu = false;
        f2->ru = false;
        f3->ru = true;
        fp.update(in);
        EXPECT_TRUE(src.isEmpty());
        EXPECT_TRUE(f1->cu);
        EXPECT_TRUE(f2->cu);
        EXPECT_TRUE(f3->cu);
    }
}

TEST(RedundantUpdateFilter, fileUpdatedFilter)
{
    FileProvider fp;
    RedundantUpdateFilter filter(&fp);
    QUrl in = QUrl::fromLocalFile("/home/test/sss");

    EXPECT_FALSE(filter.fileUpdatedFilter(in));
    EXPECT_EQ(filter.updateList.value(in, -1), 0);
    EXPECT_EQ(filter.timerid, -1);

    EXPECT_TRUE(filter.fileUpdatedFilter(in));
    EXPECT_EQ(filter.updateList.value(in, -1), 1);
    EXPECT_NE(filter.timerid, -1);

    int id = filter.timerid;

    EXPECT_TRUE(filter.fileUpdatedFilter(in));
    EXPECT_EQ(filter.updateList.value(in, -1), 2);
    EXPECT_EQ(filter.timerid, id);

    QUrl in2 = QUrl::fromLocalFile("/home/test/sss2");
    EXPECT_FALSE(filter.fileUpdatedFilter(in2));
    EXPECT_EQ(filter.updateList.value(in, -1), 2);
    EXPECT_EQ(filter.updateList.value(in2, -1), 0);
}

TEST(RedundantUpdateFilter, checkUpdate)
{
    FileProvider fp;
    RedundantUpdateFilter filter(&fp);
    QUrl in = QUrl::fromLocalFile("/home/test/sss");
    QUrl in2 = QUrl::fromLocalFile("/home/test/sss2");
    stub_ext::StubExt stub;

    int kt = -1;
    stub.set_lamda(&RedundantUpdateFilter::killTimer, [&kt](QObject *, int id) {
        kt = id;
    });

    const int fid = 2;
    filter.timerid = fid;
    filter.updateList.insert(in, 0);
    filter.updateList.insert(in2, 0);

    QList<QUrl> updated;
    QObject::connect(&fp, &FileProvider::fileUpdated, &fp, [&updated](const QUrl &in){
        updated.append(in);
    });

    {
        filter.checkUpdate();

        EXPECT_TRUE(filter.updateList.isEmpty());
        EXPECT_EQ(filter.timerid, -1);
        EXPECT_EQ(kt, fid);
        EXPECT_TRUE(updated.isEmpty());
    }

    {
        kt = -1;
        filter.updateList.clear();
        filter.updateList.insert(in, 1);
        filter.updateList.insert(in2, 0);
        filter.timerid = fid;
        updated.clear();

        filter.checkUpdate();
        EXPECT_TRUE(filter.updateList.isEmpty());
        EXPECT_EQ(filter.timerid, -1);
        EXPECT_EQ(updated.size(), 1);
        EXPECT_EQ(kt, fid);
        EXPECT_TRUE(updated.contains(in));
    }


    {
        kt = -1;
        filter.updateList.clear();
        filter.updateList.insert(in, 1);
        filter.updateList.insert(in2, 2);
        filter.timerid = fid;
        updated.clear();

        filter.checkUpdate();
        EXPECT_TRUE(filter.updateList.isEmpty());
        EXPECT_EQ(filter.timerid, -1);
        EXPECT_EQ(updated.size(), 2);
        EXPECT_EQ(kt, fid);
        EXPECT_TRUE(updated.contains(in));
        EXPECT_TRUE(updated.contains(in2));
    }
}

TEST(RedundantUpdateFilter, timerEvent)
{
    RedundantUpdateFilter filter(nullptr);
    stub_ext::StubExt stub;
    bool check = false;
    stub.set_lamda(&RedundantUpdateFilter::checkUpdate, [&check]() {
        check = true;
    });

    filter.timerid = -1;
    QTimerEvent e(2);
    filter.timerEvent(&e);
    EXPECT_FALSE(check);

    filter.timerid = 2;
    check = false;
    filter.timerEvent(&e);
    EXPECT_TRUE(check);
}

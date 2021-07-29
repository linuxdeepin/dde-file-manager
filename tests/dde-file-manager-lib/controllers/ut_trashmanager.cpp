/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <gtest/gtest.h>

#include "controllers/trashmanager.h"
#include "dfmstandardpaths.h"
#include "dfmevent.h"
#include "dfmeventdispatcher.h"
#include "dialogs/dialogmanager.h"
#include "dfileservices.h"
#include "models/trashfileinfo.h"
#include "stub.h"
#include "testhelper.h"
#include "dfileproxywatcher.h"

#include <QProcess>

namespace  {
class TestTrashManager: public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestTrashManager" << std::endl;
        m_trash = new TrashManager();
        trashFileUrl = DUrl::fromTrashFile("/test.txt");
        QProcess::execute("touch " + trashFileUrl.toLocalFile());
    }

    virtual void TearDown() override
    {
        std::cout << "end TestTrashManager" << std::endl;
        QProcess::execute("rm -f " + trashFileUrl.toLocalFile());
        if (m_trash) {
            delete m_trash;
            m_trash = nullptr;
        }
    }

    void DumpDirector(DDirIteratorPointer director)
    {
        auto fileName = director->fileName();
        auto fileUrl = director->fileUrl();
        auto fileInfo = director->fileInfo();
        auto durl = director->url();

        while (director->hasNext()) {
            director->next();
        }
    }

public:
    TrashManager *m_trash;
    DUrl trashFileUrl;
};
}

#ifndef __arm__ // arm 下崩溃
TEST_F(TestTrashManager, createFileInfo)
{
    TestHelper::runInLoop([](){}); // arm 下崩溃
    auto pfile = m_trash->createFileInfo(dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, trashFileUrl));
    EXPECT_TRUE(pfile != nullptr);
}
#endif

TEST_F(TestTrashManager, openFile)
{
    // is file
    {
        bool (*stub_isFile)() = []() {
            return true;
        };

        int (*stub_showMessageDialog)(DialogManager::messageType, const QString &, const QString &, QString)
        = [](DialogManager::messageType, const QString &, const QString &, QString) {
            return 1;
        };

        Stub stub;
        stub.set(ADDR(QFileInfo, isFile), stub_isFile);
        stub.set(ADDR(DialogManager, showMessageDialog), stub_showMessageDialog);

        auto res = m_trash->openFile(dMakeEventPointer<DFMOpenFileEvent>(nullptr, DUrl::fromTrashFile("/test")));
        EXPECT_FALSE(res);
    }

    // is dir
    {
        bool (*stub_isFile)() = []() {
            return false;
        };

        bool (*stub_openFile)(const QObject, const DUrl &) = [](const QObject, const DUrl &) {
            return true;
        };

        Stub stub;
        stub.set(ADDR(QFileInfo, isFile), stub_isFile);
        stub.set(ADDR(DFileService, openFile), stub_openFile);

        auto res = m_trash->openFile(dMakeEventPointer<DFMOpenFileEvent>(nullptr, DUrl::fromTrashFile("/test")));
        EXPECT_TRUE(res);
    }
}

TEST_F(TestTrashManager, openFiles)
{
    int (*stub_showMessageDialog)(DialogManager::messageType, const QString &, const QString &, QString)
    = [](DialogManager::messageType, const QString &, const QString &, QString) {
        return 1;
    };

    Stub stub;
    stub.set(ADDR(DialogManager, showMessageDialog), stub_showMessageDialog);

    auto res = m_trash->openFiles(dMakeEventPointer<DFMOpenFilesEvent>(nullptr, DUrlList() << DUrl::fromTrashFile("/test")));
    EXPECT_FALSE(res);
}

TEST_F(TestTrashManager, moveToTrash)
{
    bool (*stub_deleteFiles)(const QObject *, const DUrlList &, bool, bool, bool)
    = [](const QObject *, const DUrlList &, bool, bool, bool) {
        return true;
    };

    Stub stub;
    stub.set(ADDR(DFileService, deleteFiles), stub_deleteFiles);

    auto res = m_trash->moveToTrash(dMakeEventPointer<DFMMoveToTrashEvent>(nullptr, DUrlList()));
    EXPECT_TRUE(res.isEmpty());
}

TEST_F(TestTrashManager, writeFilesToClipboard)
{
    // not cutaction or copyaction
    {
        auto res = m_trash->writeFilesToClipboard(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::UnknowAction, DUrlList() << trashFileUrl));
        EXPECT_FALSE(res);
    }

    // cut
    {
        bool (*stub_writeFilesToClipboard)(const QObject *, DFMGlobal::ClipboardAction, const DUrlList &)
        = [](const QObject *, DFMGlobal::ClipboardAction, const DUrlList &) {
            return true;
        };

        Stub stub;
        stub.set(ADDR(DFileService, writeFilesToClipboard), stub_writeFilesToClipboard);

        auto res = m_trash->writeFilesToClipboard(dMakeEventPointer<DFMWriteUrlsToClipboardEvent>(nullptr, DFMGlobal::CutAction, DUrlList() << trashFileUrl));
        EXPECT_TRUE(res);
    }
}

TEST_F(TestTrashManager, pasteFile)
{
    // action is not cut action or target url is not trash
    {
        auto res = m_trash->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::UnknowAction, DUrl::fromTrashFile("/test"), DUrlList() << DUrl("file:///tmp/1.txt")));
        EXPECT_TRUE(res.isEmpty());
    }

    // url list is empty
    {
        auto res = m_trash->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromTrashFile("/"), DUrlList()));
        EXPECT_TRUE(res.isEmpty());
    }

    // normal
    {
        DUrlList(*stub_moveToTrash)(const QObject *, const DUrlList &)
        = [](const QObject *, const DUrlList &) {
            return DUrlList() << DUrl::fromTrashFile("/1.txt");
        };

        Stub stub;
        stub.set(ADDR(DFileService, moveToTrash), stub_moveToTrash);

        auto res = m_trash->pasteFile(dMakeEventPointer<DFMPasteEvent>(nullptr, DFMGlobal::CutAction, DUrl::fromTrashFile("/"), DUrlList() << DUrl("file:///tmp/1.txt")));
        EXPECT_TRUE(!res.isEmpty());
    }
}

TEST_F(TestTrashManager, restoreFile)
{
    QList<DAbstractFileInfoPointer>(*stub_getChildren)(const QObject *, const DUrl &, const QStringList &, QDir::Filters, QDirIterator::IteratorFlags, bool, bool)
    = [](const QObject *, const DUrl &, const QStringList &, QDir::Filters, QDirIterator::IteratorFlags, bool, bool) {
        DAbstractFileInfoPointer info(new DAbstractFileInfo(DUrl::fromTrashFile("/1.txt")));
        return QList<DAbstractFileInfoPointer>() << info;
    };

    bool (*stub_restore)(FileJob *) = [](FileJob *) {
        return true;
    };

    QVariant(*stub_processEvent)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) = [](const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *) {
        return QVariant(true);
    };

    Stub stub;
    stub.set(ADDR(DFileService, getChildren), stub_getChildren);
    stub.set(ADDR(TrashFileInfo, restore), stub_restore);
    stub.set((QVariant(DFMEventDispatcher::*)(const QSharedPointer<DFMEvent> &, DFMAbstractEventHandler *))ADDR(DFMEventDispatcher, processEvent), stub_processEvent);

    auto res = m_trash->restoreFile(dMakeEventPointer<DFMRestoreFromTrashEvent>(nullptr, DUrlList() << DUrl::fromTrashFile("/")));
    EXPECT_TRUE(res);
}

TEST_F(TestTrashManager, deleteFiles)
{
    bool (*stub_deleteFiles)(const QObject *, const DUrlList &, bool, bool, bool)
    = [](const QObject *, const DUrlList &, bool, bool, bool) {
        return true;
    };

    Stub stub;
    stub.set(ADDR(DFileService, deleteFiles), stub_deleteFiles);

    // is trash root
    {
        auto res = m_trash->deleteFiles(dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << DUrl::fromTrashFile("/"), true, true));
        EXPECT_TRUE(res);
    }
    // other
    {
        auto res = m_trash->deleteFiles(dMakeEventPointer<DFMDeleteEvent>(nullptr, DUrlList() << trashFileUrl, true, true));
        EXPECT_TRUE(res);
    }
}

TEST_F(TestTrashManager, createDirIterator)
{
    QDirIterator::IteratorFlags flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
    auto res = m_trash->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, DUrl::fromTrashFile("/"), QStringList(), QDir::AllEntries, flags, false, false));
    EXPECT_TRUE(res != nullptr);
    DumpDirector(res);
}

TEST_F(TestTrashManager, createFileWatcher)
{
    QPointer<DFileProxyWatcher> res = static_cast<DFileProxyWatcher *>(m_trash->createFileWatcher(dMakeEventPointer<DFMCreateFileWatcherEvent>(nullptr, DUrl::fromTrashFile("/"))));
    EXPECT_TRUE(res != nullptr);
    if (res)
        delete res;
}

TEST_F(TestTrashManager, isEmpty)
{
    auto res = m_trash->isEmpty();
    EXPECT_FALSE(res);
}

TEST_F(TestTrashManager, isWorking)
{
    auto res = m_trash->isWorking();
    EXPECT_FALSE(res);
}

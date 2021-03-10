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

#include "private/dabstractfilewatcher_p.h"
#define protected public
#include "dabstractfilewatcher.h"

namespace  {
class FileWatcher : public DAbstractFileWatcher
{
public:
    explicit FileWatcher(const QString &filePath, QObject *parent = nullptr);
};

class FileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    explicit FileWatcherPrivate(FileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}


    bool start() override
    {
        return true;
    }

    bool stop() override
    {
        return true;
    }
};

FileWatcher::FileWatcher(const QString &filePath, QObject *parent)
    : DAbstractFileWatcher(*new FileWatcherPrivate(this), DUrl::fromLocalFile(filePath), parent)
{

}

class TestDAbstractFileWatcher: public testing::Test
{
public:
    virtual void SetUp() override
    {
        std::cout << "start TestDAbstractFileWatcher" << std::endl;
        watcher = new FileWatcher("/tmp/1.txt");
    }

    virtual void TearDown() override
    {
        std::cout << "end TestDAbstractFileWatcher" << std::endl;
        if (watcher) {
            delete watcher;
            watcher = nullptr;
        }
    }

public:
    FileWatcher *watcher;
};
}

TEST_F(TestDAbstractFileWatcher, fileUrl)
{
    auto res = watcher->fileUrl();
    EXPECT_TRUE(res.toLocalFile().compare("/tmp/1.txt") == 0);
}

TEST_F(TestDAbstractFileWatcher, start_stop_watcher)
{
    auto res = watcher->startWatcher();
    EXPECT_TRUE(res);
    res = watcher->stopWatcher();
    EXPECT_TRUE(res);
}

TEST_F(TestDAbstractFileWatcher, restartWatcher)
{
    watcher->startWatcher();
    auto res = watcher->restartWatcher();
    EXPECT_TRUE(res);
}

TEST_F(TestDAbstractFileWatcher, setEnabledSubfileWatcher)
{
    watcher->setEnabledSubfileWatcher(DUrl("file:///tmp/1.txt"));
}

TEST_F(TestDAbstractFileWatcher, ghostSignal1)
{
    auto res = watcher->ghostSignal(DUrl("file:///tmp/1.txt"), &DAbstractFileWatcher::fileClosed, DUrl());
    EXPECT_TRUE(res);
}

TEST_F(TestDAbstractFileWatcher, ghostSignal2)
{
    auto res = watcher->ghostSignal(DUrl("file:///tmp/1.txt"), &DAbstractFileWatcher::fileMoved, DUrl(), DUrl());
    EXPECT_TRUE(res);
}

TEST_F(TestDAbstractFileWatcher, ghostSignal3)
{
    auto res = watcher->ghostSignal(DUrl("file:///tmp/1.txt"), &DAbstractFileWatcher::fileAttributeChanged, DUrl());
    EXPECT_TRUE(res);
}

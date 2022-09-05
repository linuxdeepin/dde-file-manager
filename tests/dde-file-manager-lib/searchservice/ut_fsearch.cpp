// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QStandardPaths>
#include <QUuid>
#include <QDir>

#include <gtest/gtest.h>


#define private public
#include "searcher/fsearch/fssearcher.h"

namespace {
class TestFsSearcher : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        DUrl targetUrl("");
        QStringList paths = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
        if (!paths.isEmpty()) {
            QString dirName = QUuid::createUuid().toString(QUuid::WithoutBraces);
            filePath = paths.at(0) + QDir::separator() + dirName;
            QDir dir;
            dir.mkdir(filePath);
            targetUrl = DUrl::fromLocalFile(filePath);
        }
        QString keyWord = "123qweasdzxc";
        search = new FsSearcher(targetUrl, keyWord);
    }

    static void TearDownTestCase()
    {
        QDir dir(filePath);
        if (dir.exists())
            dir.rmpath(filePath);
    }

    void SetUp() override
    {
        std::cout << "start TestSearchService" << std::endl;
    }

    void TearDown() override
    {
        std::cout << "end TestSearchService" << std::endl;
    }

public:
    static FsSearcher *search;
    static QString filePath;
};

FsSearcher *TestFsSearcher::search = { nullptr };
QString TestFsSearcher::filePath = "";

} // namespace

TEST_F(TestFsSearcher, tst_search) {
    EXPECT_NO_FATAL_FAILURE(search->search());
}

TEST_F(TestFsSearcher, tst_stop) {
    EXPECT_NO_FATAL_FAILURE(search->stop());
}

TEST_F(TestFsSearcher, tst_hasItem) {
    EXPECT_NO_FATAL_FAILURE(search->hasItem());
}

TEST_F(TestFsSearcher, tst_takeAll) {
    EXPECT_NO_FATAL_FAILURE(search->takeAll());
}

TEST_F(TestFsSearcher, tst_isSupported) {
    DUrl url = DUrl("file://" + filePath);
    EXPECT_NO_FATAL_FAILURE(FsSearcher::isSupported(url));
}

TEST_F(TestFsSearcher, tst_tryNotify) {
    EXPECT_NO_FATAL_FAILURE(search->tryNotify(););
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QStandardPaths>
#include <QUuid>
#include <QDir>

#include <gtest/gtest.h>


#define private public
#include "searcher/fulltext/fulltextsearcher.h"

namespace {
class TestFullTextSearcher : public testing::Test
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
        search = new FullTextSearcher(targetUrl, keyWord);
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
    static FullTextSearcher *search;
    static QString filePath;
};

FullTextSearcher *TestFullTextSearcher::search = { nullptr };
QString TestFullTextSearcher::filePath = "";

} // namespace

TEST_F(TestFullTextSearcher, tst_search) {
    EXPECT_NO_FATAL_FAILURE(search->search());
}

TEST_F(TestFullTextSearcher, tst_stop) {
    EXPECT_NO_FATAL_FAILURE(search->stop());
}

TEST_F(TestFullTextSearcher, tst_hasItem) {
    EXPECT_NO_FATAL_FAILURE(search->hasItem());
}

TEST_F(TestFullTextSearcher, tst_takeAll) {
    EXPECT_NO_FATAL_FAILURE(search->takeAll());
}

TEST_F(TestFullTextSearcher, tst_isSupport) {
    DUrl url = DUrl("file://" + filePath);
    EXPECT_NO_FATAL_FAILURE(search->isSupport(url));
}

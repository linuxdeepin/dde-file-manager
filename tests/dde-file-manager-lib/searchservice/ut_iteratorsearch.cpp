// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QStandardPaths>
#include <QUuid>
#include <QDir>

#include <gtest/gtest.h>


#define private public
#include "searcher/iterator/iteratorsearcher.h"

namespace {
class TestIteratorSearcher : public testing::Test
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
        search = new IteratorSearcher(targetUrl, keyWord);
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
    static IteratorSearcher *search;
    static QString filePath;
};

IteratorSearcher *TestIteratorSearcher::search = { nullptr };
QString TestIteratorSearcher::filePath = "";

} // namespace

TEST_F(TestIteratorSearcher, tst_search) {
    EXPECT_NO_FATAL_FAILURE(search->search());
}

TEST_F(TestIteratorSearcher, tst_stop) {
    EXPECT_NO_FATAL_FAILURE(search->stop());
}

TEST_F(TestIteratorSearcher, tst_hasItem) {
    EXPECT_NO_FATAL_FAILURE(search->hasItem());
}

TEST_F(TestIteratorSearcher, tst_takeAll) {
    EXPECT_NO_FATAL_FAILURE(search->takeAll());
}

TEST_F(TestIteratorSearcher, tst_tryNotify) {
    EXPECT_NO_FATAL_FAILURE(search->tryNotify());
}

TEST_F(TestIteratorSearcher, tst_doSearch) {
    EXPECT_NO_FATAL_FAILURE(search->doSearch());
}

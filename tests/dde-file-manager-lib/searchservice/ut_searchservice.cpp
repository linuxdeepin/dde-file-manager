/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#include "searchservice/searchservice.h"
#define private public
#include "searchservice/maincontroller/maincontroller.h"
#undef private
#include "stub-ext/stubext.h"

#include <QUuid>
#include <QStandardPaths>
#include <QDir>

#include <gtest/gtest.h>

namespace {
class TestSearchService : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        tstTaskId = QUuid::createUuid().toString(QUuid::WithoutBraces);

        QStringList paths = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
        if (!paths.isEmpty()) {
            QString dirName = QUuid::createUuid().toString(QUuid::WithoutBraces);
            filePath = paths.at(0) + QDir::separator() + dirName;
            QDir dir;
            dir.mkdir(filePath);
        }
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
    static QString tstTaskId;
    static QString filePath;
};

QString TestSearchService::tstTaskId = "";
QString TestSearchService::filePath = "";

} // namespace

TEST_F(TestSearchService, tst_search) {
    DUrl targetUrl = DUrl::fromLocalFile(filePath);
    QString keyWord = "123qweasdzxc";
    stub_ext::StubExt st;
    st.set_lamda(&MainController::doSearchTask, [](){
        return true;
    });
    EXPECT_NO_FATAL_FAILURE(searchServ->search(tstTaskId, targetUrl, keyWord));
}

TEST_F(TestSearchService, tst_stop) {
    EXPECT_NO_FATAL_FAILURE(searchServ->stop(tstTaskId));
}

TEST_F(TestSearchService, tst_matchedResults) {
    EXPECT_NO_FATAL_FAILURE(searchServ->matchedResults(tstTaskId));
}

TEST_F(TestSearchService, tst_createFullTextIndex) {
    EXPECT_NO_FATAL_FAILURE(searchServ->createFullTextIndex());
}

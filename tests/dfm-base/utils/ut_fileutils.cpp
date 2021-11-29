/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: luzhen<luzhen@uniontech.com>
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

#ifndef UT_FILEUTILS
#define UT_FILEUTILS
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/standardpaths.h"

#include <QDebug>
#include <QDir>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_FileUtils : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

TEST_F(UT_FileUtils, test_mkdir)
{
    UrlRoute::regScheme(SchemeTypes::kFile, "/");
    qInfo() << UrlRoute::pathToReal("/funningTest/");
    FileUtils::mkdir(UrlRoute::pathToReal("/"), "test");
    qInfo() << StandardPaths::location(StandardPaths::HomePath);
    FileUtils::mkdir(QUrl::fromLocalFile(StandardPaths::location(StandardPaths::HomePath)), "test");
}

#endif

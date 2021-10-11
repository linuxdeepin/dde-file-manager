#ifndef UT_FILEUTILS

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
    UrlRoute::schemeMapRoot("file","/");
    qInfo() << UrlRoute::pathToUrl("/funningTest/");
    FileUtils::mkdir(UrlRoute::pathToUrl("/"),"test");
    qInfo() << StandardPaths::location(StandardPaths::HomePath);
    FileUtils::mkdir(QUrl::fromLocalFile(StandardPaths::location(StandardPaths::HomePath)),"test");
}

#endif

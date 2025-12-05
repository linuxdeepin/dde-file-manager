// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "model/filefilter.h"
#include "model/fileprovider.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QUrl>
#include <QList>
#include <QTimerEvent>

using namespace ddplugin_canvas;

class UT_FileFilter : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        filter = new FileFilter();
        redundantFilter = new RedundantUpdateFilter(nullptr);
    }

    virtual void TearDown() override
    {
        if (filter) {
            delete filter;
            filter = nullptr;
        }

        if (redundantFilter) {
            delete redundantFilter;
            redundantFilter = nullptr;
        }

        stub.clear();
    }

public:
    QApplication *app = nullptr;
    FileFilter *filter = nullptr;
    RedundantUpdateFilter *redundantFilter = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_FileFilter, constructor_CreateFilter_InitializesCorrectly)
{
    EXPECT_NE(filter, nullptr);
}

TEST_F(UT_FileFilter, fileTraversalFilter_WithValidUrls_ReturnsFalse)
{
    QList<QUrl> urls;
    urls.append(QUrl("file:///tmp/test1.txt"));
    urls.append(QUrl("file:///tmp/test2.txt"));

    bool result = filter->fileTraversalFilter(urls);
    EXPECT_FALSE(result);
}

TEST_F(UT_FileFilter, fileDeletedFilter_WithValidUrl_ReturnsFalse)
{
    QUrl url("file:///tmp/test.txt");
    bool result = filter->fileDeletedFilter(url);
    EXPECT_FALSE(result);
}

TEST_F(UT_FileFilter, fileCreatedFilter_WithValidUrl_ReturnsFalse)
{
    QUrl url("file:///tmp/test.txt");
    bool result = filter->fileCreatedFilter(url);
    EXPECT_FALSE(result);
}

TEST_F(UT_FileFilter, fileRenameFilter_WithValidUrls_ReturnsFalse)
{
    QUrl oldUrl("file:///tmp/old.txt");
    QUrl newUrl("file:///tmp/new.txt");
    bool result = filter->fileRenameFilter(oldUrl, newUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_FileFilter, fileUpdatedFilter_WithValidUrl_ReturnsFalse)
{
    QUrl url("file:///tmp/test.txt");
    bool result = filter->fileUpdatedFilter(url);
    EXPECT_FALSE(result);
}

TEST_F(UT_FileFilter, redundantUpdateFilter_WithValidUrl_HandlesUpdates)
{
    QUrl url("file:///tmp/test.txt");
    bool result = redundantFilter->fileUpdatedFilter(url);
    EXPECT_FALSE(result);
}

TEST_F(UT_FileFilter, checkUpdate_WithValidFilter_CallsUpdate)
{
    EXPECT_NO_THROW(redundantFilter->checkUpdate());
}

TEST_F(UT_FileFilter, timerEvent_WithValidEvent_HandlesTimer)
{
    QTimerEvent event(123);
    EXPECT_NO_THROW(redundantFilter->timerEvent(&event));
}
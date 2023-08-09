// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_LOCALILEICONPRIVDER
#define UT_LOCALILEICONPRIVDER

#include <stubext.h>
#include <dfm-base/file/local/localfileiconprovider.h>

#include <dfm-io/dwatcher.h>

#include <QDir>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE

class UT_LocalFileIconProvider : public testing::Test
{
public:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    ~UT_LocalFileIconProvider() override;
};

UT_LocalFileIconProvider::~UT_LocalFileIconProvider() {

}

TEST_F(UT_LocalFileIconProvider, testLocalFileIconProvider)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    LocalFileIconProvider *privder = LocalFileIconProvider::globalProvider();
    QFileInfo info(url.path());
    EXPECT_TRUE(privder->icon(info).name().isEmpty());

    QString iconName("test");
    EXPECT_TRUE(privder->icon(iconName).isNull());
}

#endif

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>

#include "stubext.h"

#include "utils/servicemanager.h"
#include "utils/vaulthelper.h"

using namespace dfmplugin_vault;

class ServiceManagerImpl : public testing::Test
{
public:
    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(ServiceManagerImpl, BasicViewFieldFunc)
{
    QUrl url = QUrl("dfmvault:///foo.txt");
    ServiceManager::ExpandFieldMap map = ServiceManager::basicViewFieldFunc(url);

    EXPECT_TRUE(map.contains(kFieldReplace));
    auto basic = map.value(kFieldReplace);
    EXPECT_TRUE(basic.contains(kFilePosition));
    auto pair = basic.value(kFilePosition);
    EXPECT_EQ(pair.second, url.url());
}

TEST_F(ServiceManagerImpl, DetailViewFieldFunc_RootUrl)
{
    ServiceManager::ExpandFieldMap map = ServiceManager::detailViewFieldFunc(VaultHelper::instance()->rootUrl());

    EXPECT_TRUE(map.contains(kFieldInsert));
    auto basic = map.value(kFieldInsert);
    EXPECT_TRUE(basic.contains(kFileInterviewTime));
}

TEST_F(ServiceManagerImpl, DetailViewFieldFunc_NonRootUrl)
{
    QUrl url = QUrl("dfmvault:///child.txt");
    ServiceManager::ExpandFieldMap map = ServiceManager::detailViewFieldFunc(url);

    EXPECT_FALSE(map.contains(kFieldInsert));
}

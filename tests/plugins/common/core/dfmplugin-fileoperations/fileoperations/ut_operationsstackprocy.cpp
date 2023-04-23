// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/operationsstackproxy.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <gtest/gtest.h>


DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_OperationsStackProxy : public testing::Test
{
public:
    void SetUp() override {
        // 注册路由
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
        // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    }
    void TearDown() override {}
    ~UT_OperationsStackProxy() override;
};

UT_OperationsStackProxy::~UT_OperationsStackProxy(){

}

TEST_F(UT_OperationsStackProxy, testOperationsStackProxy)
{
    EXPECT_NO_FATAL_FAILURE(OperationsStackProxy::instance().saveOperations(QVariantMap()));
    EXPECT_NO_FATAL_FAILURE(OperationsStackProxy::instance().revocationOperations());
    EXPECT_NO_FATAL_FAILURE(OperationsStackProxy::instance().cleanOperations());
}

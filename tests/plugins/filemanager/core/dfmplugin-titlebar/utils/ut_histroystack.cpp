// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils/historystack.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/utils/networkutils.h>

#include "stubext.h"

#include <gtest/gtest.h>

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE

class UT_HistoryStack : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Scheme::kFile);

        url = QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
        url.setScheme(Scheme::kFile);
    }
    void TearDown() override
    {
        stub.clear();
    }

    QUrl url {};
    QString key {"theKey"};
    stub_ext::StubExt stub;
};


TEST(UT_HistoryStack, Bug_199653_checkPathIsExist)
{
    stub_ext::StubExt st;
    QUrl url("file:///run/user/1000/gvfs/mtp:host=HUAWEI_testeeeeeeeeeeeeee");
    HistoryStack stack(UINT16_MAX);
    EXPECT_FALSE(stack.checkPathIsExist(url));

    st.set_lamda(&NetworkUtils::checkFtpOrSmbBusy, []{ __DBG_STUB_INVOKE__ return true; });
    EXPECT_FALSE(stack.checkPathIsExist(url));
}

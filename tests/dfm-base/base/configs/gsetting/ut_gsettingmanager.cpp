// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <dfm-base/base/configs/gsetting/gsettingmanager.h>
#include <dfm-base/base/configs/gsetting/private/gsettingmanager_p.h>

#include <QGSettings>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
class UT_GSettingManager : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override
    {
        stub.clear();
        if (d) {
            auto ptrs = d->settings.values();
            std::for_each(ptrs.begin(), ptrs.end(), [](QGSettings *set) { delete set; });
            d->settings.clear();
        }
    }

    static void SetUpTestCase()
    {
        mng = GSettingManager::instance();
        d = mng->d.data();
    }

private:
    stub_ext::StubExt stub;
    static GSettingManager *mng;
    static GSettingManagerPrivate *d;
};

GSettingManager *UT_GSettingManager::mng { nullptr };
GSettingManagerPrivate *UT_GSettingManager::d { nullptr };

TEST_F(UT_GSettingManager, IsSchemaInstalled)
{
    stub.set_lamda(QGSettings::isSchemaInstalled, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(mng->isSchemaInstalled("hello"));
    EXPECT_NO_FATAL_FAILURE(mng->isSchemaInstalled("world"));
}

TEST_F(UT_GSettingManager, AddSettings)
{
    d->settings.insert("hello", new QGSettings("com.deepin.dde.dock.module.disk-mount"));
    QString err;
    EXPECT_FALSE(mng->addSettings("hello", "world", &err));
    EXPECT_TRUE(err.endsWith("already added"));

    EXPECT_TRUE(mng->addSettings("com.deepin.dde.filemanager.contextmenu", "", &err));
}

TEST_F(UT_GSettingManager, RemoveSettings)
{
    QString err;
    EXPECT_TRUE(mng->addSettings("com.deepin.dde.filemanager.contextmenu", "", &err));
    EXPECT_TRUE(mng->removeSettings("com.deepin.dde.filemanager.contextmenu", &err));
}

TEST_F(UT_GSettingManager, Setting)
{
    QString err;
    EXPECT_TRUE(mng->addSettings("com.deepin.dde.filemanager.contextmenu", "", &err));
    EXPECT_TRUE(mng->setting("com.deepin.dde.filemanager.contextmenu"));
}

TEST_F(UT_GSettingManager, Get)
{
    EXPECT_FALSE(mng->get("hello", "world").isValid());
    QString err;
    EXPECT_TRUE(mng->addSettings("com.deepin.dde.filemanager.contextmenu", "", &err));
    stub.set_lamda(&QGSettings::get, [] { __DBG_STUB_INVOKE__ return QVariant("yes"); });
    EXPECT_TRUE(mng->get("com.deepin.dde.filemanager.contextmenu", "whatever").isValid());
    EXPECT_TRUE(mng->get("com.deepin.dde.filemanager.contextmenu", "whatever").toString() == "yes");
}

TEST_F(UT_GSettingManager, Set)
{
    EXPECT_NO_FATAL_FAILURE(mng->set("hello", "whatever", "emmmm..."));

    QString err;
    EXPECT_TRUE(mng->addSettings("com.deepin.dde.filemanager.contextmenu", "", &err));
    stub.set_lamda(&QGSettings::set, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(mng->set("com.deepin.dde.filemanager.contextmenu", "whatever", "emmmm..."));
}

TEST_F(UT_GSettingManager, TrySet)
{
    EXPECT_NO_FATAL_FAILURE(mng->trySet("hello", "whatever", "emmmm..."));
    EXPECT_FALSE(mng->trySet("hello", "whatever", "emmmm..."));

    QString err;
    EXPECT_TRUE(mng->addSettings("com.deepin.dde.filemanager.contextmenu", "", &err));
    stub.set_lamda(&QGSettings::trySet, [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_NO_FATAL_FAILURE(mng->trySet("com.deepin.dde.filemanager.contextmenu", "whatever", "it's fine"));
    EXPECT_TRUE(mng->trySet("com.deepin.dde.filemanager.contextmenu", "ok", "it's fine"));
}

TEST_F(UT_GSettingManager, Keys)
{
    EXPECT_TRUE(mng->keys("whatever").isEmpty());
}

TEST_F(UT_GSettingManager, Choices)
{
    EXPECT_TRUE(mng->choices("whatever", "anykey").isEmpty());
}

TEST_F(UT_GSettingManager, Reset)
{
    EXPECT_NO_FATAL_FAILURE(mng->reset("whatever", "suit your self"));

    QString err;
    EXPECT_TRUE(mng->addSettings("com.deepin.dde.filemanager.contextmenu", "", &err));
    stub.set_lamda(&QGSettings::reset, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(mng->reset("com.deepin.dde.filemanager.contextmenu", "key..."));
}

/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/menus/sendtodiscmenuscene.h"
#include "plugins/common/dfmplugin-burn/menus/sendtodiscmenuscene_p.h"
#include "dfm-base/base/device/deviceutils.h"

#include <QMenu>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPBURN_USE_NAMESPACE

class UT_SendToDiscMenuCreator : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

// TEST_F(UT_SendToDiscMenuCreator, ){}

class UT_SendToDiscMenuScene : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

// TEST_F(UT_SendToDiscMenuScene, ){}

class UT_SendToDiscMenuScenePrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        scene = new SendToDiscMenuScene;
        d = scene->d.data();
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete scene;
    }

private:
    stub_ext::StubExt stub;
    SendToDiscMenuScene *scene { nullptr };
    SendToDiscMenuScenePrivate *d { nullptr };
};

TEST_F(UT_SendToDiscMenuScenePrivate, AddToSendTo)
{
    EXPECT_NO_FATAL_FAILURE(d->addToSendto(nullptr));
    QMenu *m = new QMenu();
    EXPECT_NO_FATAL_FAILURE(d->addToSendto(m));

    QVariantMap dev { { "Device", "/dev/sr0" } };
    d->destDeviceDataGroup.append(dev);
    EXPECT_NO_FATAL_FAILURE(d->addToSendto(m));

    auto act = new QAction(m);
    act->setProperty("actionID", "send-to");
    m->addAction(act);
    EXPECT_NO_FATAL_FAILURE(d->addToSendto(m));

    act->setMenu(new QMenu);
    typedef QString (*ConvertName)(const QVariantMap &);
    stub.set_lamda(static_cast<ConvertName>(DeviceUtils::convertSuitableDisplayName), [] { __DBG_STUB_INVOKE__ return "100G"; });
    EXPECT_NO_FATAL_FAILURE(d->addToSendto(m));
    delete m;
}
// TEST_F(UT_SendToDiscMenuScenePrivate, ){}

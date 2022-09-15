/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhuangshu<xushitong@uniontech.com>
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
#include "plugins/common/dfmplugin-bookmark/controller/bookmarkmanager.h"
#include "plugins/common/dfmplugin-bookmark/controller/defaultitemmanager.h"
//#include "dfm-base/utils/systempathutil.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

#include <gtest/gtest.h>
#include <gtest/gtest_prod.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
DPBOOKMARK_USE_NAMESPACE

class UT_BookmarkManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_BookmarkManager, initDefaultItems)
{
    DefaultItemManager::instance()->initDefaultItems();
    QList<BookmarkData> list = DefaultItemManager::instance()->defaultItemInitOrder();

    EXPECT_TRUE(list.count() == 7);
}

TEST_F(UT_BookmarkManager, addPluginItem)
{
    const QString &nameKey = "Recent";
    const QString &displayName = QObject::tr("Recent");

    QVariantMap map {
        { "Property_Key_NameKey", nameKey },
        { "Property_Key_DisplayName", displayName },
        { "Property_Key_Url", QUrl("recent:/") },
        { "Property_Key_Index", 0 },
        { "Property_Key_IsDefaultItem", true }
    };
    DefaultItemManager::instance()->addPluginItem(map);
    EXPECT_TRUE(DefaultItemManager::instance()->pluginItems().count() > 0);
    EXPECT_TRUE(DefaultItemManager::instance()->pluginItems().first().name == nameKey);
};

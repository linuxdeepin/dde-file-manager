/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: gongheng<gongheng@uniontech.com>
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

#include <gtest/gtest.h>

#include "stub.h"
#include "dfmfactoryloader.h"
#include "dfmbaseview.h"
#include "views/computerview.h"
#include "interfaces/plugins/dfmviewfactory.h"
#include "interfaces/plugins/private/dfmviewfactory_p.h"

DFM_USE_NAMESPACE

namespace  {
    class TestDFMViewFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMViewFactory";
        }
        void TearDown() override
        {
            std::cout << "end TestDFMViewFactory";
        }
    public:
    };


}

TEST_F(TestDFMViewFactory, testCreate)
{
    DFMBaseView *pview = DFMViewFactory::create("/views");
    EXPECT_EQ(pview, nullptr);
}

TEST_F(TestDFMViewFactory, testKeys)
{
    QMultiMap<int, QString>(*stub_keyMap)() = []()->QMultiMap<int, QString>{
        QMultiMap<int, QString> map;
        map.insert(0, "utest1");
        return map;
    };
    Stub stu;
    stu.set(ADDR(DFMFactoryLoader, keyMap), stub_keyMap);

    QStringList lst = DFMViewFactory::keys();
    EXPECT_EQ(lst.count(), 1);
}

TEST_F(TestDFMViewFactory, testViewIsSuitedWithUrl)
{
    QMap<const DFMBaseView*, int> map;
    ComputerView* view1 = new ComputerView();
    map.insert(view1, 1);

    DFMViewFactoryPrivate::viewToLoaderIndex = map;

    bool b = DFMViewFactory::viewIsSuitedWithUrl(view1, DUrl("file:///utest"));
    EXPECT_EQ(b, false);
}

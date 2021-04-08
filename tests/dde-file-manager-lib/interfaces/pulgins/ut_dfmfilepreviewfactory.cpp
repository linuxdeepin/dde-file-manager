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
#include <qmap.h>

#include "stub.h"
#include "plugins/dfmfactoryloader.h"
#include "dialogs/filepreviewdialog.h"
#include "dfmbaseview.h"
#include "views/computerview.h"

#include "interfaces/plugins/dfmfilepreviewfactory.h"
#include "interfaces/plugins/private/dfmfilepreviewfactory_p.h"

#include <QTimer>

DFM_USE_NAMESPACE

namespace  {
    class TestDFMFilePreviewFactory : public testing::Test
    {
    public:
        void SetUp() override
        {
            std::cout << "start TestDFMFilePreviewFactory";
        }
        void TearDown() override
        {
            QEventLoop loop;
            QTimer::singleShot(100, nullptr, [&loop]() {
                loop.exit();
            });
            loop.exec();

            std::cout << "end TestDFMFilePreviewFactory";
        }
    public:
    };
}

TEST_F(TestDFMFilePreviewFactory, testCreate)
{
    bool(*stub_isRootUser)() = []()->bool{
        return true;
    };
    Stub stu;
    stu.set(ADDR(DFMGlobal, isRootUser), stub_isRootUser);

    DFMFilePreview *pview = DFMFilePreviewFactory::create("/video");
    EXPECT_EQ(pview, nullptr);
}


TEST_F(TestDFMFilePreviewFactory, testKeys)
{
    QMultiMap<int, QString>(*stub_keyMap)() = []()->QMultiMap<int, QString>{
        QMultiMap<int, QString> map;
        map.insert(0, "utest1");
        return map;
    };
    Stub stu;
    stu.set(ADDR(DFMFactoryLoader, keyMap), stub_keyMap);

    QStringList lst = DFMFilePreviewFactory::keys();
    EXPECT_EQ(lst.count(), 1);
}

TEST_F(TestDFMFilePreviewFactory, testIsSuitedWithKey)
{
    QString index("");
    bool b = DFMFilePreviewFactory::isSuitedWithKey(nullptr, index);
    EXPECT_EQ(b, false);
}

TEST_F(TestDFMFilePreviewFactory, testIsSuitedWithKey2)
{
    QMap<const UnknowFilePreview*, int> map;
    UnknowFilePreview* view1 = new UnknowFilePreview();
    DFMFilePreviewFactoryPrivate::previewToLoaderIndex[view1] = 1;

    QString index("");
    bool b = DFMFilePreviewFactory::isSuitedWithKey(view1, index);
    EXPECT_EQ(b, false);
    if (view1) {
        delete view1;
        view1 = nullptr;
    }

}

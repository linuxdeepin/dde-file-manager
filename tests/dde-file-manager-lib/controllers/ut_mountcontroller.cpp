/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#include <gtest/gtest.h>
#include <QTimer>

#include "controllers/mountcontroller.h"
#include "dfmevent.h"

namespace {
class TestMountController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestMountController";
        controller = new MountController;
    }

    void TearDown() override
    {
        std::cout << "end TestMountController";
        QEventLoop loop;
        QTimer::singleShot(20, nullptr, [&loop]{
            loop.exit();
        });
        loop.exec();
        delete controller;
        controller = nullptr;
    }

public:
    MountController *controller;
};
} // namespace

TEST_F(TestMountController, createFileInfo)
{
    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("file:///"));
    EXPECT_TRUE(controller->createFileInfo(event) != nullptr);
}

TEST_F(TestMountController, getChildren)
{
    auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl("file:///"), QStringList(), QDir::AllDirs);
    EXPECT_TRUE(controller->getChildren(event).count() == 0);

    event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, DUrl(), QStringList(), QDir::AllDirs);
    EXPECT_TRUE(controller->getChildren(event).count() == 0);

    DUrl u;
    u.setFragment("udisks");
    event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr, u, QStringList(), QDir::AllDirs);
    EXPECT_TRUE(controller->getChildren(event).count() == 0);

}

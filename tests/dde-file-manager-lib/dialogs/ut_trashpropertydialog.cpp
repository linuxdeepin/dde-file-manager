/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#include "stub.h"
#include "io/dfilestatisticsjob.h"

#include <gtest/gtest.h>
#include <QLabel>

#define private public
#include "dialogs/trashpropertydialog.h"

DFM_USE_NAMESPACE

namespace  {
    class TestTrashPropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            void(*stu_start)(const DUrlList &) = [](const DUrlList &){
            };
            Stub stu;
            stu.set((void(DFileStatisticsJob::*)(const DUrlList &))ADDR(DFileStatisticsJob, start), stu_start);
            m_pTester = new TrashPropertyDialog(DUrl::fromTrashFile("/"));
            std::cout << "start TestTrashPropertyDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestTrashPropertyDialog";
        }
    public:
        TrashPropertyDialog *m_pTester;
    };
}

TEST_F(TestTrashPropertyDialog, testInit)
{
    EXPECT_NE(m_pTester, nullptr);
}

TEST_F(TestTrashPropertyDialog, testUpdateFolderSize)
{
    m_pTester->updateFolderSize(1);
    QString str = m_pTester->m_sizeLabel->text();
    EXPECT_TRUE(str == "1 B");
}

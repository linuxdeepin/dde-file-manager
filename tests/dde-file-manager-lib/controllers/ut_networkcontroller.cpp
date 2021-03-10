/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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
#include <dfmevent.h>

#include "controllers/networkcontroller.h"

#include <QDateTime>
#include <QProcess>
#include <QUrl>
#include <QFile>
#include <QDir>

using namespace testing;

class NetworkControllerTest:public testing::Test{

public:
    NetworkController controller;
    virtual void SetUp() override{
        std::cout << "start NetworkControllerTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end NetworkControllerTest" << std::endl;
    }
};

TEST_F(NetworkControllerTest, can_createFileInfo)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("smb:///"));
    // EXPECT_TRUE(controller.createFileInfo(event) != nullptr);
}

TEST_F(NetworkControllerTest, can_createDirIterator)
{
    // 阻塞CI
    // auto event = dMakeEventPointer<DFMCreateDiriterator>(nullptr,
    //                                                      DUrl("smb:///"),
    //                                                      QStringList(),
    //                                                      QDir::AllEntries,
    //                                                      QDirIterator::NoIteratorFlags);

    // auto itera = controller.createDirIterator(event);
    // EXPECT_TRUE(itera->hasNext());
    // while (itera->hasNext()) {
    //     qDebug()<< "next >> " << itera->fileUrl();
    //     qDebug()<< "next >> " << itera->fileInfo();
    //     qDebug()<< "next >> " << itera->url();
    //     qDebug()<< "next >> " << itera->fileName();
    //     EXPECT_TRUE(!itera->next().isEmpty());
    //     itera->close();
    // }
}

TEST_F(NetworkControllerTest, can_getChildren)
{
    auto event = dMakeEventPointer<DFMGetChildrensEvent>(nullptr,
                                                         DUrl("smb:///"),
                                                         QStringList(),
                                                         nullptr);

    EXPECT_TRUE(!controller.getChildren(event).isEmpty());
}

TEST_F(NetworkControllerTest, can_pasteFile)
{
    auto event = dMakeEventPointer<DFMPasteEvent>(nullptr,
                                                  DFMGlobal::CutAction,
                                                  DUrl("smb:///"),
                                                  DUrlList());

    EXPECT_TRUE(controller.pasteFile(event) == DUrlList());
}

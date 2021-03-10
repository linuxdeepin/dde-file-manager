/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhengyouge<zhengyouge@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
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

#include "interfaces/dfmevent.h"
#include "durl.h"

#include <gtest/gtest.h>
#include <QWidget>
#include <QSharedPointer>
#include <QJsonObject>
#include <QJsonArray>

#include "interfaces/dfmstandardpaths.h"

namespace {
class TestDFMEvent : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMEvent" << std::endl;
        widget = new QWidget();
        event = new DFMEvent(DFMEvent::OpenFile, widget);
    }

    void TearDown() override
    {
        std::cout << "end TestDFMEvent" << std::endl;
        if (event) {
            delete event;
            event = nullptr;
        }

        if (widget) {
            delete widget;
            widget = nullptr;
        }
    }

public:
    DFMEvent *event = nullptr;
    QWidget *widget = nullptr;
};
}

TEST_F(TestDFMEvent, nameToType)
{
    EXPECT_EQ(DFMEvent::OpenFile, DFMEvent::nameToType("OpenFile"));
}

TEST_F(TestDFMEvent, typeToName)
{
    EXPECT_EQ(QLatin1String("Unknow"), DFMEvent::typeToName(DFMEvent::UnknowType));
    EXPECT_EQ(QLatin1String("OpenFile"), DFMEvent::typeToName(DFMEvent::OpenFile));
    EXPECT_EQ(QLatin1String("OpenFileByApp"), DFMEvent::typeToName(DFMEvent::OpenFileByApp));
    EXPECT_EQ(QLatin1String("CompressFiles"), DFMEvent::typeToName(DFMEvent::CompressFiles));
    EXPECT_EQ(QLatin1String("DecompressFile"), DFMEvent::typeToName(DFMEvent::DecompressFile));
    EXPECT_EQ(QLatin1String("DecompressFileHere"), DFMEvent::typeToName(DFMEvent::DecompressFileHere));
    EXPECT_EQ(QLatin1String("WriteUrlsToClipboard"), DFMEvent::typeToName(DFMEvent::WriteUrlsToClipboard));
    EXPECT_EQ(QLatin1String("RenameFile"), DFMEvent::typeToName(DFMEvent::RenameFile));
    EXPECT_EQ(QLatin1String("DeleteFiles"), DFMEvent::typeToName(DFMEvent::DeleteFiles));
    EXPECT_EQ(QLatin1String("MoveToTrash"), DFMEvent::typeToName(DFMEvent::MoveToTrash));
    EXPECT_EQ(QLatin1String("RestoreFromTrash"), DFMEvent::typeToName(DFMEvent::RestoreFromTrash));
    EXPECT_EQ(QLatin1String("PasteFile"), DFMEvent::typeToName(DFMEvent::PasteFile));
    EXPECT_EQ(QLatin1String("Mkdir"), DFMEvent::typeToName(DFMEvent::Mkdir));
    EXPECT_EQ(QLatin1String("TouchFile"), DFMEvent::typeToName(DFMEvent::TouchFile));
    EXPECT_EQ(QLatin1String("OpenFileLocation"), DFMEvent::typeToName(DFMEvent::OpenFileLocation));
    EXPECT_EQ(QLatin1String("CreateSymlink"), DFMEvent::typeToName(DFMEvent::CreateSymlink));
    EXPECT_EQ(QLatin1String("FileShare"), DFMEvent::typeToName(DFMEvent::FileShare));
    EXPECT_EQ(QLatin1String("CancelFileShare"), DFMEvent::typeToName(DFMEvent::CancelFileShare));
    EXPECT_EQ(QLatin1String("OpenInTerminal"), DFMEvent::typeToName(DFMEvent::OpenInTerminal));
    EXPECT_EQ(QLatin1String("GetChildrens"), DFMEvent::typeToName(DFMEvent::GetChildrens));
    EXPECT_EQ(QLatin1String("CreateFileInfo"), DFMEvent::typeToName(DFMEvent::CreateFileInfo));
    EXPECT_EQ(QLatin1String("CreateDiriterator"), DFMEvent::typeToName(DFMEvent::CreateDiriterator));
    EXPECT_EQ(QLatin1String("CreateGetChildrensJob"), DFMEvent::typeToName(DFMEvent::CreateGetChildrensJob));
    EXPECT_EQ(QLatin1String("CreateFileWatcher"), DFMEvent::typeToName(DFMEvent::CreateFileWatcher));
    EXPECT_EQ(QLatin1String("ChangeCurrentUrl"), DFMEvent::typeToName(DFMEvent::ChangeCurrentUrl));
    EXPECT_EQ(QLatin1String("OpenNewWindow"), DFMEvent::typeToName(DFMEvent::OpenNewWindow));
    EXPECT_EQ(QLatin1String("OpenUrl"), DFMEvent::typeToName(DFMEvent::OpenUrl));
    EXPECT_EQ(QLatin1String("MenuAction"), DFMEvent::typeToName(DFMEvent::MenuAction));
    EXPECT_EQ(QLatin1String("SaveOperator"), DFMEvent::typeToName(DFMEvent::SaveOperator));
    EXPECT_EQ(QLatin1String("Revocation"), DFMEvent::typeToName(DFMEvent::Revocation));
    EXPECT_EQ(QLatin1String("CleanSaveOperator"), DFMEvent::typeToName(DFMEvent::CleanSaveOperator));
    EXPECT_EQ(QLatin1String("GetTagsThroughFiles"), DFMEvent::typeToName(DFMEvent::GetTagsThroughFiles));
}

TEST_F(TestDFMEvent, setType)
{
    event->setType(DFMEvent::OpenFile);
    EXPECT_EQ(DFMEvent::OpenFile, event->type());
}

TEST_F(TestDFMEvent, setSender)
{
    event->setSender(widget);
    EXPECT_TRUE(event->sender() != nullptr);
}

TEST_F(TestDFMEvent, setAccepted)
{
    event->setAccepted(true);
    EXPECT_TRUE(event->isAccepted());
}

TEST_F(TestDFMEvent, accept)
{
    event->accept();
    EXPECT_TRUE(event->isAccepted());
}

TEST_F(TestDFMEvent, ignore)
{
    event->ignore();
    EXPECT_FALSE(event->isAccepted());
}

TEST_F(TestDFMEvent, windowIdByQObject)
{
    quint64 id = DFMEvent::windowIdByQObject(widget);
    EXPECT_EQ(0, id);
}

TEST_F(TestDFMEvent, setWindowId)
{
    event->setWindowId(1);
    EXPECT_EQ(1, event->windowId());
}

TEST_F(TestDFMEvent, setData)
{
    DUrl url = DUrl::fromLocalFile("/home");
    event->setData(url);
    DUrlList urls = event->handleUrlList();
    EXPECT_FALSE(urls.isEmpty());
    QVariant path = event->data();
    EXPECT_EQ(QLatin1String("/home"), qvariant_cast<DUrl>(path).toLocalFile());
    EXPECT_EQ(url, event->fileUrl());
    EXPECT_TRUE(event->fileUrlList().isEmpty());
}

TEST_F(TestDFMEvent, setCutData)
{
    DUrl url = DUrl::fromLocalFile(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + "/test.txt");
    event->setCutData(url);
    QString path = qvariant_cast<DUrl>(event->cutData()).toLocalFile();
    EXPECT_EQ(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath) + "/test.txt", path);
}

TEST_F(TestDFMEvent, fromJson)
{
    QJsonObject json;
    json.insert("url", "/home/1.txt");
    json.insert("appName", "vim");
    json.insert("action", 1);
    json.insert("from", "/home/1.txt");
    json.insert("to", "/home/2.txt");
    json.insert("silent", true);
    json.insert("force", true);
    json.insert("targetUrl", "/home/2.txt");
    json.insert("fileUrl", "/home/2.txt");
    json.insert("toUrl", "/home/3.txt");
    json.insert("name", "share");
    json.insert("isWritable", true);
    json.insert("allowGuest", true);
    json.insert("filters", 2);
    json.insert("mode", 0);
    json.insert("isEnter", true);
    json.insert("currentUrl", "/home/1.txt");

    QJsonArray urlList;
    urlList.append("/home/1.txt");
    urlList.append("/home/2.txt");
    json.insert("urlList", urlList);

    QJsonArray nameFilters;
    nameFilters.append("1.txt");
    json.insert("nameFilters", nameFilters);

    EXPECT_TRUE(event->fromJson(DFMEvent::OpenFile, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::OpenFileByApp, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::OpenFilesByApp, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::CompressFiles, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::DecompressFile, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::DecompressFileHere, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::WriteUrlsToClipboard, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::RenameFile, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::DeleteFiles, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::MoveToTrash, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::RestoreFromTrash, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::PasteFile, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::Mkdir, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::TouchFile, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::OpenFileLocation, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::CreateSymlink, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::FileShare, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::CancelFileShare, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::OpenInTerminal, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::GetChildrens, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::CreateFileInfo, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::CreateDiriterator, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::CreateGetChildrensJob, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::CreateFileWatcher, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::ChangeCurrentUrl, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::OpenNewWindow, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::OpenUrl, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::MenuAction, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::Back, json) != nullptr);
    EXPECT_TRUE(event->fromJson(DFMEvent::Forward, json) != nullptr);
}

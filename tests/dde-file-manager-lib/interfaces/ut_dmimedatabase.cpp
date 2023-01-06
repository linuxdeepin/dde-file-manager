// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QFileInfo>

#include "dmimedatabase.h"
#include "testhelper.h"
#include "stubext.h"

using namespace testing;
using namespace stub_ext;
DFM_USE_NAMESPACE

class DMimeDatabaseTest:public testing::Test{

public:
    DMimeDatabase db;
    virtual void SetUp() override{
        std::cout << "start DFileCopyQueueTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileCopyQueueTest" << std::endl;
    }
};

TEST_F(DMimeDatabaseTest, can_get_mimeTypeForUrl)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("/etc/presage.xml");
    EXPECT_TRUE(db.mimeTypeForUrl(url).name().contains("xml"));
    StubExt stl;
    stl.set_lamda(&DUrl::isLocalFile,[](){return false;});
    EXPECT_TRUE(db.mimeTypeForUrl(url).name().contains("xml"));
}

//TEST_F(DMimeDatabaseTest, can_get_mimeTypeForFile)
//{
//    DUrl url;
//    url.setScheme(FILE_SCHEME);
//    url.setPath("/etc/presage.xml");
//    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile()).name().contains("xml"));
//    EXPECT_FALSE(db.mimeTypeForFile(QString("/proc/kmsg"),
//                                    QMimeDatabase::MatchExtension).name().contains("xml"));
//    StubExt stl;
//    QFileInfo info("/tmp/ooomsg.lock");

//    stl.set_lamda(&QRegularExpressionMatch::hasMatch, [](){return true;});
//    QMimeType (*mimeTypeForFile1)(const QFileInfo &, QMimeDatabase::MatchMode) = []
//            (const QFileInfo &, QMimeDatabase::MatchMode){
//        return QMimeType();
//    };
//    stl.set((QMimeType (QMimeDatabase::*)(const QFileInfo &, QMimeDatabase::MatchMode)const)
//            ADDR(QMimeDatabase,mimeTypeForFile),mimeTypeForFile1);
//    EXPECT_TRUE(db.mimeTypeForFile(info).name().isEmpty());

//    EXPECT_TRUE(db.mimeTypeForFile(info,QMimeDatabase::MatchDefault,
//                                   QString("11"),true).name().isEmpty());

//    EXPECT_TRUE(db.mimeTypeForFile(info,QMimeDatabase::MatchDefault,
//                                   QString("11"),true).name().isEmpty());
//    QFileInfo info2("/etc/presage.xml");
//    stl.reset((QMimeType (QMimeDatabase::*)(const QFileInfo &, QMimeDatabase::MatchMode)const)
//            ADDR(QMimeDatabase,mimeTypeForFile));
//    stl.set_lamda(&QFileInfo::suffix,[](){return QString("xls");});
//    stl.set_lamda(&QMimeType::name,[](){return QString("application/zip");});
//    EXPECT_FALSE(db.mimeTypeForFile(info2,QMimeDatabase::MatchDefault,
//                                   QString("12"),true).name().isEmpty());
//}

TEST_F(DMimeDatabaseTest, can_get_mimeTypeForFile_othertype)
{
    DUrl url;
    url.setScheme(FILE_SCHEME);
    url.setPath("/etc/presage.xml");
    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchExtension).name().contains("xml"));
}

//TEST_F(DMimeDatabaseTest, can_get_mimeTypeForFile_gvfs)
//{
//    DUrl url;
//    url.setScheme(FILE_SCHEME);
//    url.setPath("/etc/presage.xml");
//    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchExtension, QString("qwerty"),false).name().contains("xml"));
//    EXPECT_FALSE(db.mimeTypeForFile(QString("/etc"),QMimeDatabase::MatchExtension, QString("asdasda"),false).name().contains("xml"));
//    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchExtension, QString("qwerty"),false).name().contains("xml"));
//    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchDefault, QString("fdsafd"),false).name().contains("xml"));
//    EXPECT_TRUE(db.mimeTypeForFile(url.toLocalFile(),QMimeDatabase::MatchDefault, QString("fdsafed"),true).name().contains("xml"));
//}

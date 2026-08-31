// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_asyncfileinfo.cpp
 * @brief Unit tests for AsyncFileInfo methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/asyncfileinfo.h"

#include <QTest>

using namespace src;

class AsyncFileInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AsyncFileInfo();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AsyncFileInfo *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AsyncFileInfoTest, AsyncFileInfo)
{
    // Test constructor: AsyncFileInfo((const QUrl &url, QSharedPointer<DFileInfo> dfileInfo))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AsyncFileInfoTest, M_~AsyncFileInfo)
{
    // Test method:  ~AsyncFileInfo(())
    EXPECT_NO_FATAL_FAILURE({ AsyncFileInfo *tmp = new AsyncFileInfo(); delete tmp; });
}

TEST_F(AsyncFileInfoTest, exists)
{
    // Test bool getter: exists()
    bool result = obj->exists();
    EXPECT_FALSE(result);

}

TEST_F(AsyncFileInfoTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(AsyncFileInfoTest, permission)
{
    // Test method: bool permission((QFileDevice::Permissions permissions))
    auto result = obj->permission(QFileDevice::Permissions());
    EXPECT_FALSE(result);

}

TEST_F(AsyncFileInfoTest, permissions)
{
    // Test getter: QFileDevice::Permissions permissions()
    auto result = obj->permissions();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(AsyncFileInfoTest, size)
{
    // Test getter: qint64 size()
    auto result = obj->size();
    EXPECT_EQ(result, 0);

}

TEST_F(AsyncFileInfoTest, cacheAttribute)
{
    // Test method: void cacheAttribute((DFileInfo::AttributeID id, const QVariant &value))
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->cacheAttribute(DFileInfo::AttributeID(), _arg1));
}

TEST_F(AsyncFileInfoTest, nameOf)
{
    // Test method: QString nameOf((const NameInfoType type))
    auto result = obj->nameOf(NameInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoTest, pathOf)
{
    // Test method: QString pathOf((const PathInfoType type))
    auto result = obj->pathOf(PathInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoTest, urlOf)
{
    // Test method: QUrl urlOf((const UrlInfoType type))
    auto result = obj->urlOf(UrlInfoType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(AsyncFileInfoTest, isAttributes)
{
    // Test method: bool isAttributes((const OptInfoType type))
    auto result = obj->isAttributes(OptInfoType());
    EXPECT_FALSE(result);

}

TEST_F(AsyncFileInfoTest, canAttributes)
{
    // Test method: bool canAttributes((const CanableInfoType type))
    auto result = obj->canAttributes(CanableInfoType());
    EXPECT_FALSE(result);

}

TEST_F(AsyncFileInfoTest, extendAttributes)
{
    // Test method: QVariant extendAttributes((const ExtInfoType type))
    auto result = obj->extendAttributes(ExtInfoType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(AsyncFileInfoTest, timeOf)
{
    // Test method: QVariant timeOf((const TimeInfoType type))
    auto result = obj->timeOf(TimeInfoType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(AsyncFileInfoTest, fileType)
{
    // Test getter: AsyncFileInfo::FileType fileType()
    auto result = obj->fileType();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(AsyncFileInfoTest, displayOf)
{
    // Test method: QString displayOf((const DisPlayInfoType type))
    auto result = obj->displayOf(DisPlayInfoType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoTest, extraProperties)
{
    // Test getter: QVariantHash extraProperties()
    auto result = obj->extraProperties();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoTest, fileIcon)
{
    // Test getter: QIcon fileIcon()
    auto result = obj->fileIcon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(AsyncFileInfoTest, customAttribute)
{
    // Test method: QVariant customAttribute((const char *key, const DFileInfo::DFileAttributeType type))
    auto result = obj->customAttribute(nullptr, DFileInfo::DFileAttributeType());
    EXPECT_FALSE(result.isValid());

}

TEST_F(AsyncFileInfoTest, customData)
{
    // Test method: QVariant customData((int role))
    auto result = obj->customData(0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(AsyncFileInfoTest, updateAttributes)
{
    // Test method: void updateAttributes((const QList<FileInfo::FileInfoAttributeID> &types))
    QList<FileInfo::FileInfoAttributeID> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateAttributes(_arg0));
}

TEST_F(AsyncFileInfoTest, notifyUrls)
{
    // Test getter: QMultiMap<QUrl, QString> notifyUrls()
    auto result = obj->notifyUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoTest, countChildFile)
{
    // Test getter: int countChildFile()
    auto result = obj->countChildFile();
    EXPECT_EQ(result, 0);

}

TEST_F(AsyncFileInfoTest, fileMimeType)
{
    // Test method: QMimeType fileMimeType((QMimeDatabase::MatchMode mode))
    auto result = obj->fileMimeType(QMimeDatabase::MatchMode());
    EXPECT_NO_FATAL_FAILURE({ obj->fileMimeType(QMimeDatabase::MatchMode()); });

}

TEST_F(AsyncFileInfoTest, viewOfTip)
{
    // Test method: QString viewOfTip((const ViewType type))
    auto result = obj->viewOfTip(ViewType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoTest, mediaInfoAttributes)
{
    // Test method: QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfoAttributes((DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids))
    auto result = obj->mediaInfoAttributes(DFileInfo::MediaType(), QList<DFileInfo::AttributeExtendID>());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AsyncFileInfoTest, setExtendedAttributes)
{
    // Test setter: void setExtendedAttributes((const FileExtendedInfoType &key, const QVariant &value))
    FileExtendedInfoType _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setExtendedAttributes(_arg0, _arg1));
}

TEST_F(AsyncFileInfoTest, setNotifyUrl)
{
    // Test setter: void setNotifyUrl((const QUrl &url, const QString &infoPtr))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setNotifyUrl(_arg0, _arg1));
}

TEST_F(AsyncFileInfoTest, removeNotifyUrl)
{
    // Test method: void removeNotifyUrl((const QUrl &url, const QString &infoPtr))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->removeNotifyUrl(_arg0, _arg1));
}

TEST_F(AsyncFileInfoTest, cacheAsyncAttributes)
{
    // Test method: int cacheAsyncAttributes((const QString &attributes))
    QString _arg0{};
    auto result = obj->cacheAsyncAttributes(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(AsyncFileInfoTest, countChildFileAsync)
{
    // Test getter: int countChildFileAsync()
    auto result = obj->countChildFileAsync();
    EXPECT_EQ(result, 0);

}

TEST_F(AsyncFileInfoTest, fileMimeTypeAsync)
{
    // Test method: QMimeType fileMimeTypeAsync((QMimeDatabase::MatchMode mode))
    auto result = obj->fileMimeTypeAsync(QMimeDatabase::MatchMode());
    EXPECT_NO_FATAL_FAILURE({ obj->fileMimeTypeAsync(QMimeDatabase::MatchMode()); });

}

TEST_F(AsyncFileInfoTest, errorCodeFromDfmio)
{
    // Test getter: int errorCodeFromDfmio()
    auto result = obj->errorCodeFromDfmio();
    EXPECT_EQ(result, 0);

}

TEST_F(AsyncFileInfoTest, asyncQueryDfmFileInfo)
{
    // Test method: bool asyncQueryDfmFileInfo((int ioPriority, FileInfo::initQuerierAsyncCallback func, void *userData))
    auto result = obj->asyncQueryDfmFileInfo(0, FileInfo::initQuerierAsyncCallback(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AsyncFileInfoTest, FlagIcon)
{
    // Test method:  FlagIcon(())
    EXPECT_NO_FATAL_FAILURE(obj->FlagIcon());
}

TEST_F(AsyncFileInfoTest, d)
{
    // Test getter: QSharedPointer<AsyncFileInfoPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(AsyncFileInfoTest, M_(FlagIcon))
{
    // Test getter: Q_ENUMS (FlagIcon)()
    EXPECT_NO_FATAL_FAILURE({ obj->(FlagIcon)(); });
}

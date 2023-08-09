// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_DESKTOPFILEINFO
#define UT_DESKTOPFILEINFO

#include <stubext.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/desktopfile.h>
#include <dfm-base/utils/fileutils.h>

#include <QDir>

#include <gtest/gtest.h>
#include <unistd.h>
#include <pwd.h>

DFMBASE_USE_NAMESPACE

class UT_DesktopFileInfo : public testing::Test
{
public:
    virtual void SetUp() override
    {
        // 注册路由
        UrlRoute::regScheme(dfmbase::Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
    }

    virtual void TearDown() override
    {
    }

    ~UT_DesktopFileInfo() override {}
};


TEST_F(UT_DesktopFileInfo, testDesktopFileInfo)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    InfoFactory::instance().constructList.clear();
    DesktopFileInfo info(url);
    QSharedPointer<dfmio::DFileInfo> dfileinfo (new dfmio::DFileInfo(url));
    EXPECT_TRUE(info.proxy.isNull());

    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    DesktopFileInfo info1(url);
    EXPECT_FALSE(info1.proxy.isNull());
}

TEST_F(UT_DesktopFileInfo, testDesktopName)
{
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopDeepinVendor, []{ __DBG_STUB_INVOKE__ return QString("deepin");});
    stub.set_lamda(&DesktopFile::desktopDisplayName, []{ __DBG_STUB_INVOKE__ return QString("deepin");});
    DesktopFileInfo info(url);
    QSharedPointer<dfmio::DFileInfo> dfileinfo (new dfmio::DFileInfo(url));
    stub.set_lamda(VADDR(ProxyFileInfo, refresh), []{ __DBG_STUB_INVOKE__ });
    EXPECT_EQ(QString("deepin"), info.desktopName());

    stub.set_lamda(&DesktopFile::desktopDisplayName, []{ __DBG_STUB_INVOKE__ return QString("");});
    stub.set_lamda(&DesktopFile::desktopLocalName, []{ __DBG_STUB_INVOKE__ return QString("deepin");});
    info.refresh();
    EXPECT_EQ(QString("deepin"), info.desktopName());

    EXPECT_TRUE(info.desktopExec().isEmpty());
    EXPECT_TRUE(info.desktopIconName().isEmpty());
    EXPECT_FALSE(info.desktopType().isEmpty());
    EXPECT_TRUE(info.desktopCategories().isEmpty());

    stub.set_lamda(&DesktopFile::desktopIcon, []{ __DBG_STUB_INVOKE__ return QString("user-trash");});
    stub.set_lamda(&FileUtils::trashIsEmpty, []{ __DBG_STUB_INVOKE__ return false;});
    info.refresh();
    EXPECT_EQ(QString("user-trash-full"), info.desktopIconName());
}

TEST_F(UT_DesktopFileInfo, testDesktopFileInfoNameOf)
{
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    DesktopFileInfo info(url);
    stub.set_lamda(&DesktopFileInfo::desktopName, []{ __DBG_STUB_INVOKE__ return QString("eee");});
    EXPECT_EQ(QString("eee"), info.displayOf(DisPlayInfoType::kFileDisplayName));

    stub.set_lamda(&DesktopFileInfo::desktopName, []{ __DBG_STUB_INVOKE__ return QString("");});
    stub.set_lamda(VADDR(ProxyFileInfo, displayOf), []{ __DBG_STUB_INVOKE__ return QString("eeee");});
    EXPECT_EQ(QString("eeee"), info.displayOf(DisPlayInfoType::kFileDisplayName));

    stub.set_lamda(VADDR(DesktopFileInfo, displayOf), []{ __DBG_STUB_INVOKE__ return QString("display");});
    EXPECT_EQ(QString("display"), info.nameOf(NameInfoType::kFileNameOfRename));
    EXPECT_EQ(QString("display"), info.nameOf(NameInfoType::kBaseNameOfRename));
    EXPECT_TRUE(info.nameOf(NameInfoType::kSuffixOfRename).isEmpty());

    stub.set_lamda(VADDR(ProxyFileInfo, nameOf), []{ __DBG_STUB_INVOKE__ return QString("nameOf");});

    EXPECT_EQ(QString("nameOf"), info.nameOf(NameInfoType::kFileCopyName));
    EXPECT_TRUE(info.nameOf(NameInfoType::kIconName).isEmpty());
    EXPECT_EQ(QString("application-default-icon"), info.nameOf(NameInfoType::kGenericIconName));
    EXPECT_EQ(QString("nameOf"), info.nameOf(NameInfoType::kMimeTypeName));
}

TEST_F(UT_DesktopFileInfo, testDesktopFileInfoSupportedOfAttributes)
{
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopDeepinId, []{ __DBG_STUB_INVOKE__ return QString("dde-computer");});
    DesktopFileInfo info(url);

    EXPECT_FALSE(info.canTag());

    EXPECT_EQ(Qt::IgnoreAction, info.supportedOfAttributes(SupportedType::kDrag));
    stub.set_lamda(VADDR(ProxyFileInfo, supportedOfAttributes), []{ __DBG_STUB_INVOKE__ return Qt::MoveAction;});

    EXPECT_EQ(Qt::MoveAction, info.supportedOfAttributes(SupportedType::kDrop));

    stub.set_lamda(&DesktopFile::desktopDeepinId, []{ __DBG_STUB_INVOKE__ return QString("dde-file-manager");});
    stub.set_lamda(VADDR(ProxyFileInfo, refresh), []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DesktopFile::desktopExec, []{ __DBG_STUB_INVOKE__ return QString("dd -O dd");});
    info.refresh();
    EXPECT_FALSE(info.canTag());

    stub.set_lamda(&DesktopFile::desktopExec, []{ __DBG_STUB_INVOKE__ return QString("dd");});
    info.refresh();
    EXPECT_TRUE(info.canTag());
}

TEST_F(UT_DesktopFileInfo, testDesktopFileInfoCanAttributes)
{
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    stub.set_lamda(&DesktopFile::desktopDeepinId, []{ __DBG_STUB_INVOKE__ return QString("dde-computer");});
    DesktopFileInfo info(url);

    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanMoveOrCopy));
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanDrop));

    stub.set_lamda(&DesktopFile::desktopDeepinId, []{ __DBG_STUB_INVOKE__ return QString("dde-file-manager");});
    stub.set_lamda(VADDR(ProxyFileInfo, refresh), []{ __DBG_STUB_INVOKE__ });
    stub.set_lamda(&DesktopFile::desktopExec, []{ __DBG_STUB_INVOKE__ return QString("dd -O dd");});
    info.refresh();
    EXPECT_FALSE(info.canAttributes(CanableInfoType::kCanMoveOrCopy));

    stub.set_lamda(&DesktopFile::desktopExec, []{ __DBG_STUB_INVOKE__ return QString("dd");});
    info.refresh();
    EXPECT_TRUE(info.canAttributes(CanableInfoType::kCanMoveOrCopy));

    stub.set_lamda(VADDR(ProxyFileInfo, canAttributes), []{ __DBG_STUB_INVOKE__ return true;});
    EXPECT_TRUE(info.canAttributes(CanableInfoType::kCanDrop));
    EXPECT_TRUE(info.canAttributes(CanableInfoType::kCanDragCompress));
}

TEST_F(UT_DesktopFileInfo, testDesktopFileInfoFileIcon)
{
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    stub_ext::StubExt stub;
    DesktopFileInfo info(url);

    stub.set_lamda(&QIcon::isNull, []{ __DBG_STUB_INVOKE__ return false;});
    stub.set_lamda(&QIcon::availableSizes, []{ __DBG_STUB_INVOKE__ QList<QSize> list; list.append(QSize()); return list;});
    EXPECT_TRUE(info.fileIcon().name().isEmpty());

    stub.reset(&QIcon::availableSizes);
    EXPECT_TRUE(info.fileIcon().name().isEmpty());

    stub.clear();
    stub.set_lamda(VADDR(DesktopFileInfo, nameOf), []{ __DBG_STUB_INVOKE__ return "data:image/ooooooo;jjj";});
    typedef int (QString::*IndexOfFunc)(const QString &, int from, Qt::CaseSensitivity) const;
    stub.set_lamda(static_cast<IndexOfFunc>(&QString::indexOf), []{ __DBG_STUB_INVOKE__ return 12;});
    typedef bool(QPixmap::*LoadFromData)(const QByteArray &, const char *, Qt::ImageConversionFlags);
    stub.set_lamda(static_cast<LoadFromData>(&QPixmap::loadFromData), []{ __DBG_STUB_INVOKE__ return true;});
    stub.set_lamda(VADDR(ProxyFileInfo, fileIcon), []{ __DBG_STUB_INVOKE__ return QIcon();});
    EXPECT_TRUE(info.fileIcon().name().isEmpty());

    stub.set_lamda(static_cast<LoadFromData>(&QPixmap::loadFromData), []{ __DBG_STUB_INVOKE__ return false;});
    EXPECT_FALSE(info.fileIcon().name().isEmpty());

    stub.set_lamda(VADDR(DesktopFileInfo, nameOf), []{ __DBG_STUB_INVOKE__ return "~";});
    stub.set_lamda(VADDR(DesktopFileInfo, pathOf), []{ __DBG_STUB_INVOKE__ return QDir::currentPath();});
    EXPECT_FALSE(info.fileIcon().name().isEmpty());
}

#endif

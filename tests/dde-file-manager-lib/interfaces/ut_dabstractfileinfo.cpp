#define protected public
#include "dabstractfileinfo.h"
#undef protected

#include "dfileservices.h"
#include "views/dfileview.h"
#include "dfilesystemmodel.h"

#include <QProcess>
#include <QIcon>
#include <QStandardPaths>

#include <gtest/gtest.h>
#include "stub.h"

namespace  {
class TestDAbstractFileInfo : public testing::Test
{
    virtual void SetUp() override
    {
        std::cout << "start TestDAbstractFileInfo" << std::endl;
        info = new DAbstractFileInfo(DUrl("file:///tmp/1.txt"));
    }

    virtual void TearDown() override
    {
        std::cout << "end TestDAbstractFileInfo" << std::endl;
        if (info) {
            delete info;
            info = nullptr;
        }
    }

public:
    DAbstractFileInfo *info;
};
}

TEST_F(TestDAbstractFileInfo, dateTimeFormat)
{
    EXPECT_TRUE(DAbstractFileInfo::dateTimeFormat() == "yyyy/MM/dd HH:mm:ss");
}

TEST_F(TestDAbstractFileInfo, getFileInfo)
{
    auto fileInfo = DAbstractFileInfo::getFileInfo(DUrl("file:///"));
    EXPECT_TRUE(fileInfo == nullptr);
}

TEST_F(TestDAbstractFileInfo, exists)
{
    EXPECT_FALSE(info->exists());
}

TEST_F(TestDAbstractFileInfo, isPrivate)
{
    EXPECT_FALSE(info->isPrivate());
}

TEST_F(TestDAbstractFileInfo, path)
{
    EXPECT_EQ(info->path(), "/tmp");
}

TEST_F(TestDAbstractFileInfo, filePath)
{
    EXPECT_EQ(info->filePath(), "/tmp/1.txt");
}

TEST_F(TestDAbstractFileInfo, absolutePath)
{
    EXPECT_EQ(info->absolutePath(), "/tmp");
}

TEST_F(TestDAbstractFileInfo, absoluteFilePath)
{
    EXPECT_EQ(info->absoluteFilePath(), "/tmp/1.txt");
}

TEST_F(TestDAbstractFileInfo, baseName)
{
    EXPECT_EQ(info->baseName(), "1");
}

TEST_F(TestDAbstractFileInfo, baseNameOfRename)
{
    EXPECT_EQ(info->baseNameOfRename(), "1");
}

TEST_F(TestDAbstractFileInfo, fileName)
{
    EXPECT_EQ(info->fileName(), "1.txt");
}

TEST_F(TestDAbstractFileInfo, fileNameOfRename)
{
    EXPECT_EQ(info->fileNameOfRename(), "1.txt");
}

TEST_F(TestDAbstractFileInfo, fileDisplayName)
{
    EXPECT_EQ(info->fileDisplayName(), "1.txt");
}

TEST_F(TestDAbstractFileInfo, fileSharedName)
{
    EXPECT_EQ(info->fileSharedName(), "");
}

TEST_F(TestDAbstractFileInfo, fileDisplayPinyinName)
{
    EXPECT_EQ(info->fileDisplayPinyinName(), "1.txt");
}

TEST_F(TestDAbstractFileInfo, canRename)
{
    EXPECT_FALSE(info->canRename());
}

TEST_F(TestDAbstractFileInfo, canShare)
{
    EXPECT_FALSE(info->canShare());
}

TEST_F(TestDAbstractFileInfo, canFetch)
{
    EXPECT_FALSE(info->canFetch());
}

TEST_F(TestDAbstractFileInfo, isReadable)
{
    EXPECT_FALSE(info->isReadable());
}

TEST_F(TestDAbstractFileInfo, isWritable)
{
    EXPECT_FALSE(info->isWritable());
}

TEST_F(TestDAbstractFileInfo, isExecutable)
{
    EXPECT_FALSE(info->isExecutable());
}

TEST_F(TestDAbstractFileInfo, isHidden)
{
    EXPECT_FALSE(info->isHidden());
}

TEST_F(TestDAbstractFileInfo, isRelative)
{
    EXPECT_FALSE(info->isRelative());
}

TEST_F(TestDAbstractFileInfo, isAbsolute)
{
    EXPECT_FALSE(info->isAbsolute());
}

TEST_F(TestDAbstractFileInfo, isShared)
{
    EXPECT_FALSE(info->isShared());
}

TEST_F(TestDAbstractFileInfo, isTaged)
{
    EXPECT_FALSE(info->isTaged());
}

TEST_F(TestDAbstractFileInfo, canTag)
{
    EXPECT_FALSE(info->canTag());
}

TEST_F(TestDAbstractFileInfo, isWritableShared)
{
    EXPECT_FALSE(info->isWritableShared());
}

TEST_F(TestDAbstractFileInfo, isAllowGuestShared)
{
    EXPECT_FALSE(info->isAllowGuestShared());
}

TEST_F(TestDAbstractFileInfo, makeAbsolute)
{
    EXPECT_FALSE(info->makeAbsolute());
}

TEST_F(TestDAbstractFileInfo, canManageAuth)
{
    EXPECT_TRUE(info->canManageAuth());
}

TEST_F(TestDAbstractFileInfo, canMoveOrCopy)
{
    EXPECT_TRUE(info->canMoveOrCopy());
}

TEST_F(TestDAbstractFileInfo, fileType)
{
    EXPECT_EQ(info->fileType(), DAbstractFileInfo::Unknown);
}

TEST_F(TestDAbstractFileInfo, isFile)
{
    EXPECT_FALSE(info->isFile());
}

TEST_F(TestDAbstractFileInfo, isDir)
{
    EXPECT_FALSE(info->isDir());
}

TEST_F(TestDAbstractFileInfo, isSymLink)
{
    EXPECT_FALSE(info->isSymLink());
}

TEST_F(TestDAbstractFileInfo, isDesktopFile)
{
    EXPECT_FALSE(info->isDesktopFile());
}

TEST_F(TestDAbstractFileInfo, isVirtualEntry)
{
    EXPECT_FALSE(info->isVirtualEntry());
}

TEST_F(TestDAbstractFileInfo, symlinkTargetPath)
{
    EXPECT_TRUE(info->symlinkTargetPath().isEmpty());
}

TEST_F(TestDAbstractFileInfo, symLinkTarget)
{
    EXPECT_TRUE(info->symLinkTarget() == DUrl());
}

TEST_F(TestDAbstractFileInfo, rootSymLinkTarget)
{
    QProcess::execute("touch /tmp/2.txt");
    QProcess::execute("ln -s /tmp/2.txt /tmp/link.txt");
    DAbstractFileInfoPointer(*stub_createFileInfo)(const QObject *, const DUrl &) = [](const QObject *, const DUrl &) ->DAbstractFileInfoPointer {
        return DAbstractFileInfoPointer(new DAbstractFileInfo(DUrl("file:///tmp/link.txt")));
    };

    typedef bool (*fptr)();
    fptr DAbstractFileInfo_isSymLink = (fptr)(&DAbstractFileInfo::isSymLink);
    bool (*stub_isSymLink)() = []() {
        static bool flag = true;
        if (flag) {
            flag = !flag;
            return true;
        } else {
            flag = !flag;
            return false;
        }
    };
    Stub stub;
    stub.set(ADDR(DFileService, createFileInfo), stub_createFileInfo);
    stub.set(DAbstractFileInfo_isSymLink, stub_isSymLink);

    DAbstractFileInfo fileInfo(DUrl("file:///tmp/link.txt"));
    auto symLinkTarget = fileInfo.rootSymLinkTarget();
    EXPECT_TRUE(symLinkTarget == DUrl("file:///tmp/link.txt"));
    QProcess::execute("rm -f /tmp/2.txt /tmp/link.txt");
}

TEST_F(TestDAbstractFileInfo, owner)
{
    EXPECT_TRUE(info->owner().isEmpty());
}

TEST_F(TestDAbstractFileInfo, ownerId)
{
    EXPECT_TRUE(info->ownerId() == 0);
}

TEST_F(TestDAbstractFileInfo, group)
{
    EXPECT_TRUE(info->group().isEmpty());
}

TEST_F(TestDAbstractFileInfo, groupId)
{
    EXPECT_TRUE(info->groupId() == 0);
}

TEST_F(TestDAbstractFileInfo, permission)
{
    EXPECT_FALSE(info->permission(QFileDevice::ReadUser));
}

TEST_F(TestDAbstractFileInfo, permissions)
{
    EXPECT_EQ(info->permissions(), QFileDevice::Permissions());
}

TEST_F(TestDAbstractFileInfo, size)
{
    EXPECT_EQ(info->size(), -1);
}

TEST_F(TestDAbstractFileInfo, filesCount)
{
    EXPECT_NO_FATAL_FAILURE(info->filesCount());
}

TEST_F(TestDAbstractFileInfo, fileSize)
{
    EXPECT_EQ(info->fileSize(), -1);
}

TEST_F(TestDAbstractFileInfo, created)
{
    EXPECT_EQ(info->created(), QDateTime());
}

TEST_F(TestDAbstractFileInfo, lastModified)
{
    EXPECT_EQ(info->lastModified(), QDateTime());
}

TEST_F(TestDAbstractFileInfo, lastRead)
{
    EXPECT_EQ(info->lastRead(), QDateTime());
}

TEST_F(TestDAbstractFileInfo, mimeType)
{
    EXPECT_EQ(info->mimeType(), QMimeType());
}

TEST_F(TestDAbstractFileInfo, iconName)
{
    EXPECT_EQ(info->iconName(), "");
}

TEST_F(TestDAbstractFileInfo, genericIconName)
{
    auto iconName = info->genericIconName();
    EXPECT_EQ(iconName, "-x-generic");
}

TEST_F(TestDAbstractFileInfo, lastReadDisplayName)
{
    auto displayName = info->lastReadDisplayName();
    EXPECT_EQ(displayName, "");
}

TEST_F(TestDAbstractFileInfo, lastModifiedDisplayName)
{
    auto displayName = info->lastModifiedDisplayName();
    EXPECT_EQ(displayName, "Unknown");
}

TEST_F(TestDAbstractFileInfo, createdDisplayName)
{
    auto displayName = info->createdDisplayName();
    EXPECT_EQ(displayName, "");
}

TEST_F(TestDAbstractFileInfo, sizeDisplayName)
{
    auto displayName = info->sizeDisplayName();
    EXPECT_EQ(displayName, "0 B");
}

TEST_F(TestDAbstractFileInfo, mimeTypeDisplayName)
{
    auto displayName = info->mimeTypeDisplayName();
    EXPECT_EQ(displayName, "Unknown ()");
}

TEST_F(TestDAbstractFileInfo, fileTypeDisplayName)
{
    auto displayName = info->fileTypeDisplayName();
    EXPECT_EQ(displayName, "14txt");
}

TEST_F(TestDAbstractFileInfo, fileUrl)
{
    EXPECT_EQ(info->fileUrl(), DUrl("file:///tmp/1.txt"));
}

#ifndef __arm__
TEST_F(TestDAbstractFileInfo, fileIcon)
{
    EXPECT_NO_FATAL_FAILURE(info->fileIcon().name());
}
#endif

TEST_F(TestDAbstractFileInfo, additionalIcon)
{
    bool (*stub_isSymLink)() = []() {
        return true;
    };
    bool (*stub_isWritable)() = []() {
        return false;
    };
    bool (*stub_isReadable)() = []() {
        return false;
    };
    bool (*stub_isShared)() = []() {
        return true;
    };

    Stub stub;
    stub.set((bool (*)())ADDR(DAbstractFileInfo, isSymLink), stub_isSymLink);
    stub.set((bool (*)())ADDR(DAbstractFileInfo, isWritable), stub_isWritable);
    stub.set((bool (*)())ADDR(DAbstractFileInfo, isReadable), stub_isReadable);
    stub.set((bool (*)())ADDR(DAbstractFileInfo, isShared), stub_isShared);

    EXPECT_TRUE(!info->additionalIcon().isEmpty());
}

TEST_F(TestDAbstractFileInfo, parentUrl)
{
    EXPECT_TRUE(info->parentUrl() == DUrl("file:///tmp"));
}

TEST_F(TestDAbstractFileInfo, parentUrlList)
{
    EXPECT_TRUE(!info->parentUrlList().isEmpty());
}

TEST_F(TestDAbstractFileInfo, isAncestorsUrl)
{
    DUrlList urlList;
    EXPECT_FALSE(info->isAncestorsUrl(DUrl("file:///tmp/2.txt"), &urlList));
}
#ifndef __arm__
TEST_F(TestDAbstractFileInfo, menuActionList)
{
    // SpaceArea
    {
        auto list = info->menuActionList(DAbstractFileInfo::SpaceArea);
        EXPECT_TRUE(!list.isEmpty());
    }

    // SingleFile
    {
        // isDir and system path
        {
            bool (*stub_isDir)() = []() {
                return true;
            };
            bool (*stub_canShare)() = []() {
                return true;
            };

            Stub stub;
            stub.set((bool (*)())ADDR(DAbstractFileInfo, isDir), stub_isDir);
            stub.set((bool (*)())ADDR(DAbstractFileInfo, canShare), stub_canShare);

            DAbstractFileInfo fileInfo(DUrl::fromLocalFile("/"));
            auto list = fileInfo.menuActionList(DAbstractFileInfo::SingleFile);
            EXPECT_TRUE(!list.isEmpty());
        }

        // isDir
        {
            bool (*stub_isDir)() = []() {
                return true;
            };

            Stub stub;
            stub.set((bool (*)())ADDR(DAbstractFileInfo, isDir), stub_isDir);

            DAbstractFileInfo fileInfo(DUrl::fromLocalFile("/"));
            auto list = fileInfo.menuActionList(DAbstractFileInfo::SingleFile);
            EXPECT_TRUE(!list.isEmpty());
        }

        // other
        {
            DAbstractFileInfo fileInfo(DUrl::fromLocalFile("/"));
            auto list = fileInfo.menuActionList(DAbstractFileInfo::SingleFile);
            EXPECT_TRUE(!list.isEmpty());
        }
    }

    // MultiFiles
    {
        auto list = info->menuActionList(DAbstractFileInfo::MultiFiles);
        EXPECT_TRUE(!list.isEmpty());
    }

    // MultiFilesSystemPathIncluded
    {
        auto list = info->menuActionList(DAbstractFileInfo::MultiFilesSystemPathIncluded);
        EXPECT_TRUE(!list.isEmpty());
    }
}
#endif
TEST_F(TestDAbstractFileInfo, supportViewMode)
{
    EXPECT_TRUE(info->supportViewMode() == DFileView::AllViewMode);
}

TEST_F(TestDAbstractFileInfo, supportSelectionModes)
{
    EXPECT_TRUE(!info->supportSelectionModes().isEmpty());
}

TEST_F(TestDAbstractFileInfo, columnIsCompact)
{
    EXPECT_FALSE(info->columnIsCompact());
}

TEST_F(TestDAbstractFileInfo, userColumnRoles)
{
    // columnCompact is true
    {
        info->setColumnCompact(true);
        EXPECT_FALSE(info->userColumnRoles().isEmpty());
    }

    // columnCompact is false
    {
        info->setColumnCompact(false);
        EXPECT_FALSE(info->userColumnRoles().isEmpty());
    }
}

TEST_F(TestDAbstractFileInfo, userColumnDisplayName)
{
    // DFileSystemModel::FileUserRole + 1
    {
        info->setColumnCompact(true);
        auto displayName = info->userColumnDisplayName(DFileSystemModel::FileUserRole + 1);
        EXPECT_EQ(displayName, "Time modified");
    }

    // other
    {
        info->setColumnCompact(false);
        auto displayName = info->userColumnDisplayName(DFileSystemModel::FileDisplayNameRole);
        EXPECT_EQ(displayName, "Name");
    }
}

TEST_F(TestDAbstractFileInfo, userColumnData)
{
    // FileUserRole + 1
    {
        info->setColumnCompact(true);
        auto data = info->userColumnData(DFileSystemModel::FileUserRole + 1);
    }
    info->setColumnCompact(false);
    // FileLastModifiedRole
    {
        auto data = info->userColumnData(DFileSystemModel::FileLastModifiedRole);
    }

    // FileSizeRole
    {
        auto data = info->userColumnData(DFileSystemModel::FileSizeRole);
    }

    // FileMimeTypeRole
    {
        auto data = info->userColumnData(DFileSystemModel::FileMimeTypeRole);
    }

    // FileCreatedRole
    {
        auto data = info->userColumnData(DFileSystemModel::FileCreatedRole);
    }

    // other
    {
        auto data = info->userColumnData(DFileSystemModel::UnknowRole);
    }
}

TEST_F(TestDAbstractFileInfo, userColumnChildRoles)
{
    // columnCompact is true and col is 1
    {
        info->setColumnCompact(true);
        EXPECT_FALSE(info->userColumnChildRoles(1).isEmpty());
    }

    // other
    {
        info->setColumnCompact(false);
        EXPECT_TRUE(info->userColumnChildRoles(2).isEmpty());
    }
}

TEST_F(TestDAbstractFileInfo, userColumnWidth_1)
{
    EXPECT_TRUE(info->userColumnWidth(DFileSystemModel::FileSizeRole) > 0);
}

TEST_F(TestDAbstractFileInfo, userColumnWidth_2)
{
    QFont font("text");
    QFontMetrics fontMetrics(font);
    // FileNameRole or FileDisplayNameRole
    {
        EXPECT_TRUE(info->userColumnWidth(DFileSystemModel::FileNameRole, fontMetrics) == -1);
    }

    // FileSizeRole
    {
        EXPECT_TRUE(info->userColumnWidth(DFileSystemModel::FileSizeRole, fontMetrics) == 80);
    }

    // FileMimeTypeRole
    {
        EXPECT_TRUE(info->userColumnWidth(DFileSystemModel::FileMimeTypeRole, fontMetrics) == 80);
    }

    // other
    {
        auto width = info->userColumnWidth(DFileSystemModel::FileMimeTypeRole, fontMetrics);
        EXPECT_TRUE(width == 80);
    }
}

TEST_F(TestDAbstractFileInfo, userRowHeight_1)
{
    auto height = info->userRowHeight();
    EXPECT_TRUE(height > 0);
}

TEST_F(TestDAbstractFileInfo, userRowHeight_2)
{
    QFont font("text");
    QFontMetrics fontMetrics(font);

    // columnCompact is true
    {
        info->setColumnCompact(true);
        auto height = info->userRowHeight(fontMetrics);
        EXPECT_TRUE(height > 0);
    }

    // columnCompact is false
    {
        info->setColumnCompact(false);
        auto height = info->userRowHeight(fontMetrics);
        EXPECT_TRUE(height > 0);
    }
}

TEST_F(TestDAbstractFileInfo, columnDefaultVisibleForRole)
{
    // columnCompact is true and role is FileUserRole + 1
    {
        info->setColumnCompact(true);
        auto res = info->columnDefaultVisibleForRole(DFileSystemModel::FileUserRole + 1);
        EXPECT_TRUE(res);
    }

    // other
    {
        info->setColumnCompact(false);
        auto res = info->columnDefaultVisibleForRole(DFileSystemModel::FileCreatedRole);
        EXPECT_FALSE(res);
    }
}

TEST_F(TestDAbstractFileInfo, compareFunByColumn)
{
    // FileDisplayNameRole
    {
        EXPECT_NO_FATAL_FAILURE(info->compareFunByColumn(DFileSystemModel::FileDisplayNameRole));
    }

    // FileLastModifiedRole
    {
        EXPECT_NO_FATAL_FAILURE(info->compareFunByColumn(DFileSystemModel::FileLastModifiedRole));
    }

    // FileSizeRole
    {
        EXPECT_NO_FATAL_FAILURE(info->compareFunByColumn(DFileSystemModel::FileSizeRole));
    }

    // FileMimeTypeRole
    {
        EXPECT_NO_FATAL_FAILURE(info->compareFunByColumn(DFileSystemModel::FileMimeTypeRole));
    }

    // FileCreatedRole
    {
        EXPECT_NO_FATAL_FAILURE(info->compareFunByColumn(DFileSystemModel::FileCreatedRole));
    }

    // FileLastReadRole
    {
        EXPECT_NO_FATAL_FAILURE(info->compareFunByColumn(DFileSystemModel::FileLastReadRole));
    }

    // other
    {
        EXPECT_NO_FATAL_FAILURE(info->compareFunByColumn(DFileSystemModel::UnknowRole));
    }
}

TEST_F(TestDAbstractFileInfo, hasOrderly)
{
    EXPECT_TRUE(info->hasOrderly());
}

TEST_F(TestDAbstractFileInfo, canRedirectionFileUrl)
{
    EXPECT_FALSE(info->canRedirectionFileUrl());
}

TEST_F(TestDAbstractFileInfo, redirectedFileUrl)
{
    EXPECT_TRUE(info->redirectedFileUrl() == DUrl("file:///tmp/1.txt"));
}

TEST_F(TestDAbstractFileInfo, isEmptyFloder)
{
    EXPECT_FALSE(info->isEmptyFloder(QDir::Dirs));
}

TEST_F(TestDAbstractFileInfo, fileItemDisableFlags)
{
    EXPECT_TRUE(info->fileItemDisableFlags() == Qt::ItemFlags());
}

TEST_F(TestDAbstractFileInfo, canIteratorDir)
{
    EXPECT_FALSE(info->canIteratorDir());
}

TEST_F(TestDAbstractFileInfo, getUrlByNewFileName)
{
    EXPECT_TRUE(info->getUrlByNewFileName("2.txt") == DUrl("file:///tmp/2.txt"));
}

TEST_F(TestDAbstractFileInfo, getUrlByChildFileName)
{
    bool (*stub_isDir)() = []() {
        return true;
    };

    Stub stub;
    stub.set((bool (*)())ADDR(DAbstractFileInfo, isDir), stub_isDir);

    DAbstractFileInfo fileInfo(DUrl("file:///tmp"));
    auto url = fileInfo.getUrlByChildFileName("2.txt");
    EXPECT_TRUE(url == DUrl("file:///tmp/2.txt"));
}

TEST_F(TestDAbstractFileInfo, mimeDataUrl)
{
    EXPECT_TRUE(info->mimeDataUrl() == DUrl("file:///tmp/1.txt"));
}

TEST_F(TestDAbstractFileInfo, supportedDragActions)
{
    EXPECT_TRUE(info->supportedDragActions() == (Qt::CopyAction | Qt::MoveAction | Qt::LinkAction));
}

TEST_F(TestDAbstractFileInfo, supportedDropActions)
{
    // isWritable
    {
        bool (*stub_isWritable)() = []() {return true;};

        Stub stub;
        stub.set((bool(*)())ADDR(DAbstractFileInfo, isWritable), stub_isWritable);

        EXPECT_TRUE(info->supportedDropActions() == (Qt::CopyAction | Qt::MoveAction | Qt::LinkAction));
    }

    // canDrop
    {
        bool (*stub_isWritable)() = []() {return false;};
        bool (*stub_canDrop)() = []() {return true;};

        Stub stub;
        stub.set((bool(*)())ADDR(DAbstractFileInfo, isWritable), stub_isWritable);
        stub.set((bool(*)())ADDR(DAbstractFileInfo, canDrop), stub_canDrop);

        EXPECT_TRUE(info->supportedDropActions() == (Qt::CopyAction | Qt::MoveAction));
    }

    // other
    {
        bool (*stub_isWritable)() = []() {return false;};
        bool (*stub_canDrop)() = []() {return false;};

        Stub stub;
        stub.set((bool(*)())ADDR(DAbstractFileInfo, isWritable), stub_isWritable);
        stub.set((bool(*)())ADDR(DAbstractFileInfo, canDrop), stub_canDrop);

        EXPECT_TRUE(info->supportedDropActions() == Qt::IgnoreAction);
    }
}

TEST_F(TestDAbstractFileInfo, loadingTip)
{
    EXPECT_TRUE(info->loadingTip() == "Loading...");
}

TEST_F(TestDAbstractFileInfo, subtitleForEmptyFloder)
{
    EXPECT_TRUE(info->subtitleForEmptyFloder().isEmpty());
}

TEST_F(TestDAbstractFileInfo, suffix)
{
    EXPECT_TRUE(info->suffix() == "txt");
}

TEST_F(TestDAbstractFileInfo, suffixOfRename)
{
    EXPECT_TRUE(info->suffixOfRename() == "txt");
}

TEST_F(TestDAbstractFileInfo, completeSuffix)
{
    auto suffix = info->completeSuffix();
    EXPECT_TRUE(suffix == "txt");
}

TEST_F(TestDAbstractFileInfo, makeToInactive)
{
    EXPECT_NO_FATAL_FAILURE(info->makeToInactive());
}

TEST_F(TestDAbstractFileInfo, goToUrlWhenDeleted)
{
    EXPECT_TRUE(info->goToUrlWhenDeleted() == DUrl("file:///tmp"));
}

TEST_F(TestDAbstractFileInfo, toLocalFile)
{
    EXPECT_TRUE(info->toLocalFile() == "/tmp/1.txt");
}

TEST_F(TestDAbstractFileInfo, canDrop)
{
    EXPECT_FALSE(info->canDrop());
}

TEST_F(TestDAbstractFileInfo, toQFileInfo)
{
    EXPECT_FALSE(info->toQFileInfo() == QFileInfo());
}

TEST_F(TestDAbstractFileInfo, createIODevice)
{
    EXPECT_TRUE(info->createIODevice() == nullptr);
}

TEST_F(TestDAbstractFileInfo, extraProperties)
{
    EXPECT_TRUE(info->extraProperties() == QVariantHash());
}

TEST_F(TestDAbstractFileInfo, checkMpsStr)
{
    EXPECT_FALSE(info->checkMpsStr(""));
}

TEST_F(TestDAbstractFileInfo, getReadTime)
{
    EXPECT_TRUE(info->getReadTime() == QDateTime());
}

TEST_F(TestDAbstractFileInfo, updateReadTime)
{
    EXPECT_NO_FATAL_FAILURE(info->updateReadTime(QDateTime::currentDateTime()));
}

TEST_F(TestDAbstractFileInfo, isGvfsMountFile)
{
    EXPECT_FALSE(info->isGvfsMountFile());
}

TEST_F(TestDAbstractFileInfo, gvfsMountFile)
{
    auto mountFile = info->gvfsMountFile();
    EXPECT_TRUE(mountFile == -1);
}

TEST_F(TestDAbstractFileInfo, checkMountFile)
{
    EXPECT_NO_FATAL_FAILURE(info->checkMountFile());
}

TEST_F(TestDAbstractFileInfo, inode)
{
    EXPECT_TRUE(info->inode() == 0);
}

TEST_F(TestDAbstractFileInfo, makeToActive)
{
    EXPECT_NO_FATAL_FAILURE(info->makeToActive());
}

TEST_F(TestDAbstractFileInfo, isActive)
{
    EXPECT_FALSE(info->isActive());
}

TEST_F(TestDAbstractFileInfo, refresh)
{
    EXPECT_NO_FATAL_FAILURE(info->refresh(true));
}
#ifndef __arm__
TEST_F(TestDAbstractFileInfo, subMenuActionList)
{
    EXPECT_TRUE(!info->subMenuActionList().isEmpty());
}
#endif
TEST_F(TestDAbstractFileInfo, disableMenuActionList)
{
    EXPECT_TRUE(!info->disableMenuActionList().isEmpty());
}

TEST_F(TestDAbstractFileInfo, menuActionByColumnRole)
{
    // FileDisplayNameRole or FileNameRole
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileNameRole) == MenuAction::Name);
    // FileSizeRole
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileSizeRole) == MenuAction::Size);
    // FileMimeTypeRole
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileMimeTypeRole) == MenuAction::Type);
    // FileCreatedRole
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileCreatedRole) == MenuAction::CreatedDate);
    // FileLastModifiedRole
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileLastModifiedRole) == MenuAction::LastModifiedDate);
    // FileLastReadRole
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::FileLastReadRole) == MenuAction::LastRead);
    // UnknowRole
    EXPECT_TRUE(info->menuActionByColumnRole(DFileSystemModel::UnknowRole) == MenuAction::Unknow);
}

TEST_F(TestDAbstractFileInfo, sortSubMenuActionUserColumnRoles)
{
    info->setColumnCompact(true);
    EXPECT_TRUE(!info->sortSubMenuActionUserColumnRoles().isEmpty());
}

TEST_F(TestDAbstractFileInfo, setProxy)
{
    auto fileInfo = DAbstractFileInfoPointer(new DAbstractFileInfo(DUrl("file:///tmp/3.txt")));
    EXPECT_NO_FATAL_FAILURE(info->setProxy(fileInfo));
}

TEST_F(TestDAbstractFileInfo, setUrl)
{
    EXPECT_NO_FATAL_FAILURE(info->setUrl(DUrl("file:///tmp/4.txt")));
}

TEST_F(TestDAbstractFileInfo, loadFileEmblems)
{
    QList<QIcon> iconList;
    EXPECT_FALSE(info->loadFileEmblems(iconList));
}

TEST_F(TestDAbstractFileInfo, parseEmblemString)
{
    QProcess::execute("touch " + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/text.png");
    QString emblemStr("~/text.png;rd");
    QIcon icon;
    QString pos;
    EXPECT_TRUE(info->parseEmblemString(icon, pos, emblemStr));
    QProcess::execute("rm -f " + QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/text.png");
}

TEST_F(TestDAbstractFileInfo, setEmblemIntoIcons)
{
    QList<QIcon> iconList{QIcon(), QIcon(), QIcon(), QIcon()};
    QIcon emblem;
    QString pos;

    // 左下
    {
        pos = "ld";
        info->setEmblemIntoIcons(pos, emblem, iconList);
    }

    // 左上
    {
        pos = "lu";
        info->setEmblemIntoIcons(pos, emblem, iconList);
    }

    //右上
    {
        pos = "ru";
        info->setEmblemIntoIcons(pos, emblem, iconList);
    }
    EXPECT_FALSE(iconList.isEmpty());
}

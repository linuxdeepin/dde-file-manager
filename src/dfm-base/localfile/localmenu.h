/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef LOCALMENU_H
#define LOCALMENU_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QSharedPointer>

DFMBASE_BEGIN_NAMESPACE
class LocalMenuPrivate;
class LocalMenu : public QObject
{
    Q_OBJECT

    enum MenuAction {
        Unknow,
        Open,                               // 打开文件
        OpenDisk,                           // 打开磁盘
        OpenInNewWindow,                    // 在新窗口中打开目录
        OpenInNewTab,                       // 在新标签中打开目录
        OpenDiskInNewWindow,                // 在新窗口中打开磁盘
        OpenDiskInNewTab,                   // 在新标签中打开磁盘
        OpenAsAdmin,                        // 以管理员身份打开
        OpenWith,                           // 使用什么打开
        OpenWithCustom,                     // 用户自定义应用打开
        OpenFileLocation,                   // 打开文件的本地路径
        Compress,                           // 压缩
        Decompress,                         // 解压缩
        DecompressHere,                     // 解压到当前目录
        Cut,                                // 剪切
        Copy,                               // 拷贝
        Paste,                              // 粘贴
        Rename,                             // 重命名
        BookmarkRename,                     // 书签重命名
        BookmarkRemove,                     // 移除书签
        CreateSymlink,                      // 创建系统链接
        SendToDesktop,                      // 发送到桌面
        SendToRemovableDisk,                // 发送到U盘（可移动设备）
        SendToBluetooth,                    // 发送到蓝牙
        AddToBookMark,                      // 加入书签
        Delete,                             // 删除（移动到回收站）
        Property,                           // 属性
        NewFolder,                          // 新建目录
        NewWindow,                          // 新窗口
        SelectAll,                          // 全选
        Separator,                          // menu的中的空格线
        ClearRecent,                        // 清理最近使用
        ClearTrash,                         // 清理回收站
        DisplayAs,                          // sub menu 以什么显示
        SortBy,                             // sub menu 以相应的方式排序
        NewDocument,                        // sub menu 新建word文档
        NewWord,                            // sub menu 新建word文档
        NewExcel,                           // sub menu 新建excel文档
        NewPowerpoint,                      // sub menu 新建ppt文档
        NewText,                            // sub menu 新建文本文档
        OpenInTerminal,                     // 打开bash窗口
        Restore,                            // 从回收站还原
        RestoreAll,                         // 回收站中的文件全部还原
        CompleteDeletion,                   // 侧底删除
        Mount,                              // 挂载
        Unmount,                            // 卸载
        Eject,                              // 移除
        SafelyRemoveDrive,                  // 安全移除
        Name,                               // 名称
        Size,                               // 大小
        Type,                               // 文件的类型
        CreatedDate,                        // 创建日期
        LastModifiedDate,                   // 最后修改时间
        LastRead,                           // 最后
        DeletionDate,                       // 删除时间
        SourcePath,                         // 源文件路径
        AbsolutePath,                       // 文件的路径不包含文件名
        Settings,                           // 设置
        Exit,                               // 退出
        IconView,                           // 图标模式
        ListView,                           // 列表模式
        ExtendView,                         // 扩展模式
        SetAsWallpaper,                     // 保存为壁纸
        ForgetPassword,                     // 忘记密码
        Share,                              // 共享
        UnShare,                            // 取消共享
        SetUserSharePassword,               // 设置共享密码
        FormatDevice,                       // 格式化磁盘
        OpticalBlank,                       // 光驱刻录
        Vault,                              // 保险箱
        ConnectToServer,                    // 连接服务器

        ///###: tag protocol.
        TagInfo,                            // tag的信息
        TagFilesUseColor,                   // tag文件的颜色
        ChangeTagColor,                     // 修改tag文件的颜色
        DeleteTags,                         // 删除设置的tag
        RenameTag,                          // 重命名tag
        RemoveFromRecent,                   // 从最近使用中移除
        MountImage,                         // 挂载镜像文件
        StageFileForBurning,                // 加载需要刻录的文件
        LockNow,                            // 保险箱 现在锁上
        AutoLock,                           // 保险箱 自动锁
        Never,                              // 保险箱 永不上锁
        FiveMinutes,                        // 保险箱 5分钟上锁
        TenMinutes,                         // 保险箱 10分钟上锁
        TwentyMinutes,                      // 保险箱 20分钟上锁
        DeleteVault,                        // 保险箱 删除保险箱
        UnLock,                             // 保险箱 解锁
        UnLockByKey,                        // 保险箱 使用key解锁
        UserMenuAction = Unknow + 2000
    };

    enum MenuType {
        SingleFile,                             // 单个文件
        MultiFiles,                             // 多个文件
        MultiFilesSystemPathIncluded,           // 多个文件包含了系统文件
        SpaceArea                               // 空白区域
    };

    Q_DECLARE_PRIVATE(LocalMenu)
    QSharedPointer<LocalMenuPrivate> d;
public:
    explicit LocalMenu(const QString &filePath);

    virtual QVector<MenuAction> menuActionList(MenuType type) const;
    virtual QMap<MenuAction, QVector<MenuAction>>subMenuActionList(MenuType type = SingleFile) const;
    virtual QSet<MenuAction> disableMenuActionList() const;
    virtual MenuAction menuActionByColumnRole(int role) const;
    virtual QList<int> sortSubMenuActionUserColumnRoles() const;

    bool isAddOemExternalAction();
    void setAddOemExternalAction(bool isAdd);
    void setIsNeedLoadCustomActions(bool needCustom);
    bool isNeedLoadCustomActions();

};
DFMBASE_END_NAMESPACE

#endif // LOCALMENU_H

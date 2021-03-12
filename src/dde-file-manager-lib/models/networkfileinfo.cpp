/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "networkfileinfo.h"

#include "app/define.h"

#include "singleton.h"

#include "controllers/pathmanager.h"

#include "views/dfileview.h"

#include <QIcon>

NetworkFileInfo::NetworkFileInfo(const DUrl &url):
    DAbstractFileInfo(url)
{

}

NetworkFileInfo::~NetworkFileInfo()
{

}

QString NetworkFileInfo::filePath() const
{
    return m_networkNode.url();
}

QString NetworkFileInfo::absoluteFilePath() const
{
    return m_networkNode.url();
}

bool NetworkFileInfo::exists() const
{
    return true;
}

bool NetworkFileInfo::isReadable() const
{
    return true;
}

bool NetworkFileInfo::isWritable() const
{
    return true;
}

bool NetworkFileInfo::isVirtualEntry() const
{
    if (fileUrl() == DUrl(SMB_ROOT) || fileUrl() == DUrl(NETWORK_ROOT))
        return true;

    return DAbstractFileInfo::isVirtualEntry();
}

bool NetworkFileInfo::canDrop() const
{
    if (fileUrl() == DUrl(SMB_ROOT) || fileUrl() == DUrl(NETWORK_ROOT))
        return false;

    return DAbstractFileInfo::canDrop();
}

bool NetworkFileInfo::canRename() const
{
    return false;
}

bool NetworkFileInfo::canIteratorDir() const
{
//    return fileUrl() == DUrl(NETWORK_ROOT);
    return false;
}

//QIcon NetworkFileInfo::fileIcon() const
//{
//    return QIcon(fileIconProvider->getThemeIconPath(mimeTypeName(), 256));
//}

bool NetworkFileInfo::isDir() const
{
    return true;
}

DUrl NetworkFileInfo::parentUrl() const
{
    return DUrl();
}

QString NetworkFileInfo::fileDisplayName() const
{

    if (systemPathManager->isSystemPath(fileUrl().toString()))
        return systemPathManager->getSystemPathDisplayNameByPath(fileUrl().toString());

    return m_networkNode.displayName();
}

//QString NetworkFileInfo::mimeTypeName(QMimeDatabase::MatchMode mode) const
//{
//    Q_UNUSED(mode)

//    return m_networkNode.iconType();
//}
NetworkNode NetworkFileInfo::networkNode() const
{
    return m_networkNode;
}

void NetworkFileInfo::setNetworkNode(const NetworkNode &networkNode)
{
    m_networkNode = networkNode;
}

QVector<MenuAction> NetworkFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;
    if(type == SpaceArea) {

    } else if (type == SingleFile){
        actionKeys << MenuAction::Open;
        actionKeys << MenuAction::OpenInNewWindow;
    }else if (type == MultiFiles){

    }
    return actionKeys;
}

QList<DAbstractFileInfo::SelectionMode> NetworkFileInfo::supportSelectionModes() const
{
    return QList<SelectionMode>() << SingleSelection;
}

Qt::ItemFlags NetworkFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

quint8 NetworkFileInfo::supportViewMode() const
{
    return DFileView::IconMode;
}

int NetworkFileInfo::filesCount() const
{
    return -1;
}

QString NetworkFileInfo::iconName() const
{
    return m_networkNode.iconType();
}

bool NetworkFileInfo::canRedirectionFileUrl() const
{
    if (!mountPoint.isEmpty()) {
        return true;
    }

    const DUrl &url = fileUrl();

    if (url.isNetWorkFile()) {
        return false;
    }

    // for samba file
    GFile *gio_file = g_file_new_for_uri(QFile::encodeName(url.toString()));

    if (!gio_file)
        return false;

    char *local_path = g_file_get_path(gio_file);

    mountPoint = QFile::decodeName(local_path);

    g_free(local_path);
    g_object_unref(gio_file);

    return !mountPoint.isEmpty();
}

DUrl NetworkFileInfo::redirectedFileUrl() const
{
    if  (mountPoint.isEmpty())
        return DUrl();

    return DUrl::fromLocalFile(mountPoint);
}

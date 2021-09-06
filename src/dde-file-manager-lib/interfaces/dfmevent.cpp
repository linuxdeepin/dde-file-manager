/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfmevent.h"
#include "views/windowmanager.h"

#include <QDebug>
#include <QWidget>
#include <QGraphicsWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QJsonArray>
#include <QMetaEnum>

static QString fmeventType2String(DFMEvent::Type type)
{
    switch (type) {
    case DFMEvent::UnknowType:
        return QStringLiteral(QT_STRINGIFY(Unknow));
    case DFMEvent::OpenFile:
        return QStringLiteral(QT_STRINGIFY(OpenFile));
    case DFMEvent::OpenFileByApp:
        return QStringLiteral(QT_STRINGIFY(OpenFileByApp));
    case DFMEvent::CompressFiles:
        return QStringLiteral(QT_STRINGIFY(CompressFiles));
    case DFMEvent::DecompressFile:
        return QStringLiteral(QT_STRINGIFY(DecompressFile));
    case DFMEvent::DecompressFileHere:
        return QStringLiteral(QT_STRINGIFY(DecompressFileHere));
    case DFMEvent::WriteUrlsToClipboard:
        return QStringLiteral(QT_STRINGIFY(WriteUrlsToClipboard));
    case DFMEvent::RenameFile:
        return QStringLiteral(QT_STRINGIFY(RenameFile));
    case DFMEvent::DeleteFiles:
        return QStringLiteral(QT_STRINGIFY(DeleteFiles));
    case DFMEvent::MoveToTrash:
        return QStringLiteral(QT_STRINGIFY(MoveToTrash));
    case DFMEvent::RestoreFromTrash:
        return QStringLiteral(QT_STRINGIFY(RestoreFromTrash));
    case DFMEvent::PasteFile:
        return QStringLiteral(QT_STRINGIFY(PasteFile));
    case DFMEvent::Mkdir:
        return QStringLiteral(QT_STRINGIFY(Mkdir));
    case DFMEvent::TouchFile:
        return QStringLiteral(QT_STRINGIFY(TouchFile));
    case DFMEvent::OpenFileLocation:
        return QStringLiteral(QT_STRINGIFY(OpenFileLocation));
    case DFMEvent::CreateSymlink:
        return QStringLiteral(QT_STRINGIFY(CreateSymlink));
    case DFMEvent::FileShare:
        return QStringLiteral(QT_STRINGIFY(FileShare));
    case DFMEvent::CancelFileShare:
        return QStringLiteral(QT_STRINGIFY(CancelFileShare));
    case DFMEvent::OpenInTerminal:
        return QStringLiteral(QT_STRINGIFY(OpenInTerminal));
    case DFMEvent::GetChildrens:
        return QStringLiteral(QT_STRINGIFY(GetChildrens));
    case DFMEvent::CreateFileInfo:
        return QStringLiteral(QT_STRINGIFY(CreateFileInfo));
    case DFMEvent::CreateDiriterator:
        return QStringLiteral(QT_STRINGIFY(CreateDiriterator));
    case DFMEvent::CreateGetChildrensJob:
        return QStringLiteral(QT_STRINGIFY(CreateGetChildrensJob));
    case DFMEvent::CreateFileWatcher:
        return QStringLiteral(QT_STRINGIFY(CreateFileWatcher));
    case DFMEvent::ChangeCurrentUrl:
        return QStringLiteral(QT_STRINGIFY(ChangeCurrentUrl));
    case DFMEvent::OpenNewWindow:
        return QStringLiteral(QT_STRINGIFY(OpenNewWindow));
    case DFMEvent::OpenUrl:
        return QStringLiteral(QT_STRINGIFY(OpenUrl));
    case DFMEvent::MenuAction:
        return QStringLiteral(QT_STRINGIFY(MenuAction));
    case DFMEvent::SaveOperator:
        return QStringLiteral(QT_STRINGIFY(SaveOperator));
    case DFMEvent::Revocation:
        return QStringLiteral(QT_STRINGIFY(Revocation));
    case DFMEvent::CleanSaveOperator:
        return QStringLiteral(QT_STRINGIFY(CleanSaveOperator));
    case DFMEvent::GetTagsThroughFiles:
        return QStringLiteral(QT_STRINGIFY(GetTagsThroughFiles));
    default:
        return QStringLiteral("Custom: %1").arg(type);
    }
}

DFMEvent::DFMEvent(const QObject *sender)
    : DFMEvent(UnknowType, sender)
{

}

DFMEvent::DFMEvent(DFMEvent::Type type, const QObject *sender)
    : m_type(type)
    , m_sender(sender)
    , m_accept(true)
    , m_id(0)
{

}

DFMEvent::DFMEvent(const DFMEvent &other)
    : m_type(other.m_type)
    , m_data(other.m_data)
    , m_cutData(other.m_cutData)
    , m_properties(other.m_properties)
    , m_sender(other.m_sender)
    , m_accept(other.m_accept)
    , m_id(other.m_id)
{

}

DFMEvent::~DFMEvent()
{

}

DFMEvent &DFMEvent::operator =(const DFMEvent &other)
{
    m_type = other.m_type;
    m_sender = other.m_sender;
    m_accept = other.m_accept;
    m_data = other.m_data;
    m_cutData = other.m_cutData;
    m_properties = other.m_properties;
    m_id = other.m_id;

    return *this;
}

DFMEvent::Type DFMEvent::nameToType(const QString &name)
{
    for (int i = UnknowType; i <= CustomBase; ++i) {
        if (fmeventType2String(static_cast<DFMEvent::Type>(i)) == name) {
            return static_cast<DFMEvent::Type>(i);
        }
    }

    return UnknowType;
}

QString DFMEvent::typeToName(DFMEvent::Type type)
{
    return fmeventType2String(type);
}

quint64 DFMEvent::windowIdByQObject(const QObject *object)
{
    const QObject *obj = object;

    while (obj) {
        const QWidget *w = qobject_cast<const QWidget *>(obj);

        if (w) {
            return WindowManager::getWindowId(w);
        }

        obj = obj->parent();
    }

    const QGraphicsWidget *gw = qobject_cast<const QGraphicsWidget *>(object);

    return (gw && !gw->scene()->views().isEmpty()) ? WindowManager::getWindowId(gw->scene()->views().first()) : 0;
}

quint64 DFMEvent::windowId() const
{
    if (m_id > 0) {
        return m_id;
    }

    if (!m_sender) {
        return 0;
    }

    const QObject *obj = m_sender.data();

    return windowIdByQObject(obj);
}

void DFMEvent::setWindowId(quint64 id)
{
    m_id = id;
}

DUrlList DFMEvent::handleUrlList() const
{
    DUrlList list = qvariant_cast<DUrlList>(m_data);

    if (list.isEmpty()) {
        const DUrl &url = qvariant_cast<DUrl>(m_data);

        if (url.isValid()) {
            list << url;
        }
    }

    return list;
}

const QSharedPointer<DFMEvent> DFMEvent::fromJson(DFMEvent::Type type, const QJsonObject &json)
{
    switch (static_cast<int>(type)) {
    case OpenFile:
        return DFMOpenFileEvent::fromJson(json);
    case OpenFileByApp:
        return DFMOpenFileByAppEvent::fromJson(json);
    case OpenFilesByApp:
        return DFMOpenFilesByAppEvent::fromJson(json);
    case CompressFiles:
        return DFMCompressEvent::fromJson(json);
    case DecompressFile:
        return DFMDecompressEvent::fromJson(json);
    case DecompressFileHere:
        return DFMDecompressHereEvent::fromJson(json);
    case WriteUrlsToClipboard:
        return DFMWriteUrlsToClipboardEvent::fromJson(json);
    case RenameFile:
        return DFMRenameEvent::fromJson(json);
    case DeleteFiles:
        return DFMDeleteEvent::fromJson(json);
    case MoveToTrash:
        return DFMMoveToTrashEvent::fromJson(json);
    case RestoreFromTrash:
        return DFMRestoreFromTrashEvent::fromJson(json);
    case PasteFile:
        return DFMPasteEvent::fromJson(json);
    case Mkdir:
        return DFMMkdirEvent::fromJson(json);
    case TouchFile:
        return DFMTouchFileEvent::fromJson(json);
    case OpenFileLocation:
        return DFMOpenFileLocation::fromJson(json);
    case CreateSymlink:
        return DFMCreateSymlinkEvent::fromJson(json);
    case FileShare:
        return DFMFileShareEvent::fromJson(json);
    case CancelFileShare:
        return DFMCancelFileShareEvent::fromJson(json);
    case OpenInTerminal:
        return DFMOpenInTerminalEvent::fromJson(json);
    case GetChildrens:
        return DFMGetChildrensEvent::fromJson(json);
    case CreateFileInfo:
        return DFMCreateFileInfoEvent::fromJson(json);
    case CreateDiriterator:
        return DFMCreateDiriterator::fromJson(json);
    case CreateGetChildrensJob:
        return DFMCreateGetChildrensJob::fromJson(json);
    case CreateFileWatcher:
        return DFMCreateFileWatcherEvent::fromJson(json);
    case ChangeCurrentUrl:
        return DFMChangeCurrentUrlEvent::fromJson(json);
    case OpenNewWindow:
        return DFMOpenNewWindowEvent::fromJson(json);
    case OpenUrl:
        return DFMOpenUrlEvent::fromJson(json);
    case MenuAction:
        return DFMMenuActionEvent::fromJson(json);
    case Back:
        return DFMBackEvent::fromJson(json);
    case Forward:
        return DFMForwardEvent::fromJson(json);
    default: break;
    }

    return QSharedPointer<DFMEvent>();
}

const QSharedPointer<DFMEvent> DFMEvent::fromJson(const QJsonObject &json)
{
    return fromJson(nameToType(json["eventType"].toString()), json);
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info)
{
    deg << "type:" << fmeventType2String(info.type()) << "sender:" << info.sender();
    deg << "data:" << info.data();
    deg << "properties:" << info.properties();

    return deg;
}
QT_END_NAMESPACE

DFMUrlBaseEvent::DFMUrlBaseEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(DFMEvent::UnknowType, sender, url)
{

}

DFMUrlBaseEvent::DFMUrlBaseEvent(DFMEvent::Type type, const QObject *sender, const DUrl &url)
    : DFMEvent(type, sender)
{
    setData(url);
}

QSharedPointer<DFMUrlBaseEvent> DFMUrlBaseEvent::fromJson(Type type, const QJsonObject &json)
{
    return dMakeEventPointer<DFMUrlBaseEvent>(type, Q_NULLPTR, DUrl::fromUserInput(json["url"].toString()));
}

DFMUrlListBaseEvent::DFMUrlListBaseEvent()
    : DFMUrlListBaseEvent(nullptr, DUrlList())
{

}

DFMUrlListBaseEvent::DFMUrlListBaseEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(DFMEvent::UnknowType, sender, list)
{

}

DFMUrlListBaseEvent::DFMUrlListBaseEvent(DFMEvent::Type type, const QObject *sender, const DUrlList &list)
    : DFMEvent(type, sender)
{
    //从数据盘删除文件到回收站实际是同一目录下的操作
    //处理数据盘下的url 使其与回收站路径开头相同 保证后续删除还原逻辑判断正确
    if (type == MoveToTrash) {
        DUrlList urls;
        //遍历传入的urllist
        for (DUrl url : list) {
            //判断是否是来自数据盘路径
            //fix bug63348 在最近使用文件夹下无法删除/data目录下的文件，因为在删除的时候把/data/home处理成/home
            if (url.path().startsWith("/data/home/") && !url.isRecentFile()) {
                //去掉数据盘路径开头
                url.setPath(url.path().mid(sizeof("/data") - 1));
            }
            //将处理后的url添加到临时list
            urls.append(url);
        }
        //设置事件的url列表
        setData(urls);
    }
    //暂时只处理movetotrash事件
    else {
        setData(list);
    }
}

QSharedPointer<DFMUrlListBaseEvent> DFMUrlListBaseEvent::fromJson(Type type, const QJsonObject &json)
{
    DUrlList list;

    for (const QJsonValue &value : json["urlList"].toArray()) {
        list << DUrl::fromUserInput(value.toString());
    }

    return dMakeEventPointer<DFMUrlListBaseEvent>(type, Q_NULLPTR, list);
}

DFMOpenFileEvent::DFMOpenFileEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenFile, sender, url)
{

}

QSharedPointer<DFMOpenFileEvent> DFMOpenFileEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(OpenFile, json).staticCast<DFMOpenFileEvent>();
}

DFMOpenFileByAppEvent::DFMOpenFileByAppEvent(const QObject *sender, const QString &appName, const DUrl &url)
    : DFMOpenFileEvent(sender, url)
{
    m_type = OpenFileByApp;
    setProperty(QT_STRINGIFY(DFMOpenFileByAppEvent::appName), appName);
}

QString DFMOpenFileByAppEvent::appName() const
{
    return property(QT_STRINGIFY(DFMOpenFileByAppEvent::appName), QString());
}

QSharedPointer<DFMOpenFileByAppEvent> DFMOpenFileByAppEvent::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMOpenFileByAppEvent>(Q_NULLPTR, json["appName"].toString(), DUrl::fromUserInput(json["url"].toString()));
}

DFMOpenFilesByAppEvent::DFMOpenFilesByAppEvent(const QObject *sender, const QString &appName, const QList<DUrl> &url, const bool isEnter)
    : DFMOpenFilesEvent(sender, url, isEnter)
{
    m_type = OpenFilesByApp;
    setProperty(QT_STRINGIFY(DFMOpenFilesByAppEvent::appName), appName);
}

QString DFMOpenFilesByAppEvent::appName() const
{
    return property(QT_STRINGIFY(DFMOpenFilesByAppEvent::appName), QString());
}

QSharedPointer<DFMOpenFilesByAppEvent> DFMOpenFilesByAppEvent::fromJson(const QJsonObject &json)
{
    DUrlList list;

    for (const QJsonValue &value : json["urlList"].toArray()) {
        list << DUrl::fromUserInput(value.toString());
    }

    return dMakeEventPointer<DFMOpenFilesByAppEvent>(Q_NULLPTR, json["appName"].toString(), list);
}

DFMCompressEvent::DFMCompressEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(CompressFiles, sender, list)
{

}

QSharedPointer<DFMCompressEvent> DFMCompressEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(CompressFiles, json).staticCast<DFMCompressEvent>();
}

DFMDecompressEvent::DFMDecompressEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(DecompressFile, sender, list)
{

}

QSharedPointer<DFMDecompressEvent> DFMDecompressEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(DecompressFile, json).staticCast<DFMDecompressEvent>();
}

DFMDecompressHereEvent::DFMDecompressHereEvent(const QObject *sender, const DUrlList &list)
    : DFMDecompressEvent(sender, list)
{
    m_type = DecompressFileHere;
}

QSharedPointer<DFMDecompressHereEvent> DFMDecompressHereEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(DecompressFileHere, json).staticCast<DFMDecompressHereEvent>();
}

DFMWriteUrlsToClipboardEvent::DFMWriteUrlsToClipboardEvent(const QObject *sender,
                                                           DFMGlobal::ClipboardAction action,
                                                           const DUrlList &list)
    : DFMUrlListBaseEvent(WriteUrlsToClipboard, sender, list)
{
    setProperty(QT_STRINGIFY(DFMWriteUrlsToClipboardEvent::action), action);
}

DFMGlobal::ClipboardAction DFMWriteUrlsToClipboardEvent::action() const
{
    return property(QT_STRINGIFY(DFMWriteUrlsToClipboardEvent::action), DFMGlobal::CutAction);
}

QSharedPointer<DFMWriteUrlsToClipboardEvent> DFMWriteUrlsToClipboardEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event = DFMUrlListBaseEvent::fromJson(WriteUrlsToClipboard, json).staticCast<DFMWriteUrlsToClipboardEvent>();

    event->setProperty(QT_STRINGIFY(DFMWriteUrlsToClipboardEvent::action), (DFMGlobal::ClipboardAction)json["action"].toInt());

    return event;
}

DFMRenameEvent::DFMRenameEvent(const QObject *sender, const DUrl &from, const DUrl &to, const bool silent)
    : DFMEvent(RenameFile, sender)
{
    setData(QPair<DUrl, DUrl>(from, to));
    setProperty(QT_STRINGIFY(DFMRenameEvent::silent), silent);
}

DUrlList DFMRenameEvent::handleUrlList() const
{
    const auto &&d = qvariant_cast<QPair<DUrl, DUrl>>(m_data);

    return DUrlList() << d.first << d.second;
}

bool DFMRenameEvent::silent() const
{
    return property(QT_STRINGIFY(DFMRenameEvent::silent), false);
}

QSharedPointer<DFMRenameEvent> DFMRenameEvent::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMRenameEvent>(Q_NULLPTR, DUrl::fromUserInput(json["from"].toString()), DUrl::fromUserInput(json["to"].toString()));
}

DFMDeleteEvent::DFMDeleteEvent(const QObject *sender, const DUrlList &list, bool silent, bool force)
    : DFMUrlListBaseEvent(DeleteFiles, sender, list)
{
    setProperty(QT_STRINGIFY(DFMDeleteEvent::silent), silent);
    setProperty(QT_STRINGIFY(DFMDeleteEvent::force), force);
}

bool DFMDeleteEvent::silent() const
{
    return property(QT_STRINGIFY(DFMDeleteEvent::silent), false);
}

bool DFMDeleteEvent::force() const
{
    return property(QT_STRINGIFY(DFMDeleteEvent::force), false);
}

QSharedPointer<DFMDeleteEvent> DFMDeleteEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMDeleteEvent> &event = DFMUrlListBaseEvent::fromJson(DeleteFiles, json).staticCast<DFMDeleteEvent>();

    event->setProperty(QT_STRINGIFY(DFMDeleteEvent::silent), json["silent"].toBool());
    event->setProperty(QT_STRINGIFY(DFMDeleteEvent::force), json["force"].toBool());

    return event;
}

DFMMoveToTrashEvent::DFMMoveToTrashEvent(const QObject *sender, const DUrlList &list, bool silent, const DUrlList &cutList)
    : DFMUrlListBaseEvent(MoveToTrash, sender, list)
{
    setProperty(QT_STRINGIFY(DFMMoveToTrashEvent::silent), silent);
    setCutData(cutList);
}

bool DFMMoveToTrashEvent::silent() const
{
    return property(QT_STRINGIFY(DFMMoveToTrashEvent::silent), false);
}

QSharedPointer<DFMMoveToTrashEvent> DFMMoveToTrashEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(MoveToTrash, json).staticCast<DFMMoveToTrashEvent>();
}

DFMRestoreFromTrashEvent::DFMRestoreFromTrashEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(RestoreFromTrash, sender, list)
{

}

QSharedPointer<DFMRestoreFromTrashEvent> DFMRestoreFromTrashEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(RestoreFromTrash, json).staticCast<DFMRestoreFromTrashEvent>();
}

DFMPasteEvent::DFMPasteEvent(const QObject *sender, DFMGlobal::ClipboardAction action,
                             const DUrl &targetUrl, const DUrlList &list)
    : DFMUrlListBaseEvent(PasteFile, sender, list)
{
    setData(list);
    setProperty(QT_STRINGIFY(DFMPasteEvent::action), action);
    setProperty(QT_STRINGIFY(DFMPasteEvent::targetUrl), targetUrl);
}

DFMGlobal::ClipboardAction DFMPasteEvent::action() const
{
    return property(QT_STRINGIFY(DFMPasteEvent::action), DFMGlobal::CutAction);
}

DUrl DFMPasteEvent::targetUrl() const
{
    return property(QT_STRINGIFY(DFMPasteEvent::targetUrl), DUrl());
}

DUrlList DFMPasteEvent::handleUrlList() const
{
    return DUrlList() << targetUrl() << urlList();
}

QSharedPointer<DFMPasteEvent> DFMPasteEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMUrlListBaseEvent> &event = DFMUrlListBaseEvent::fromJson(PasteFile, json);

    return dMakeEventPointer<DFMPasteEvent>(Q_NULLPTR, (DFMGlobal::ClipboardAction)json["action"].toInt(),
                                            DUrl::fromUserInput(json["targetUrl"].toString()), event->urlList());
}

DFMMkdirEvent::DFMMkdirEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(Mkdir, sender, url)
{

}

QSharedPointer<DFMMkdirEvent> DFMMkdirEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(Mkdir, json).staticCast<DFMMkdirEvent>();
}

DFMTouchFileEvent::DFMTouchFileEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(TouchFile, sender, url)
{

}

QSharedPointer<DFMTouchFileEvent> DFMTouchFileEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(TouchFile, json).staticCast<DFMTouchFileEvent>();
}

DFMOpenFileLocation::DFMOpenFileLocation(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenFileLocation, sender, url)
{

}

QSharedPointer<DFMOpenFileLocation> DFMOpenFileLocation::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(OpenFileLocation, json).staticCast<DFMOpenFileLocation>();
}

DFMAddToBookmarkEvent::DFMAddToBookmarkEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(AddToBookmark, sender, url)
{

}

DFMRemoveBookmarkEvent::DFMRemoveBookmarkEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(RemoveBookmark, sender, url)
{

}

DFMCreateSymlinkEvent::DFMCreateSymlinkEvent(const QObject *sender, const DUrl &fileUrl, const DUrl &toUrl, bool force/* = false*/)
    : DFMEvent(CreateSymlink, sender)
{
    setData(makeData(fileUrl, toUrl));
    setProperty(QT_STRINGIFY(DFMDeleteEvent::force), force);
}

bool DFMCreateSymlinkEvent::force() const
{
    return property(QT_STRINGIFY(DFMDeleteEvent::force), false);
}

DUrlList DFMCreateSymlinkEvent::handleUrlList() const
{
    return DUrlList() << fileUrl() << toUrl();
}

QSharedPointer<DFMCreateSymlinkEvent> DFMCreateSymlinkEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMCreateSymlinkEvent> &event = dMakeEventPointer<DFMCreateSymlinkEvent>(Q_NULLPTR, DUrl::fromUserInput(json["fileUrl"].toString()),
                                                                                                  DUrl::fromUserInput(json["toUrl"].toString()));
    event->setProperty(QT_STRINGIFY(DFMDeleteEvent::force), json["force"].toBool());
    return event;
}

DFMFileShareEvent::DFMFileShareEvent(const QObject *sender, const DUrl &url,
                                     const QString &name, bool isWritable, bool allowGuest)
    : DFMUrlBaseEvent(FileShare, sender, url)
{
    setProperty(QT_STRINGIFY(DFMFileShareEvent::name), name);
    setProperty(QT_STRINGIFY(DFMFileShareEvent::isWritable), isWritable);
    setProperty(QT_STRINGIFY(DFMFileShareEvent::allowGuest), allowGuest);
}

QString DFMFileShareEvent::name() const
{
    return property(QT_STRINGIFY(DFMFileShareEvent::name), QString());
}

bool DFMFileShareEvent::isWritable() const
{
    return property(QT_STRINGIFY(DFMFileShareEvent::isWritable), false);
}

bool DFMFileShareEvent::allowGuest() const
{
    return property(QT_STRINGIFY(DFMFileShareEvent::allowGuest), false);
}

QSharedPointer<DFMFileShareEvent> DFMFileShareEvent::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMFileShareEvent>(Q_NULLPTR, DUrl::fromUserInput(json["url"].toString()), json["name"].toString(),
                                                json["isWritable"].toBool(), json["allowGuest"].toBool());
}

DFMCancelFileShareEvent::DFMCancelFileShareEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(CancelFileShare, sender, url)
{

}

QSharedPointer<DFMCancelFileShareEvent> DFMCancelFileShareEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(CancelFileShare, json).staticCast<DFMCancelFileShareEvent>();
}

DFMOpenInTerminalEvent::DFMOpenInTerminalEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenInTerminal, sender, url)
{

}

QSharedPointer<DFMOpenInTerminalEvent> DFMOpenInTerminalEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(OpenInTerminal, json).staticCast<DFMOpenInTerminalEvent>();
}

DFMGetChildrensEvent::DFMGetChildrensEvent(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags, bool slient, bool canconst)
    : DFMUrlBaseEvent(GetChildrens, sender, url)
{
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::nameFilters), nameFilters);
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::filters), filters);
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::flags), flags);
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::slient), slient);
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::canconst), canconst);
}

DFMGetChildrensEvent::DFMGetChildrensEvent(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                           QDir::Filters filters, bool slient, bool canconst)
    : DFMGetChildrensEvent(sender, url, nameFilters, filters, QDirIterator::NoIteratorFlags, slient, canconst)
{

}

QStringList DFMGetChildrensEvent::nameFilters() const
{
    return property(QT_STRINGIFY(DFMGetChildrensEvent::nameFilters), QStringList());
}

QDir::Filters DFMGetChildrensEvent::filters() const
{
    return property(QT_STRINGIFY(DFMGetChildrensEvent::filters), QDir::Filters());
}

QDirIterator::IteratorFlags DFMGetChildrensEvent::flags() const
{
    return property(QT_STRINGIFY(DFMGetChildrensEvent::flags), QDirIterator::IteratorFlags());
}

bool DFMGetChildrensEvent::silent() const
{
    return property(QT_STRINGIFY(DFMGetChildrensEvent::silent), false);
}

bool DFMGetChildrensEvent::canconst() const
{
    return property(QT_STRINGIFY(DFMGetChildrensEvent::canconst), false);
}

QSharedPointer<DFMGetChildrensEvent> DFMGetChildrensEvent::fromJson(const QJsonObject &json)
{
    QStringList nameFilters;

    for (const QJsonValue &value : json["nameFilters"].toArray()) {
        nameFilters << value.toString();
    }

    return dMakeEventPointer<DFMGetChildrensEvent>(Q_NULLPTR, DUrl::fromUserInput(json["url"].toString()),
                                                   nameFilters, (QDir::Filters)json["filters"].toInt(), QDirIterator::NoIteratorFlags,
                                                   json["silent"].toBool());
}

DFMCreateDiriterator::DFMCreateDiriterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent, bool isgvfs)
    : DFMGetChildrensEvent(sender, fileUrl, nameFilters, filters, flags, silent)
{
    m_type = CreateDiriterator;
    setProperty(QT_STRINGIFY(DFMCreateDiriterator::isgvfsfile), isgvfs);
}

DFMCreateDiriterator::DFMCreateDiriterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, bool silent, bool isgvfs)
    : DFMGetChildrensEvent(sender, fileUrl, nameFilters, filters, silent)
{
    m_type = CreateDiriterator;
    setProperty(QT_STRINGIFY(DFMCreateDiriterator::isgvfsfile), isgvfs);
}

bool DFMCreateDiriterator::isGvfsFile() const
{
    return property<bool>(QT_STRINGIFY(DFMCreateDiriterator::isgvfsfile));
}

QSharedPointer<DFMCreateDiriterator> DFMCreateDiriterator::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMCreateDiriterator> &event = DFMGetChildrensEvent::fromJson(json).staticCast<DFMCreateDiriterator>();

    event->m_type = CreateDiriterator;

    return event;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                                   QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent, const bool isgvfsfile)
    : DFMCreateDiriterator(sender, url, nameFilters, filters, flags, silent, isgvfsfile)
{
    m_type = CreateGetChildrensJob;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                                   QDir::Filters filters, bool silent, const bool isgvfsfile)
    : DFMCreateDiriterator(sender, url, nameFilters, filters, silent, isgvfsfile)
{
    m_type = CreateGetChildrensJob;
}

QSharedPointer<DFMCreateGetChildrensJob> DFMCreateGetChildrensJob::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMCreateGetChildrensJob> &event = DFMCreateDiriterator::fromJson(json).staticCast<DFMCreateGetChildrensJob>();

    event->m_type = CreateGetChildrensJob;

    return event;
}

DFMCreateFileInfoEvent::DFMCreateFileInfoEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(CreateFileInfo, sender, url)
{

}

QSharedPointer<DFMCreateFileInfoEvent> DFMCreateFileInfoEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(CreateFileInfo, json).staticCast<DFMCreateFileInfoEvent>();
}

DFMCreateFileWatcherEvent::DFMCreateFileWatcherEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(CreateFileWatcher, sender, url)
{

}

QSharedPointer<DFMCreateFileWatcherEvent> DFMCreateFileWatcherEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(CreateFileWatcher, json).staticCast<DFMCreateFileWatcherEvent>();
}

DFMChangeCurrentUrlEvent::DFMChangeCurrentUrlEvent(const QObject *sender, const DUrl &url, const QWidget *window)
    : DFMUrlBaseEvent(ChangeCurrentUrl, sender, url)
{
    setProperty(QT_STRINGIFY(DFMChangeCurrentUrlEvent::window), (quintptr)window);
}

const QWidget *DFMChangeCurrentUrlEvent::window() const
{
    return (const QWidget *)(property<quintptr>(QT_STRINGIFY(DFMChangeCurrentUrlEvent::window)));
}

QSharedPointer<DFMChangeCurrentUrlEvent> DFMChangeCurrentUrlEvent::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMChangeCurrentUrlEvent>(Q_NULLPTR, DUrl::fromUserInput(json["url"].toString()), Q_NULLPTR);
}

DFMOpenNewWindowEvent::DFMOpenNewWindowEvent(const QObject *sender, const DUrlList &list, bool force)
    : DFMUrlListBaseEvent(OpenNewWindow, sender, list)
{
    setProperty(QT_STRINGIFY(DFMOpenNewWindowEvent::force), force);
}

bool DFMOpenNewWindowEvent::force() const
{
    return property(QT_STRINGIFY(DFMOpenNewWindowEvent::force), false);
}

QSharedPointer<DFMOpenNewWindowEvent> DFMOpenNewWindowEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMOpenNewWindowEvent> &event = DFMUrlListBaseEvent::fromJson(OpenNewWindow, json).staticCast<DFMOpenNewWindowEvent>();

    event->setProperty(QT_STRINGIFY(DFMOpenNewWindowEvent::force), json["force"].toBool());

    return event;
}

DFMOpenNewTabEvent::DFMOpenNewTabEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenNewTab, sender, url)
{

}

QSharedPointer<DFMOpenNewTabEvent> DFMOpenNewTabEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(OpenNewTab, json).staticCast<DFMOpenNewTabEvent>();
}

DFMOpenUrlEvent::DFMOpenUrlEvent(const QObject *sender, const DUrlList &list, DFMOpenUrlEvent::DirOpenMode mode, const bool isEnter)
    : DFMUrlListBaseEvent(OpenUrl, sender, list)
{
    setProperty(QT_STRINGIFY(DFMOpenUrlEvent::dirOpenMode), mode);
    setProperty(QT_STRINGIFY(DFMOpenUrlEvent::isEnter), isEnter);
}

DFMOpenUrlEvent::DirOpenMode DFMOpenUrlEvent::dirOpenMode() const
{
    return property(QT_STRINGIFY(DFMOpenUrlEvent::dirOpenMode), DirOpenMode::OpenNewWindow);
}

bool DFMOpenUrlEvent::isEnter() const
{
    return property(QT_STRINGIFY(DFMOpenUrlEvent::isEnter)).toBool();
}

QSharedPointer<DFMOpenUrlEvent> DFMOpenUrlEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMOpenUrlEvent> &event = DFMUrlListBaseEvent::fromJson(OpenUrl, json).staticCast<DFMOpenUrlEvent>();

    event->setProperty(QT_STRINGIFY(DFMOpenUrlEvent::dirOpenMode), static_cast<DFMOpenUrlEvent::DirOpenMode>(json["mode"].toInt()));
    event->setProperty(QT_STRINGIFY(DFMOpenUrlEvent::isEnter), json["isEnter"].toBool());

    return event;
}

DFMMenuActionEvent::DFMMenuActionEvent(const QObject *sender, const DFileMenu *menu, const DUrl &currentUrl,
                                       const DUrlList &selectedUrls, DFMGlobal::MenuAction action, const QModelIndex &index)
    : DFMUrlListBaseEvent(MenuAction, sender, selectedUrls)
{
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::menu), (quintptr)menu);
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::currentUrl), currentUrl);
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::action), action);
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::clickedIndex), index);
}

const DFileMenu *DFMMenuActionEvent::menu() const
{
    return (const DFileMenu *)(property<quintptr>(QT_STRINGIFY(DFMMenuActionEvent::menu), 0));
}

const DUrl DFMMenuActionEvent::currentUrl() const
{
    return property(QT_STRINGIFY(DFMMenuActionEvent::currentUrl), DUrl());
}

const DUrlList DFMMenuActionEvent::selectedUrls() const
{
    return data<DUrlList>();
}

DFMGlobal::MenuAction DFMMenuActionEvent::action() const
{
    return property(QT_STRINGIFY(DFMMenuActionEvent::action), DFMGlobal::Unknow);
}

const QModelIndex DFMMenuActionEvent::clickedIndex() const
{
    return property(QT_STRINGIFY(DFMMenuActionEvent::clickedIndex), QModelIndex());
}

const QList<QColor> DFMMenuActionEvent::tagColors() const
{
    return m_tagColorsList;
}

void DFMMenuActionEvent::setTagColors(const QList<QColor> &list)
{
    m_tagColorsList = list;
}

QSharedPointer<DFMMenuActionEvent> DFMMenuActionEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMMenuActionEvent> &event = DFMUrlListBaseEvent::fromJson(MenuAction, json).staticCast<DFMMenuActionEvent>();
    int action = DFMGlobal::instance()->metaObject()->enumerator(DFMGlobal::instance()->metaObject()->indexOfEnumerator("MenuAction")).keyToValue(json["action"].toString().toLocal8Bit().constData());

    event->setProperty(QT_STRINGIFY(DFMMenuActionEvent::currentUrl), DUrl::fromUserInput(json["currentUrl"].toString()));
    event->setProperty(QT_STRINGIFY(DFMMenuActionEvent::action), static_cast<DFMGlobal::MenuAction>(action));

    return event;
}

DFMBackEvent::DFMBackEvent(const QObject *sender)
    : DFMEvent(Back, sender)
{

}

QSharedPointer<DFMBackEvent> DFMBackEvent::fromJson(const QJsonObject &json)
{
    Q_UNUSED(json)

    return dMakeEventPointer<DFMBackEvent>(Q_NULLPTR);
}

DFMForwardEvent::DFMForwardEvent(const QObject *sender)
    : DFMEvent(Forward, sender)
{

}

QSharedPointer<DFMForwardEvent> DFMForwardEvent::fromJson(const QJsonObject &json)
{
    Q_UNUSED(json)

    return dMakeEventPointer<DFMForwardEvent>(Q_NULLPTR);
}

DFMSaveOperatorEvent::DFMSaveOperatorEvent(const QSharedPointer<DFMEvent> &iniaiator, const QSharedPointer<DFMEvent> &event, bool async)
    : DFMEvent(SaveOperator, nullptr)
{
    setProperty(QT_STRINGIFY(DFMSaveOperatorEvent::iniaiator), QVariant::fromValue(iniaiator));
    setProperty(QT_STRINGIFY(DFMSaveOperatorEvent::event), QVariant::fromValue(event));
    setProperty(QT_STRINGIFY(DFMSaveOperatorEvent::async), async);
}

DFMSaveOperatorEvent::DFMSaveOperatorEvent()
    : DFMEvent(SaveOperator, nullptr)
{
    setProperty(QT_STRINGIFY(DFMSaveOperatorEvent::split), true);
}

QSharedPointer<DFMEvent> DFMSaveOperatorEvent::iniaiator() const
{
    return qvariant_cast<QSharedPointer<DFMEvent>>(property(QT_STRINGIFY(DFMSaveOperatorEvent::iniaiator)));
}

QSharedPointer<DFMEvent> DFMSaveOperatorEvent::event() const
{
    return qvariant_cast<QSharedPointer<DFMEvent>>(property(QT_STRINGIFY(DFMSaveOperatorEvent::event)));
}

bool DFMSaveOperatorEvent::async() const
{
    return property(QT_STRINGIFY(DFMSaveOperatorEvent::async)).toBool();
}

bool DFMSaveOperatorEvent::split() const
{
    return property(QT_STRINGIFY(DFMSaveOperatorEvent::split)).toBool();
}

DFMRevocationEvent::DFMRevocationEvent(const QObject *sender)
    : DFMEvent(Revocation, sender)
{

}

DFMCleanSaveOperatorEvent::DFMCleanSaveOperatorEvent(const QObject *sender)
    : DFMEvent(CleanSaveOperator, sender)
{

}

DFMSetFileTagsEvent::DFMSetFileTagsEvent(const QObject *sender, const DUrl &url, const QList<QString> &tags)
    : DFMUrlBaseEvent{Tag, sender, url }
{
    setProperty(QT_STRINGIFY(DFMSetFileTagsEvent::tags), tags);
}

QSharedPointer<DFMSetFileTagsEvent> DFMSetFileTagsEvent::fromJson(const QJsonObject &json)
{
    (void)json;
    return QSharedPointer<DFMSetFileTagsEvent> { nullptr };
}

QList<QString> DFMSetFileTagsEvent::tags() const
{
    QVariant var{ property(QT_STRINGIFY(DFMSetFileTagsEvent::tags), {QString()}) };
    QList<QString> tag_name{ var.value<QList<QString>>() };

    return tag_name;
}

DFMRemoveTagsOfFileEvent::DFMRemoveTagsOfFileEvent(const QObject *sender, const DUrl &url, const QList<QString> &tags)
    : DFMUrlBaseEvent{DFMEvent::Untag, sender, url}
{
    setProperty(QT_STRINGIFY(DFMRemoveTagsOfFileEvent::tags), QVariant{tags});
}

QSharedPointer<DFMRemoveTagsOfFileEvent> DFMRemoveTagsOfFileEvent::fromJson(const QJsonObject &json)
{
    (void)json;
    return QSharedPointer<DFMRemoveTagsOfFileEvent> { nullptr };
}

QList<QString> DFMRemoveTagsOfFileEvent::tags() const
{
    QVariant var{ property(QT_STRINGIFY(DFMRemoveTagsOfFileEvent::tags), {QString()}) };
    QList<QString> tag_name{ var.toStringList() };

    return tag_name;
}




DFMChangeTagColorEvent::DFMChangeTagColorEvent(const QObject *sender, const QColor &color, const DUrl &tagUrl)
    : DFMEvent{ Type::ChangeTagColor, sender },
      m_newColorForTag{ color },
      m_tagUrl{ tagUrl }
{
}

QSharedPointer<DFMChangeTagColorEvent> DFMChangeTagColorEvent::fromJson(const QJsonObject &json)
{
    (void)json;
    return QSharedPointer<DFMChangeTagColorEvent> { nullptr };
}

DFMGetTagsThroughFilesEvent::DFMGetTagsThroughFilesEvent(const QObject *sender, const QList<DUrl> &files)
    : DFMUrlListBaseEvent{ Type::GetTagsThroughFiles, sender, files }
{
}

QSharedPointer<DFMGetTagsThroughFilesEvent> DFMGetTagsThroughFilesEvent::fromJson(const QJsonObject &json)
{
    (void)json;
    return QSharedPointer<DFMGetTagsThroughFilesEvent> { nullptr };
}

DFMSetFileExtraProperties::DFMSetFileExtraProperties(const QObject *sender, const DUrl &url, const QVariantHash &ep)
    : DFMUrlBaseEvent(SetFileExtraProperties, sender, url)
{
    setProperty(QT_STRINGIFY(DFMSetFileExtraProperties::extraProperties), ep);
}

QVariantHash DFMSetFileExtraProperties::extraProperties() const
{
    return property(QT_STRINGIFY(DFMSetFileExtraProperties::extraProperties)).toHash();
}

DFMSetPermissionEvent::DFMSetPermissionEvent(const QObject *sender, const DUrl &url, const QFileDevice::Permissions &permissions)
    : DFMUrlBaseEvent{ Type::SetPermission, sender, url }
{
    setProperty(QT_STRINGIFY(DFMSetPermissionEvent::permissions), static_cast<int>(permissions));
}

QFileDevice::Permissions DFMSetPermissionEvent::permissions() const
{
    return static_cast<QFileDevice::Permissions>(property(QT_STRINGIFY(DFMSetPermissionEvent::permissions)).toInt());
}

DFMOpenFilesEvent::DFMOpenFilesEvent(const QObject *sender, const DUrlList &list, const bool isEnter)
    : DFMUrlListBaseEvent(OpenFiles, sender, list)
{
    setProperty(QT_STRINGIFY(DFMOpenFilesEvent::isEnter), isEnter);
}

bool DFMOpenFilesEvent::isEnter() const
{
    return property(QT_STRINGIFY(DFMOpenFilesEvent::isEnter)).toBool();
}

QSharedPointer<DFMOpenFilesEvent> DFMOpenFilesEvent::fromJson(const QJsonObject &json)
{
    DUrlList list;

    for (const QJsonValue value : json["urlList"].toArray()) {
        list << DUrl::fromUserInput(value.toString());
    }

    return dMakeEventPointer<DFMOpenFilesEvent>(Q_NULLPTR, list, json["isEnter"].toBool());
}

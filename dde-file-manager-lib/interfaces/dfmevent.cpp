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
    case DFMEvent::NewFolder:
        return QStringLiteral(QT_STRINGIFY(NewFolder));
    case DFMEvent::NewFile:
        return QStringLiteral(QT_STRINGIFY(NewFile));
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
    : DFMEvent((DFMEvent::Type)other.m_type, other.m_sender)
{
    m_accept = other.m_accept;
    m_data = other.m_data;
    m_propertys = other.m_propertys;
    m_id = other.m_id;
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
    m_propertys = other.m_propertys;
    m_id = other.m_id;

    return *this;
}

DFMEvent::Type DFMEvent::nameToType(const QString &name)
{
    for (int i = UnknowType; i <= CustomBase; ++i) {
        if (fmeventType2String((DFMEvent::Type)i) == name)
            return DFMEvent::Type(i);
    }

    return UnknowType;
}

quint64 DFMEvent::windowId() const
{
    if (m_id > 0)
        return m_id;

    if (!m_sender)
        return 0;

    const QWidget *w = qobject_cast<const QWidget*>(m_sender.data());

    if (w)
        return WindowManager::getWindowId(w);

    const QGraphicsWidget *gw = qobject_cast<const QGraphicsWidget*>(m_sender.data());

    return (gw && !gw->scene()->views().isEmpty()) ? WindowManager::getWindowId(gw->scene()->views().first()) : 0;
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

        if (url.isValid())
            list << url;
    }

    return list;
}

const QSharedPointer<DFMEvent> DFMEvent::fromJson(DFMEvent::Type type, const QJsonObject &json)
{
    switch ((int)type) {
    case OpenFile:
        return DFMOpenFileEvent::fromJson(json);
    case OpenFileByApp:
        return DFMOpenFileByAppEvent::fromJson(json);
    case CompressFiles:
        return DFMCompressEvnet::fromJson(json);
    case DecompressFile:
        return DFMDecompressEvnet::fromJson(json);
    case DecompressFileHere:
        return DFMDecompressHereEvnet::fromJson(json);
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
    case NewFolder:
        return DFMNewFolderEvent::fromJson(json);
    case NewFile:
        return DFMNewFileEvent::fromJson(json);
    case OpenFileLocation:
        return DFMOpenFileLocation::fromJson(json);
    case CreateSymlink:
        return DFMCreateSymlinkEvent::fromJson(json);
    case FileShare:
        return DFMFileShareEvnet::fromJson(json);
    case CancelFileShare:
        return DFMCancelFileShareEvent::fromJson(json);
    case OpenInTerminal:
        return DFMOpenInTerminalEvent::fromJson(json);
    case GetChildrens:
        return DFMGetChildrensEvent::fromJson(json);
    case CreateFileInfo:
        return DFMCreateFileInfoEvnet::fromJson(json);
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
    deg << "propertys:" << info.propertys();

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
    : DFMUrlListBaseEvent(0, DUrlList())
{

}

DFMUrlListBaseEvent::DFMUrlListBaseEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(DFMEvent::UnknowType, sender, list)
{

}

DFMUrlListBaseEvent::DFMUrlListBaseEvent(DFMEvent::Type type, const QObject *sender, const DUrlList &list)
    : DFMEvent(type, sender)
{
    setData(list);
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

DFMCompressEvnet::DFMCompressEvnet(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(CompressFiles, sender, list)
{

}

QSharedPointer<DFMCompressEvnet> DFMCompressEvnet::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(CompressFiles, json).staticCast<DFMCompressEvnet>();
}

DFMDecompressEvnet::DFMDecompressEvnet(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(DecompressFile, sender, list)
{

}

QSharedPointer<DFMDecompressEvnet> DFMDecompressEvnet::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(DecompressFile, json).staticCast<DFMDecompressEvnet>();
}

DFMDecompressHereEvnet::DFMDecompressHereEvnet(const QObject *sender, const DUrlList &list)
    : DFMDecompressEvnet(sender, list)
{
    m_type = DecompressFileHere;
}

QSharedPointer<DFMDecompressHereEvnet> DFMDecompressHereEvnet::fromJson(const QJsonObject &json)
{
    return DFMUrlListBaseEvent::fromJson(DecompressFileHere, json).staticCast<DFMDecompressHereEvnet>();
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

DFMRenameEvent::DFMRenameEvent(const QObject *sender, const DUrl &from, const DUrl &to)
    : DFMEvent(RenameFile, sender)
{
    setData(QPair<DUrl, DUrl>(from, to));
}

DUrlList DFMRenameEvent::handleUrlList() const
{
    const auto&& d = qvariant_cast<QPair<DUrl, DUrl>>(m_data);

    return DUrlList() << d.first << d.second;
}

QSharedPointer<DFMRenameEvent> DFMRenameEvent::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMRenameEvent>(Q_NULLPTR, DUrl::fromUserInput(json["from"].toString()), DUrl::fromUserInput(json["to"].toString()));
}

DFMDeleteEvent::DFMDeleteEvent(const QObject *sender, const DUrlList &list, bool silent)
    : DFMUrlListBaseEvent(DeleteFiles, sender, list)
{
    setProperty(QT_STRINGIFY(DFMDeleteEvent::silent), silent);
}

bool DFMDeleteEvent::silent() const
{
    return property(QT_STRINGIFY(DFMDeleteEvent::silent), false);
}

QSharedPointer<DFMDeleteEvent> DFMDeleteEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMDeleteEvent> &event = DFMUrlListBaseEvent::fromJson(DeleteFiles, json).staticCast<DFMDeleteEvent>();

    event->setProperty(QT_STRINGIFY(DFMDeleteEvent::silent), json["silent"].toBool());

    return event;
}

DFMMoveToTrashEvent::DFMMoveToTrashEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(MoveToTrash, sender, list)
{

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

DFMNewFolderEvent::DFMNewFolderEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(NewFolder, sender, url)
{

}

QSharedPointer<DFMNewFolderEvent> DFMNewFolderEvent::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(NewFolder, json).staticCast<DFMNewFolderEvent>();
}

DFMNewFileEvent::DFMNewFileEvent(const QObject *sender, const DUrl &url, const QString &suffix)
    : DFMUrlBaseEvent(NewFile, sender, url)
{
    setProperty(QT_STRINGIFY(DFMNewFileEvent::fileSuffix), suffix);
}

QString DFMNewFileEvent::fileSuffix() const
{
    return property(QT_STRINGIFY(DFMNewFileEvent::fileSuffix), QString());
}

QSharedPointer<DFMNewFileEvent> DFMNewFileEvent::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMNewFileEvent>(Q_NULLPTR, DUrl::fromUserInput(json["url"].toString()), json["suffix"].toString());
}

DFMOpenFileLocation::DFMOpenFileLocation(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenFileLocation, sender, url)
{

}

QSharedPointer<DFMOpenFileLocation> DFMOpenFileLocation::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(OpenFileLocation, json).staticCast<DFMOpenFileLocation>();
}

DFMCreateSymlinkEvent::DFMCreateSymlinkEvent(const QObject *sender, const DUrl &fileUrl, const DUrl &toUrl)
    : DFMEvent(CreateSymlink, sender)
{
    setData(makeData(fileUrl, toUrl));
}

DUrlList DFMCreateSymlinkEvent::handleUrlList() const
{
    return DUrlList() << fileUrl() << toUrl();
}

QSharedPointer<DFMCreateSymlinkEvent> DFMCreateSymlinkEvent::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMCreateSymlinkEvent>(Q_NULLPTR, DUrl::fromUserInput(json["fileUrl"].toString()),
                                                    DUrl::fromUserInput(json["toUrl"].toString()));
}

DFMFileShareEvnet::DFMFileShareEvnet(const QObject *sender, const DUrl &url,
                                     const QString &name, bool isWritable, bool allowGuest)
    : DFMUrlBaseEvent(FileShare, sender, url)
{
    setProperty(QT_STRINGIFY(DFMFileShareEvnet::name), name);
    setProperty(QT_STRINGIFY(DFMFileShareEvnet::isWritable), isWritable);
    setProperty(QT_STRINGIFY(DFMFileShareEvnet::allowGuest), allowGuest);
}

QString DFMFileShareEvnet::name() const
{
    return property(QT_STRINGIFY(DFMFileShareEvnet::name), QString());
}

bool DFMFileShareEvnet::isWritable() const
{
    return property(QT_STRINGIFY(DFMFileShareEvnet::isWritable), false);
}

bool DFMFileShareEvnet::allowGuest() const
{
    return property(QT_STRINGIFY(DFMFileShareEvnet::allowGuest), false);
}

QSharedPointer<DFMFileShareEvnet> DFMFileShareEvnet::fromJson(const QJsonObject &json)
{
    return dMakeEventPointer<DFMFileShareEvnet>(Q_NULLPTR, DUrl::fromUserInput(json["url"].toString()), json["name"].toString(),
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
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : DFMUrlBaseEvent(GetChildrens, sender, url)
{
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::nameFilters), nameFilters);
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::filters), filters);
    setProperty(QT_STRINGIFY(DFMGetChildrensEvent::flags), flags);
}

DFMGetChildrensEvent::DFMGetChildrensEvent(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                           QDir::Filters filters)
    : DFMGetChildrensEvent(sender, url, nameFilters, filters, QDirIterator::NoIteratorFlags)
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

QSharedPointer<DFMGetChildrensEvent> DFMGetChildrensEvent::fromJson(const QJsonObject &json)
{
    QStringList nameFilters;

    for (const QJsonValue &value : json["nameFilters"].toArray())
        nameFilters << value.toString();

    return dMakeEventPointer<DFMGetChildrensEvent>(Q_NULLPTR, DUrl::fromUserInput(json["url"].toString()),
                                                   nameFilters, (QDir::Filters)json["filters"].toInt());
}

DFMCreateDiriterator::DFMCreateDiriterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : DFMGetChildrensEvent(sender, fileUrl, nameFilters, filters, flags)
{
    m_type = CreateDiriterator;
}

DFMCreateDiriterator::DFMCreateDiriterator(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters)
    : DFMGetChildrensEvent(sender, fileUrl, nameFilters, filters)
{
    m_type = CreateDiriterator;
}

QSharedPointer<DFMCreateDiriterator> DFMCreateDiriterator::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMCreateDiriterator> &event = DFMGetChildrensEvent::fromJson(json).staticCast<DFMCreateDiriterator>();

    event->m_type = CreateDiriterator;

    return event;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                                   QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : DFMCreateDiriterator(sender, fileUrl, nameFilters, filters, flags)
{
    m_type = CreateGetChildrensJob;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const QObject *sender, const DUrl &fileUrl, const QStringList &nameFilters,
                                                   QDir::Filters filters)
    : DFMCreateDiriterator(sender, fileUrl, nameFilters, filters)
{
    m_type = CreateGetChildrensJob;
}

QSharedPointer<DFMCreateGetChildrensJob> DFMCreateGetChildrensJob::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMCreateGetChildrensJob> &event = DFMCreateDiriterator::fromJson(json).staticCast<DFMCreateGetChildrensJob>();

    event->m_type = CreateGetChildrensJob;

    return event;
}

DFMCreateFileInfoEvnet::DFMCreateFileInfoEvnet(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(CreateFileInfo, sender, url)
{

}

QSharedPointer<DFMCreateFileInfoEvnet> DFMCreateFileInfoEvnet::fromJson(const QJsonObject &json)
{
    return DFMUrlBaseEvent::fromJson(CreateFileInfo, json).staticCast<DFMCreateFileInfoEvnet>();
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
    return (const QWidget*)(property<quintptr>(QT_STRINGIFY(DFMChangeCurrentUrlEvent::window)));
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

DFMOpenUrlEvent::DFMOpenUrlEvent(const QObject *sender, const DUrlList &list, DFMOpenUrlEvent::DirOpenMode mode)
    : DFMUrlListBaseEvent(OpenUrl, sender, list)
{
    setProperty(QT_STRINGIFY(DFMOpenUrlEvent::dirOpenMode), mode);
}

DFMOpenUrlEvent::DirOpenMode DFMOpenUrlEvent::dirOpenMode() const
{
    return property(QT_STRINGIFY(DFMOpenUrlEvent::dirOpenMode), DirOpenMode::OpenNewWindow);
}

QSharedPointer<DFMOpenUrlEvent> DFMOpenUrlEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMOpenUrlEvent> &event = DFMUrlListBaseEvent::fromJson(OpenUrl, json).staticCast<DFMOpenUrlEvent>();

    event->setProperty(QT_STRINGIFY(DFMOpenUrlEvent::dirOpenMode), (DFMOpenUrlEvent::DirOpenMode)json["mode"].toInt());

    return event;
}

DFMMenuActionEvent::DFMMenuActionEvent(const QObject *sender, const DFileMenu *menu, const DUrl &currentUrl,
                                       const DUrlList &selectedUrls, DFMGlobal::MenuAction action)
    : DFMUrlListBaseEvent(MenuAction, sender, selectedUrls)
{
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::menu), (quintptr)menu);
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::currentUrl), currentUrl);
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::action), action);
}

const DFileMenu *DFMMenuActionEvent::menu() const
{
    return (const DFileMenu*)property<quintptr>(QT_STRINGIFY(DFMMenuActionEvent::menu), 0);
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

QSharedPointer<DFMMenuActionEvent> DFMMenuActionEvent::fromJson(const QJsonObject &json)
{
    const QSharedPointer<DFMMenuActionEvent> &event = DFMUrlListBaseEvent::fromJson(MenuAction, json).staticCast<DFMMenuActionEvent>();
    int action = DFMGlobal::instance()->metaObject()->enumerator(DFMGlobal::instance()->metaObject()->indexOfEnumerator("MenuAction")).keyToValue(json["action"].toString().toLocal8Bit().constData());

    event->setProperty(QT_STRINGIFY(DFMMenuActionEvent::currentUrl), DUrl::fromUserInput(json["currentUrl"].toString()));
    event->setProperty(QT_STRINGIFY(DFMMenuActionEvent::action), (DFMGlobal::MenuAction)action);

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

#include "dfmevent.h"
#include "views/windowmanager.h"

#include <QDebug>
#include <QWidget>

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

quint64 DFMEvent::eventId() const
{
    if (m_id > 0)
        return m_id;

    const QWidget *w = qobject_cast<const QWidget*>(m_sender.data());

    return w ? WindowManager::getWindowId(w) : 0;
}

void DFMEvent::setEventId(quint64 id)
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

QT_BEGIN_NAMESPACE
QString fmeventType2String(DFMEvent::Type type)
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
    default:
        return QStringLiteral("Custom: %1").arg(type);
    }
}

QDebug operator<<(QDebug deg, const DFMEvent &info)
{
    deg << "type:" << fmeventType2String(info.type()) << "sender:" << info.sender();
    deg << "data:" << info.data();

    return deg;
}
QT_END_NAMESPACE

DFMUrlBaseEvent::DFMUrlBaseEvent(const DUrl &url, const QObject *sender)
    : DFMUrlBaseEvent(DFMEvent::UnknowType, url, sender)
{

}

DFMUrlBaseEvent::DFMUrlBaseEvent(DFMEvent::Type type, const DUrl &url, const QObject *sender)
    : DFMEvent(type, sender)
{
    setData(url);
}

DFMUrlListBaseEvent::DFMUrlListBaseEvent(const DUrlList &list, const QObject *sender)
    : DFMUrlListBaseEvent(DFMEvent::UnknowType, list, sender)
{

}

DFMUrlListBaseEvent::DFMUrlListBaseEvent(DFMEvent::Type type, const DUrlList &list, const QObject *sender)
    : DFMEvent(type, sender)
{
    setData(list);
}

DFMOpenFileEvent::DFMOpenFileEvent(const DUrl &url, const QObject *sender)
    : DFMUrlBaseEvent(OpenFile, url, sender)
{

}

DFMOpenFileByAppEvent::DFMOpenFileByAppEvent(const QString &appName, const DUrl &url, const QObject *sender)
    : DFMOpenFileEvent(url, sender)
    , m_appName(appName)
{
    m_type = OpenFileByApp;
}

DFMCompressEvnet::DFMCompressEvnet(const DUrlList &list, const QObject *sender)
    : DFMUrlListBaseEvent(CompressFiles, list, sender)
{

}

DFMDecompressEvnet::DFMDecompressEvnet(const DUrlList &list, const QObject *sender)
    : DFMUrlListBaseEvent(DecompressFile, list, sender)
{

}

DFMDecompressHereEvnet::DFMDecompressHereEvnet(const DUrlList &list, const QObject *sender)
    : DFMDecompressEvnet(list, sender)
{
    m_type = DecompressFileHere;
}

DFMWriteUrlsToClipboardEvent::DFMWriteUrlsToClipboardEvent(DFMGlobal::ClipboardAction action,
                                                           const DUrlList &list, const QObject *sender)
    : DFMUrlListBaseEvent(WriteUrlsToClipboard, list, sender)
    , m_action(action)
{

}

DFMRenameEvent::DFMRenameEvent(const DUrl &from, const DUrl &to, const QObject *sender)
    : DFMEvent(RenameFile, sender)
{
    setData(QPair<DUrl, DUrl>(from, to));
}

DUrlList DFMRenameEvent::handleUrlList() const
{
    const auto&& d = qvariant_cast<QPair<DUrl, DUrl>>(m_data);

    return DUrlList() << d.first << d.second;
}

DFMDeleteEvent::DFMDeleteEvent(const DUrlList &list, const QObject *sender)
    : DFMUrlListBaseEvent(DeleteFiles, list, sender)
{

}

DFMMoveToTrashEvent::DFMMoveToTrashEvent(const DUrlList &list, const QObject *sender)
    : DFMEvent(MoveToTrash, sender)
{
    setData(list);
}

DFMPasteEvent::DFMPasteEvent(DFMGlobal::ClipboardAction action, const DUrl &targetUrl,
                             const DUrlList &list, const QObject *sender)
    : DFMUrlListBaseEvent(PasteFile, list, sender)
    , m_action(action)
    , m_target(targetUrl)
{
    setData(list);
}

DUrlList DFMPasteEvent::handleUrlList() const
{
    return DUrlList() << targetUrl() << urlList();
}

DFMNewFolderEvent::DFMNewFolderEvent(const DUrl &targetUrl, const QObject *sender)
    : DFMEvent(NewFolder, sender)
{
    setData(targetUrl);
}

DFMNewFileEvent::DFMNewFileEvent(const DUrl &targetUrl, const QString &fileSuffix, const QObject *sender)
    : DFMEvent(NewFile, sender)
    , m_suffix(fileSuffix)
{
    setData(targetUrl);
}

DFMOpenFileLocation::DFMOpenFileLocation(const DUrl &url, const QObject *sender)
    : DFMUrlBaseEvent(OpenFileLocation, url, sender)
{

}

DFMCreateSymlinkEvent::DFMCreateSymlinkEvent(const DUrl &fileUrl, const DUrl &toUrl, const QObject *sender)
    : DFMEvent(CreateSymlink, sender)
{
    setData(makeData(fileUrl, toUrl));
}

DUrlList DFMCreateSymlinkEvent::handleUrlList() const
{
    return DUrlList() << fileUrl() << toUrl();
}

DFMFileShareEvnet::DFMFileShareEvnet(const DUrl &url, const QString &name, bool isWritable,
                                                 bool allowGuest, const QObject *sender)
    : DFMUrlBaseEvent(FileShare, url, sender)
    , m_name(name)
    , m_writable(isWritable)
    , m_allowGuest(allowGuest)
{

}

DFMCancelFileShareEvent::DFMCancelFileShareEvent(const DUrl &url, const QObject *sender)
    : DFMUrlBaseEvent(CancelFileShare, url, sender)
{

}

DFMOpenInTerminalEvent::DFMOpenInTerminalEvent(const DUrl &url, const QObject *sender)
    : DFMUrlBaseEvent(OpenInTerminal, url, sender)
{

}

DFMGetChildrensEvent::DFMGetChildrensEvent(const DUrl &url, const QStringList &nameFilters,
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                           const QObject *sender)
    : DFMUrlBaseEvent(GetChildrens, url, sender)
    , m_nameFilters(nameFilters)
    , m_filters(filters)
    , m_flags(flags)
{

}

DFMGetChildrensEvent::DFMGetChildrensEvent(const DUrl &url, const QStringList &nameFilters,
                                           QDir::Filters filters, const QObject *sender)
    : DFMGetChildrensEvent(url, nameFilters, filters, QDirIterator::NoIteratorFlags, sender)
{

}

DFMCreateDiriterator::DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                           const QObject *sender)
    : DFMGetChildrensEvent(fileUrl, nameFilters, filters, flags, sender)
{
    m_type = CreateDiriterator;
}

DFMCreateDiriterator::DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, const QObject *sender)
    : DFMGetChildrensEvent(fileUrl, nameFilters, filters, sender)
{
    m_type = CreateDiriterator;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                                   QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                   const QObject *sender)
    : DFMCreateDiriterator(fileUrl, nameFilters, filters, flags, sender)
{
    m_type = CreateGetChildrensJob;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                                   QDir::Filters filters, const QObject *sender)
    : DFMCreateDiriterator(fileUrl, nameFilters, filters, sender)
{
    m_type = CreateGetChildrensJob;
}

DFMCreateFileInfoEvnet::DFMCreateFileInfoEvnet(const DUrl &url, const QObject *sender)
    : DFMUrlBaseEvent(CreateFileInfo, url, sender)
{

}

DFMCreateFileWatcherEvent::DFMCreateFileWatcherEvent(const DUrl &url, const QObject *sender)
    : DFMUrlBaseEvent(CreateFileWatcher, url, sender)
{

}

DFMChangeCurrentUrlEvent::DFMChangeCurrentUrlEvent(const DUrl &url, const QWidget *window, const QObject *sender)
    : DFMUrlBaseEvent(ChangeCurrentUrl, url, sender)
    , m_window(window)
{

}

DFMOpenNewWindowEvent::DFMOpenNewWindowEvent(const DUrlList &list, bool force, const QObject *sender)
    : DFMUrlListBaseEvent(OpenNewWindow, list, sender)
    , m_force(force)
{

}

DFMOpenUrlEvent::DFMOpenUrlEvent(const DUrlList &list, DFMOpenUrlEvent::DirOpenMode mode, const QObject *sender)
    : DFMUrlListBaseEvent(OpenUrl, list, sender)
    , m_dirOpenModel(mode)
{

}

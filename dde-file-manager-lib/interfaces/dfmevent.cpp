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

quint64 DFMEvent::windowId() const
{
    if (m_id > 0)
        return m_id;

    const QWidget *w = qobject_cast<const QWidget*>(m_sender.data());

    return w ? WindowManager::getWindowId(w) : 0;
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

DFMUrlListBaseEvent::DFMUrlListBaseEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(DFMEvent::UnknowType, sender, list)
{

}

DFMUrlListBaseEvent::DFMUrlListBaseEvent(DFMEvent::Type type, const QObject *sender, const DUrlList &list)
    : DFMEvent(type, sender)
{
    setData(list);
}

DFMOpenFileEvent::DFMOpenFileEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenFile, sender, url)
{

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

DFMCompressEvnet::DFMCompressEvnet(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(CompressFiles, sender, list)
{

}

DFMDecompressEvnet::DFMDecompressEvnet(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(DecompressFile, sender, list)
{

}

DFMDecompressHereEvnet::DFMDecompressHereEvnet(const QObject *sender, const DUrlList &list)
    : DFMDecompressEvnet(sender, list)
{
    m_type = DecompressFileHere;
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

DFMDeleteEvent::DFMDeleteEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(DeleteFiles, sender, list)
{

}

DFMMoveToTrashEvent::DFMMoveToTrashEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(MoveToTrash, sender, list)
{

}

DFMRestoreFromTrashEvent::DFMRestoreFromTrashEvent(const QObject *sender, const DUrlList &list)
    : DFMUrlListBaseEvent(RestoreFromTrash, sender, list)
{

}

DFMPasteEvent::DFMPasteEvent(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrl &targetUrl,
                             const DUrlList &list)
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

DFMNewFolderEvent::DFMNewFolderEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(NewFolder, sender, url)
{

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

DFMOpenFileLocation::DFMOpenFileLocation(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenFileLocation, sender, url)
{

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

DFMCancelFileShareEvent::DFMCancelFileShareEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(CancelFileShare, sender, url)
{

}

DFMOpenInTerminalEvent::DFMOpenInTerminalEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(OpenInTerminal, sender, url)
{

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

DFMCreateFileInfoEvnet::DFMCreateFileInfoEvnet(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(CreateFileInfo, sender, url)
{

}

DFMCreateFileWatcherEvent::DFMCreateFileWatcherEvent(const QObject *sender, const DUrl &url)
    : DFMUrlBaseEvent(CreateFileWatcher, sender, url)
{

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

DFMOpenNewWindowEvent::DFMOpenNewWindowEvent(const QObject *sender, const DUrlList &list, bool force)
    : DFMUrlListBaseEvent(OpenNewWindow, sender, list)
{
    setProperty(QT_STRINGIFY(DFMOpenNewWindowEvent::force), force);
}

bool DFMOpenNewWindowEvent::force() const
{
    return property(QT_STRINGIFY(DFMOpenNewWindowEvent::force), false);
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

DFMMenuActionEvent::DFMMenuActionEvent(const QObject *sender, const DFileMenu *menu, const DUrl &currentUrl,
                                       const DUrlList &selectedUrls, DFMGlobal::MenuAction action)
    : DFMEvent(MenuAction, sender)
{
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::menu), (quintptr)menu);
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::currentUrl), currentUrl);
    setProperty(QT_STRINGIFY(DFMMenuActionEvent::action), action);

    setData(selectedUrls);
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

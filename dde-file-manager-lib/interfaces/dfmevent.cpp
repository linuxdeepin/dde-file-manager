#include "dfmevent.h"

#include <QDebug>

DFMEvent::DFMEvent(int wId, DFMEvent::EventSource source, const DUrl &fileUrl)
    : data(new FMEventData)
{
    data->windowId = wId;
    data->source = source;
    data->fileUrl = fileUrl;
}

DFMEvent::DFMEvent(DFMEvent::Type type, QObject *sender)
    : m_type(type)
    , m_sender(sender)
    , m_accept(true)
{

}

DFMEvent::DFMEvent(const DFMEvent &other)
    : DFMEvent((DFMEvent::Type)other.m_type, other.m_sender)
{
    m_accept = other.m_accept;
    data = other.data;
}

DFMEvent::~DFMEvent()
{

}

DFMEvent &DFMEvent::operator =(const DFMEvent &other)
{
    data = other.data;
    m_type = other.m_type;
    m_sender = other.m_sender;
    m_accept = other.m_accept;
    return *this;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info)
{
    deg << "window id:" << info.windowId() << "source:" << info.source() << "url" << info.fileUrl();
    deg << "urlList{";
    foreach (DUrl url, info.fileUrlList()) {
        deg << url;
    }
    deg << "}";
    return deg;
}
QT_END_NAMESPACE

DFMOpenFileEvent::DFMOpenFileEvent(const DUrl &url, QObject *sender)
    : DFMEvent(OpenFile, sender)
    , m_url(url)
{

}

DFMCompressEvnet::DFMCompressEvnet(const DUrlList &list, QObject *sender)
    : DFMEvent(CompressFiles, sender)
    , m_list(list)
{

}

DFMDecompressEvnet::DFMDecompressEvnet(const DUrlList &list, QObject *sender)
    : DFMEvent(DecompressFile, sender)
    , m_list(list)
{

}

DFMWriteUrlsToClipboardEvent::DFMWriteUrlsToClipboardEvent(DFMGlobal::ClipboardAction action,
                                                           const DUrlList &list, QObject *sender)
    : DFMEvent(WriteUrlsToClipboard, sender)
    , m_action(action)
    , m_list(list)
{

}

DFMRenameEvent::DFMRenameEvent(const DUrl &from, const DUrl &to, QObject *sender)
    : DFMEvent(RenameFile, sender)
    , m_from(from)
    , m_to(to)
{

}

DFMDeleteEvent::DFMDeleteEvent(const DUrlList &list, QObject *sender)
    : DFMEvent(DeleteFiles, sender)
    , m_list(list)
{

}

DFMMoveToTrashEvent::DFMMoveToTrashEvent(const DUrlList &list, QObject *sender)
    : DFMEvent(MoveToTrash, sender)
    , m_list(list)
{

}

DFMPasteEvent::DFMPasteEvent(DFMGlobal::ClipboardAction action, const DUrl &targetUrl,
                             const DUrlList &list, QObject *sender)
    : DFMEvent(PasteFile, sender)
    , m_action(action)
    , m_target(targetUrl)
    , m_list(list)
{

}

DFMNewFolderEvent::DFMNewFolderEvent(const DUrl &targetUrl, QObject *sender)
    : DFMEvent(NewFolder, sender)
    , m_target(targetUrl)
{

}

DFMNewFileEvent::DFMNewFileEvent(const DUrl &targetUrl, const QString &fileSuffix, QObject *sender)
    : DFMEvent(NewFile, sender)
    , m_target(targetUrl)
    , m_suffix(fileSuffix)
{

}

DFMCreateSymlinkEvent::DFMCreateSymlinkEvent(const DUrl &fileUrl, const DUrl &toUrl, QObject *sender)
    : DFMEvent(CreateSymlink, sender)
    , m_fileUrl(fileUrl)
    , m_toUrl(toUrl)
{

}

DFMSetFileShareEnabledEvnet::DFMSetFileShareEnabledEvnet(const DUrl &fileUrl, bool enabled, QObject *sender)
    : DFMEvent(SetFileShareEnabled, sender)
    , m_fileUrl(fileUrl)
    , m_enabled(enabled)
{

}

DFMOpenInTerminalEvent::DFMOpenInTerminalEvent(const DUrl &fileUrl, QObject *sender)
    : DFMEvent(OpenInTerminal, sender)
    , m_fileUrl(fileUrl)
{

}

DFMGetChildrensEvent::DFMGetChildrensEvent(const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                           QObject *sender)
    : DFMEvent(GetChildrens, sender)
    , m_fileUrl(fileUrl)
    , m_nameFilters(nameFilters)
    , m_filters(filters)
    , m_flags(flags)
{

}

DFMGetChildrensEvent::DFMGetChildrensEvent(const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, QObject *sender)
    : DFMGetChildrensEvent(fileUrl, nameFilters, filters, QDirIterator::NoIteratorFlags, sender)
{

}

DFMCreateDiriterator::DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                           QObject *sender)
    : DFMGetChildrensEvent(fileUrl, nameFilters, filters, flags, sender)
{
    m_type = CreateDiriterator;
}

DFMCreateDiriterator::DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                           QDir::Filters filters, QObject *sender)
    : DFMGetChildrensEvent(fileUrl, nameFilters, filters, sender)
{
    m_type = CreateDiriterator;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                                   QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                                   QObject *sender)
    : DFMGetChildrensEvent(fileUrl, nameFilters, filters, flags, sender)
{
    m_type = CreateGetChildrensJob;
}

DFMCreateGetChildrensJob::DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                                   QDir::Filters filters, QObject *sender)
    : DFMGetChildrensEvent(fileUrl, nameFilters, filters, sender)
{
    m_type = CreateGetChildrensJob;
}

DFMCreateFileInfoEvnet::DFMCreateFileInfoEvnet(const DUrl &fileUrl, QObject *sender)
    : DFMEvent(CreateFileInfo, sender)
    , m_fileUrl(fileUrl)
{

}

DFMCreateFileWatcherEvent::DFMCreateFileWatcherEvent(const DUrl &fileUrl, QObject *sender)
    : DFMEvent(CreateFileWatcher, sender)
    , m_fileUrl(fileUrl)
{

}

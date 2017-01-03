#ifndef FMEVENT_H
#define FMEVENT_H

#include <QString>
#include <QSharedData>
#include <QMetaType>
#include <QEvent>
#include <QPointer>
#include <QDir>
#include <QDirIterator>

#include <functional>

#include "durl.h"
#include "dfmglobal.h"

class DFMEvent
{
public:
    enum EventSource {
        CrumbButton,
        LeftSideBar,
        BackAndForwardButton,
        UpButton,
        FileView,
        SearchBar,
        Menu,
        Unknow
    };

    enum Type {
        OpenFile,
        OpenFileByApp,
        CompressFiles,
        DecompressFile,
        DecompressFileHere,
        WriteUrlsToClipboard,
        RenameFile,
        DeleteFiles,
        MoveToTrash,
        PasteFile,
        NewFolder,
        NewFile,
        CreateSymlink,
        SetFileShareEnabled,
        OpenInTerminal,
        GetChildrens,
        CreateFileInfo,
        CreateDiriterator,
        CreateGetChildrensJob,
        CreateFileWatcher,
        CustomBase = 1000                            // first user event id
    };

    class FMEventData : public QSharedData
    {
    private:
        int windowId = -1;
        EventSource source = Unknow;
        EventSource parentSource = Unknow;
        DUrl fileUrl;
        DUrlList fileUrlList;

        friend class DFMEvent;
        int bookMarkIndex = -1;
    };

    explicit DFMEvent(int wId = -1, EventSource source = Unknow, const DUrl &fileUrl = DUrl());
    explicit DFMEvent(Type type, QObject *sender = 0);

    DFMEvent(const DFMEvent &other);

    virtual ~DFMEvent();

    DFMEvent &operator =(const DFMEvent &other);

    inline Type type() const { return static_cast<Type>(m_type);}
    inline QPointer<QObject> sender() const {return m_sender;}

    inline void setAccepted(bool accepted) { m_accept = accepted; }
    inline bool isAccepted() const { return m_accept; }

    inline void accept() { m_accept = true; }
    inline void ignore() { m_accept = false; }

    inline DFMEvent &operator <<(EventSource source)
    {
        data->parentSource = data->source;
        data->source = source;
        return *this;
    }

    inline DFMEvent &operator <<(int wId)
    {data->windowId = wId; return *this;}

    inline DFMEvent &operator <<(const DUrl &fileUrl)
    {data->fileUrl = fileUrl; return *this;}

    inline void operator <<(const DUrlList &list)
    {data->fileUrlList = list; /*return *this;*/}

    inline int windowId() const
    {return data->windowId;}

    inline EventSource source() const
    {return data->source;}

    inline EventSource parentSource() const
    {return data->parentSource;}

    inline const DUrl &fileUrl() const
    {return data->fileUrl;}

    inline const DUrlList &fileUrlList() const
    {return data->fileUrlList;}

    inline const int &bookmarkIndex() const
    {return data->bookMarkIndex;}

    inline void setBookmarkIndex(const int& index)
    {data->bookMarkIndex = index;}

protected:
    ushort m_type;
    QPointer<QObject> m_sender;

private:
    ushort m_accept : 1;

    QSharedDataPointer<FMEventData> data;
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info);
QT_END_NAMESPACE

class DFMOpenFileEvent : public DFMEvent
{
public:
    explicit DFMOpenFileEvent(const DUrl &url, QObject *sender = 0);

    inline DUrl url() const { return m_url;}

private:
    DUrl m_url;
};

class DFMCompressEvnet : public DFMEvent
{
public:
    explicit DFMCompressEvnet(const DUrlList &list, QObject *sender = 0);

    inline DUrlList urlList() const { return m_list;}

private:
    DUrlList m_list;
};

class DFMDecompressEvnet : public DFMEvent
{
public:
    explicit DFMDecompressEvnet(const DUrlList &list, QObject *sender = 0);

    inline DUrlList urlList() const { return m_list;}

private:
    DUrlList m_list;
};


class DFMWriteUrlsToClipboardEvent : public DFMEvent
{
public:
    explicit DFMWriteUrlsToClipboardEvent(DFMGlobal::ClipboardAction action, const DUrlList &list, QObject *sender = 0);

    inline DFMGlobal::ClipboardAction action() const { return m_action;}
    inline DUrlList urlList() const { return m_list;}

private:
    DFMGlobal::ClipboardAction m_action;
    DUrlList m_list;
};

class DFMRenameEvent : public DFMEvent
{
public:
    explicit DFMRenameEvent(const DUrl &from, const DUrl &to, QObject *sender = 0);

    inline DUrl fromUrl() const { return m_from;}
    inline DUrl toUrl() const { return m_to;}

private:
    DUrl m_from;
    DUrl m_to;
};

class DFMDeleteEvent : public DFMEvent
{
public:
    explicit DFMDeleteEvent(const DUrlList &list, QObject *sender = 0);

    inline DUrlList urlList() const { return m_list;}

private:
    DUrlList m_list;
};

class DFMMoveToTrashEvent : public DFMEvent
{
public:
    explicit DFMMoveToTrashEvent(const DUrlList &list, QObject *sender = 0);

    inline DUrlList urlList() const { return m_list;}

private:
    DUrlList m_list;
};

class DFMPasteEvent : public DFMEvent
{
public:
    explicit DFMPasteEvent(DFMGlobal::ClipboardAction action, const DUrl &targetUrl,
                           const DUrlList &list, QObject *sender = 0);

    inline DFMGlobal::ClipboardAction action() const { return m_action;}
    inline DUrl targetUrl() const { return m_target;}
    inline DUrlList urlList() const { return m_list;}

private:
    DFMGlobal::ClipboardAction m_action;
    DUrl m_target;
    DUrlList m_list;
};

class DFMNewFolderEvent : public DFMEvent
{
public:
    explicit DFMNewFolderEvent(const DUrl &targetUrl, QObject *sender = 0);

    inline DUrl targetUrl() const { return m_target;}

private:
    DUrl m_target;
};

class DFMNewFileEvent : public DFMEvent
{
public:
    explicit DFMNewFileEvent(const DUrl &targetUrl, const QString &fileSuffix, QObject *sender = 0);

    inline DUrl targetUrl() const { return m_target;}
    inline QString fileSuffix() const { return m_suffix;}

private:
    DUrl m_target;
    QString m_suffix;
};

class DFMCreateSymlinkEvent : public DFMEvent
{
public:
    explicit DFMCreateSymlinkEvent(const DUrl &fileUrl, const DUrl &toUrl, QObject *sender = 0);

    inline DUrl fileUrl() const { return m_fileUrl;}
    inline DUrl toUrl() const { return m_toUrl;}

private:
    DUrl m_fileUrl;
    DUrl m_toUrl;
};

class DFMSetFileShareEnabledEvnet : public DFMEvent
{
public:
    explicit DFMSetFileShareEnabledEvnet(const DUrl &fileUrl, bool enabled, QObject *sender = 0);

    inline DUrl fileUrl() const { return m_fileUrl;}
    inline bool enabled() const { return m_enabled;}

private:
    DUrl m_fileUrl;
    bool m_enabled;
};

class DFMOpenInTerminalEvent : public DFMEvent
{
public:
    explicit DFMOpenInTerminalEvent(const DUrl &fileUrl, QObject *sender = 0);

    inline DUrl fileUrl() const { return m_fileUrl;}

private:
    DUrl m_fileUrl;
};

class DFMGetChildrensEvent : public DFMEvent
{
public:
    explicit DFMGetChildrensEvent(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  QObject *sender = 0);
    explicit DFMGetChildrensEvent(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QObject *sender = 0);

    inline DUrl fileUrl() const { return m_fileUrl;}
    inline QStringList nameFilters() const { return m_nameFilters;}
    inline QDir::Filters filters() const { return m_filters;}
    inline QDirIterator::IteratorFlags flags() const { return m_flags;}

private:
    DUrl m_fileUrl;
    QStringList m_nameFilters;
    QDir::Filters m_filters;
    QDirIterator::IteratorFlags m_flags;
};

class DFMCreateDiriterator : public DFMGetChildrensEvent
{
public:
    explicit DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  QObject *sender = 0);
    explicit DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QObject *sender = 0);
};

class DFMCreateGetChildrensJob : public DFMGetChildrensEvent
{
public:
    explicit DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  QObject *sender = 0);
    explicit DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QObject *sender = 0);
};

class DFMCreateFileInfoEvnet : public DFMEvent
{
public:
    explicit DFMCreateFileInfoEvnet(const DUrl &fileUrl, QObject *sender = 0);

    inline DUrl fileUrl() const { return m_fileUrl;}

private:
    DUrl m_fileUrl;
};

class DFMCreateFileWatcherEvent : public DFMEvent
{
public:
    explicit DFMCreateFileWatcherEvent(const DUrl &fileUrl, QObject *sender = 0);

    inline DUrl fileUrl() const { return m_fileUrl;}

private:
    DUrl m_fileUrl;
};

#endif // FMEVENT_H

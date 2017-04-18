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
        OpenFileLocation,
        CreateSymlink,
        FileShare,
        CancelFileShare,
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
    explicit DFMEvent(Type type, const QObject *sender = 0);

    DFMEvent(const DFMEvent &other);

    virtual ~DFMEvent();

    DFMEvent &operator =(const DFMEvent &other);

    inline Type type() const { return static_cast<Type>(m_type);}
    inline QPointer<const QObject> sender() const {return m_sender;}

    inline void setAccepted(bool accepted) { m_accept = accepted; }
    inline bool isAccepted() const { return m_accept; }

    inline void accept() { m_accept = true; }
    inline void ignore() { m_accept = false; }

    inline DFMEvent &operator <<(EventSource source)
    {
        _data->parentSource = _data->source;
        _data->source = source;
        return *this;
    }

    inline DFMEvent &operator <<(int wId)
    {_data->windowId = wId; return *this;}

    inline DFMEvent &operator <<(const DUrl &fileUrl)
    {_data->fileUrl = fileUrl; return *this;}

    inline void operator <<(const DUrlList &list)
    {_data->fileUrlList = list; /*return *this;*/}

    inline int windowId() const
    {return _data->windowId;}

    inline EventSource source() const
    {return _data->source;}

    inline EventSource parentSource() const
    {return _data->parentSource;}

    inline const DUrl &fileUrl() const
    {return _data->fileUrl;}

    inline const DUrlList &fileUrlList() const
    {return _data->fileUrlList;}

    inline const int &bookmarkIndex() const
    {return _data->bookMarkIndex;}

    inline void setBookmarkIndex(const int& index)
    {_data->bookMarkIndex = index;}

    //! 在DFileServices中通过此url列表来获取处理此事件的Controller
    virtual DUrlList handleUrlList() const;
    inline void setData(const QVariant &data) { m_data = data;}
    template <typename T>
    void setData(T&& data)
    {
        m_data = QVariant::fromValue(std::forward<T>(data));
    }
    inline QVariant data() const
    { return m_data;}

protected:
    ushort m_type;
    QVariant m_data;
    QPointer<const QObject> m_sender;

private:
    ushort m_accept : 1;

    QSharedDataPointer<FMEventData> _data;
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info);
QT_END_NAMESPACE

template<class T, typename... Args>
QSharedPointer<T> dMakeEventPointer(Args&&... args)
{
    return QSharedPointer<T>(new T(std::forward<Args>(args)...));
}

class DFMOpenFileEvent : public DFMEvent
{
public:
    explicit DFMOpenFileEvent(const DUrl &url, const QObject *sender = 0);

    inline DUrl url() const { return qvariant_cast<DUrl>(m_data);}
};

class DFMOpenFileByAppEvent : public DFMOpenFileEvent
{
public:
    explicit DFMOpenFileByAppEvent(const QString &appName, const DUrl &url, const QObject *sender = 0);

    inline QString appName() const { return m_appName;}

private:
    QString m_appName;
};

class DFMCompressEvnet : public DFMEvent
{
public:
    explicit DFMCompressEvnet(const DUrlList &list, const QObject *sender = 0);

    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}
};

class DFMDecompressEvnet : public DFMEvent
{
public:
    explicit DFMDecompressEvnet(const DUrlList &list, const QObject *sender = 0);

    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}
};

class DFMDecompressHereEvnet : public DFMDecompressEvnet
{
public:
    explicit DFMDecompressHereEvnet(const DUrlList &list, const QObject *sender = 0);
};

class DFMWriteUrlsToClipboardEvent : public DFMEvent
{
public:
    explicit DFMWriteUrlsToClipboardEvent(DFMGlobal::ClipboardAction action, const DUrlList &list, const QObject *sender = 0);

    inline DFMGlobal::ClipboardAction action() const { return m_action;}
    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}

private:
    DFMGlobal::ClipboardAction m_action;
};

class DFMRenameEvent : public DFMEvent
{
public:
    explicit DFMRenameEvent(const DUrl &from, const DUrl &to, const QObject *sender = 0);

    inline DUrl fromUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).first;}
    inline DUrl toUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).second;}

    DUrlList handleUrlList() const Q_DECL_OVERRIDE;

    inline static QVariant makeData(const DUrl &from, const DUrl &to)
    { return QVariant::fromValue(QPair<DUrl, DUrl>(from, to));}
};

class DFMDeleteEvent : public DFMEvent
{
public:
    explicit DFMDeleteEvent(const DUrlList &list, const QObject *sender = 0);

    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}
};

class DFMMoveToTrashEvent : public DFMEvent
{
public:
    explicit DFMMoveToTrashEvent(const DUrlList &list, const QObject *sender = 0);

    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}
};

class DFMPasteEvent : public DFMEvent
{
public:
    explicit DFMPasteEvent(DFMGlobal::ClipboardAction action, const DUrl &targetUrl,
                           const DUrlList &list, const QObject *sender = 0);

    inline DFMGlobal::ClipboardAction action() const { return m_action;}
    inline DUrl targetUrl() const { return m_target;}
    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}

    DUrlList handleUrlList() const Q_DECL_OVERRIDE;

private:
    DFMGlobal::ClipboardAction m_action;
    DUrl m_target;
};

class DFMNewFolderEvent : public DFMEvent
{
public:
    explicit DFMNewFolderEvent(const DUrl &targetUrl, const QObject *sender = 0);

    inline DUrl targetUrl() const { return qvariant_cast<DUrl>(m_data);}
};

class DFMNewFileEvent : public DFMEvent
{
public:
    explicit DFMNewFileEvent(const DUrl &targetUrl, const QString &fileSuffix, const QObject *sender = 0);

    inline DUrl targetUrl() const { return qvariant_cast<DUrl>(m_data);}
    inline QString fileSuffix() const { return m_suffix;}

private:
    QString m_suffix;
};

class DFMOpenFileLocation : public DFMEvent
{
public:
    explicit DFMOpenFileLocation(const DUrl &url, const QObject *sender = 0);

    inline DUrl url() const { return qvariant_cast<DUrl>(m_data);}
};

class DFMCreateSymlinkEvent : public DFMEvent
{
public:
    explicit DFMCreateSymlinkEvent(const DUrl &fileUrl, const DUrl &toUrl, const QObject *sender = 0);

    inline DUrl fileUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).first;}
    inline DUrl toUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).second;}

    DUrlList handleUrlList() const Q_DECL_OVERRIDE;

    inline static QVariant makeData(const DUrl &url, const DUrl &to)
    { return QVariant::fromValue(QPair<DUrl, DUrl>(url, to));}
};

class DFMFileShareEvnet : public DFMEvent
{
public:
    explicit DFMFileShareEvnet(const DUrl &fileUrl, const QString &name, bool isWritable = false, bool allowGuest = false, const QObject *sender = 0);

    inline DUrl fileUrl() const { return qvariant_cast<DUrl>(m_data);}
    inline QString name() const { return m_name;}
    inline bool isWritable() const { return m_writable;}
    inline bool allowGuest() const { return m_allowGuest;}

private:
    QString m_name;
    bool m_writable;
    bool m_allowGuest;
};

class DFMCancelFileShareEvent : public DFMEvent
{
public:
    explicit DFMCancelFileShareEvent(const DUrl &fileUrl, const QObject *sender = 0);

    inline DUrl fileUrl() const { return qvariant_cast<DUrl>(m_data);}
};

class DFMOpenInTerminalEvent : public DFMEvent
{
public:
    explicit DFMOpenInTerminalEvent(const DUrl &fileUrl, const QObject *sender = 0);

    inline DUrl fileUrl() const { return qvariant_cast<DUrl>(m_data);}
};

class DFMGetChildrensEvent : public DFMEvent
{
public:
    explicit DFMGetChildrensEvent(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  const QObject *sender = 0);
    explicit DFMGetChildrensEvent(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, const QObject *sender = 0);

    inline DUrl fileUrl() const { return qvariant_cast<DUrl>(m_data);}
    inline QStringList nameFilters() const { return m_nameFilters;}
    inline QDir::Filters filters() const { return m_filters;}
    inline QDirIterator::IteratorFlags flags() const { return m_flags;}

private:
    QStringList m_nameFilters;
    QDir::Filters m_filters;
    QDirIterator::IteratorFlags m_flags;
};

class DFMCreateDiriterator : public DFMGetChildrensEvent
{
public:
    explicit DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  const QObject *sender = 0);
    explicit DFMCreateDiriterator(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, const QObject *sender = 0);
};

class DFMCreateGetChildrensJob : public DFMCreateDiriterator
{
public:
    explicit DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  const QObject *sender = 0);
    explicit DFMCreateGetChildrensJob(const DUrl &fileUrl, const QStringList &nameFilters,
                                  QDir::Filters filters, const QObject *sender = 0);
};

class DFMCreateFileInfoEvnet : public DFMEvent
{
public:
    explicit DFMCreateFileInfoEvnet(const DUrl &fileUrl, const QObject *sender = 0);

    inline DUrl fileUrl() const { return qvariant_cast<DUrl>(m_data);}
};

class DFMCreateFileWatcherEvent : public DFMEvent
{
public:
    explicit DFMCreateFileWatcherEvent(const DUrl &fileUrl, const QObject *sender = 0);

    inline DUrl fileUrl() const { return qvariant_cast<DUrl>(m_data);}
};

#endif // FMEVENT_H

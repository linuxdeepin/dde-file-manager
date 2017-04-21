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
    enum Type {
        UnknowType,
        // for file serices
        OpenFile,
        OpenFileByApp,
        CompressFiles,
        DecompressFile,
        DecompressFileHere,
        WriteUrlsToClipboard,
        RenameFile,
        DeleteFiles,
        MoveToTrash,
        RestoreFromTrash,
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
        // other
        ChangeCurrentUrl,
        OpenNewWindow,
        OpenUrl,
        MenuAction,
        // user custom
        CustomBase = 1000                            // first user event id
    };

    explicit DFMEvent(const QObject *sender = 0);
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

    quint64 windowId() const;
    void setWindowId(quint64 id);

    //! 在DFileServices中通过此url列表来获取处理此事件的Controller
    virtual DUrlList handleUrlList() const;
    inline void setData(const QVariant &data) { m_data = data;}
    template<typename T>
    void setData(T&& data)
    {
        m_data = QVariant::fromValue(std::forward<T>(data));
    }
    inline QVariant data() const
    { return m_data;}
    template<typename T>
    inline T data() const
    { return qvariant_cast<T>(m_data);}

    inline DUrl fileUrl() const
    { return data<DUrl>();}
    inline DUrlList fileUrlList() const
    { return data<DUrlList>();}

    inline QVariant property(const QString &name, const QVariant &defaultValue = QVariant()) const
    { return m_propertys.value(name, defaultValue);}
    template<typename T>
    T property(const QString &name, T&& defaultValue) const
    { return qvariant_cast<T>(m_propertys.value(name, QVariant::fromValue(std::forward<T>(defaultValue))));}
    template<typename T>
    T property(const QString &name) const
    { return qvariant_cast<T>(property(name));}
    inline void setProperty(const QString &name, const QVariant &value)
    { m_propertys[name] = value;}
    template<typename T>
    void setProperty(const QString &name, T&& value)
    {
        m_propertys[name] = QVariant::fromValue(std::forward<T>(value));
    }

protected:
    ushort m_type;
    QVariant m_data;
    QVariantMap m_propertys;
    QPointer<const QObject> m_sender;

private:
    ushort m_accept : 1;
    quint64 m_id;
};

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info);
QT_END_NAMESPACE

class DFMUrlBaseEvent : public DFMEvent
{
public:
    explicit DFMUrlBaseEvent(const DUrl &url, const QObject *sender = 0);
    explicit DFMUrlBaseEvent(Type type, const DUrl &url, const QObject *sender = 0);

    inline DUrl url() const { return qvariant_cast<DUrl>(m_data);}
};

class DFMUrlListBaseEvent : public DFMEvent
{
public:
    explicit DFMUrlListBaseEvent(const DUrlList &list, const QObject *sender = 0);
    explicit DFMUrlListBaseEvent(Type type, const DUrlList &list, const QObject *sender = 0);

    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}
};

template<class T, typename... Args>
QSharedPointer<T> dMakeEventPointer(Args&&... args)
{
    return QSharedPointer<T>(new T(std::forward<Args>(args)...));
}

class DFMOpenFileEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenFileEvent(const DUrl &url, const QObject *sender = 0);
};

class DFMOpenFileByAppEvent : public DFMOpenFileEvent
{
public:
    explicit DFMOpenFileByAppEvent(const QString &appName, const DUrl &url, const QObject *sender = 0);

    QString appName() const;
};

class DFMCompressEvnet : public DFMUrlListBaseEvent
{
public:
    explicit DFMCompressEvnet(const DUrlList &list, const QObject *sender = 0);
};

class DFMDecompressEvnet : public DFMUrlListBaseEvent
{
public:
    explicit DFMDecompressEvnet(const DUrlList &list, const QObject *sender = 0);
};

class DFMDecompressHereEvnet : public DFMDecompressEvnet
{
public:
    explicit DFMDecompressHereEvnet(const DUrlList &list, const QObject *sender = 0);
};

class DFMWriteUrlsToClipboardEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMWriteUrlsToClipboardEvent(DFMGlobal::ClipboardAction action, const DUrlList &list, const QObject *sender = 0);

    DFMGlobal::ClipboardAction action() const;
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

class DFMDeleteEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMDeleteEvent(const DUrlList &list, const QObject *sender = 0);
};

class DFMMoveToTrashEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMMoveToTrashEvent(const DUrlList &list, const QObject *sender = 0);
};

class DFMRestoreFromTrashEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMRestoreFromTrashEvent(const DUrlList &list, const QObject *sender = 0);
};

class DFMPasteEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMPasteEvent(DFMGlobal::ClipboardAction action, const DUrl &targetUrl,
                           const DUrlList &list, const QObject *sender = 0);

    DFMGlobal::ClipboardAction action() const;
    DUrl targetUrl() const;

    DUrlList handleUrlList() const Q_DECL_OVERRIDE;
};

class DFMNewFolderEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMNewFolderEvent(const DUrl &url, const QObject *sender = 0);
};

class DFMNewFileEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMNewFileEvent(const DUrl &url, const QString &fileSuffix, const QObject *sender = 0);

    QString fileSuffix() const;
};

class DFMOpenFileLocation : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenFileLocation(const DUrl &url, const QObject *sender = 0);
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

class DFMFileShareEvnet : public DFMUrlBaseEvent
{
public:
    explicit DFMFileShareEvnet(const DUrl &url, const QString &name, bool isWritable = false, bool allowGuest = false, const QObject *sender = 0);

    QString name() const;
    bool isWritable() const;
    bool allowGuest() const;
};

class DFMCancelFileShareEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMCancelFileShareEvent(const DUrl &url, const QObject *sender = 0);
};

class DFMOpenInTerminalEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenInTerminalEvent(const DUrl &url, const QObject *sender = 0);
};

class DFMGetChildrensEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMGetChildrensEvent(const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  const QObject *sender = 0);
    explicit DFMGetChildrensEvent(const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, const QObject *sender = 0);

    QStringList nameFilters() const;
    QDir::Filters filters() const;
    QDirIterator::IteratorFlags flags() const;
};

Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDirIterator::IteratorFlags)

class DFMCreateDiriterator : public DFMGetChildrensEvent
{
public:
    explicit DFMCreateDiriterator(const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  const QObject *sender = 0);
    explicit DFMCreateDiriterator(const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, const QObject *sender = 0);
};

class DFMCreateGetChildrensJob : public DFMCreateDiriterator
{
public:
    explicit DFMCreateGetChildrensJob(const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags,
                                  const QObject *sender = 0);
    explicit DFMCreateGetChildrensJob(const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, const QObject *sender = 0);
};

class DFMCreateFileInfoEvnet : public DFMUrlBaseEvent
{
public:
    explicit DFMCreateFileInfoEvnet(const DUrl &url, const QObject *sender = 0);
};

class DFMCreateFileWatcherEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMCreateFileWatcherEvent(const DUrl &url, const QObject *sender = 0);
};

class DFMChangeCurrentUrlEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMChangeCurrentUrlEvent(const DUrl &url, const QWidget *window, const QObject *sender = 0);

    const QWidget *window() const;
};

class DFMOpenNewWindowEvent : public DFMUrlListBaseEvent
{
public:
    //! force为false时，如果当前已打开窗口中有currentUrl==url的窗口时时不会打开新的窗口，只会激活此窗口。
    explicit DFMOpenNewWindowEvent(const DUrlList &list, bool force = true, const QObject *sender = 0);

    bool force() const;
};

class DFMOpenUrlEvent : public DFMUrlListBaseEvent
{
public:
    enum DirOpenMode {
        OpenInCurrentWindow,
        OpenNewWindow,
        ForceOpenNewWindow
    };

    explicit DFMOpenUrlEvent(const DUrlList &list, DirOpenMode mode, const QObject *sender = 0);

    DirOpenMode dirOpenMode() const;
};
Q_DECLARE_METATYPE(DFMOpenUrlEvent::DirOpenMode)

class DFileMenu;
class DFMMenuActionEvent : public DFMEvent
{
public:
    explicit DFMMenuActionEvent(const DFileMenu *menu, const DUrl &currentUrl, const DUrlList &selectedUrls, DFMGlobal::MenuAction action, const QObject *sender = 0);

    const DFileMenu *menu() const;
    const DUrl currentUrl() const;
    const DUrlList selectedUrls() const;
    DFMGlobal::MenuAction action() const;
};

#endif // FMEVENT_H

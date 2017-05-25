#ifndef FMEVENT_H
#define FMEVENT_H

#include <QString>
#include <QSharedData>
#include <QMetaType>
#include <QEvent>
#include <QPointer>
#include <QDir>
#include <QDirIterator>
#include <QJsonObject>
#include <QJsonDocument>

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
        OpenNewTab,
        OpenUrl,
        MenuAction,
        Back,
        Forward,
        // user custom
        CustomBase = 1000                            // first user event id
    };

    explicit DFMEvent(const QObject *sender = 0);
    explicit DFMEvent(Type type, const QObject *sender);

    DFMEvent(const DFMEvent &other);

    virtual ~DFMEvent();

    DFMEvent &operator =(const DFMEvent &other);

    static Type nameToType(const QString &name);
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

    inline QVariantMap propertys() const
    { return m_propertys;}
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

    static const QSharedPointer<DFMEvent> fromJson(Type type, const QJsonObject &json);
    static const QSharedPointer<DFMEvent> fromJson(const QJsonObject &json);

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
    explicit DFMUrlBaseEvent(const QObject *sender, const DUrl &url);
    explicit DFMUrlBaseEvent(Type type, const QObject *sender, const DUrl &url);

    inline DUrl url() const { return qvariant_cast<DUrl>(m_data);}

    static QSharedPointer<DFMUrlBaseEvent> fromJson(Type type, const QJsonObject &json);
};

class DFMUrlListBaseEvent : public DFMEvent
{
public:
    explicit DFMUrlListBaseEvent();
    explicit DFMUrlListBaseEvent(const QObject *sender, const DUrlList &list);
    explicit DFMUrlListBaseEvent(Type type, const QObject *sender, const DUrlList &list);

    inline DUrlList urlList() const { return qvariant_cast<DUrlList>(m_data);}

    static QSharedPointer<DFMUrlListBaseEvent> fromJson(Type type, const QJsonObject &json);
};
Q_DECLARE_METATYPE(DFMUrlListBaseEvent)

template<class T, typename... Args>
QSharedPointer<T> dMakeEventPointer(Args&&... args)
{
    return QSharedPointer<T>(new T(std::forward<Args>(args)...));
}

class DFMOpenFileEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenFileEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMOpenFileEvent> fromJson(const QJsonObject &json);
};

class DFMOpenFileByAppEvent : public DFMOpenFileEvent
{
public:
    explicit DFMOpenFileByAppEvent(const QObject *sender, const QString &appName, const DUrl &url);

    QString appName() const;

    static QSharedPointer<DFMOpenFileByAppEvent> fromJson(const QJsonObject &json);
};

class DFMCompressEvnet : public DFMUrlListBaseEvent
{
public:
    explicit DFMCompressEvnet(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMCompressEvnet> fromJson(const QJsonObject &json);
};

class DFMDecompressEvnet : public DFMUrlListBaseEvent
{
public:
    explicit DFMDecompressEvnet(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMDecompressEvnet> fromJson(const QJsonObject &json);
};

class DFMDecompressHereEvnet : public DFMDecompressEvnet
{
public:
    explicit DFMDecompressHereEvnet(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMDecompressHereEvnet> fromJson(const QJsonObject &json);
};

class DFMWriteUrlsToClipboardEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMWriteUrlsToClipboardEvent(const QObject *sender, DFMGlobal::ClipboardAction action, const DUrlList &list);

    DFMGlobal::ClipboardAction action() const;

    static QSharedPointer<DFMWriteUrlsToClipboardEvent> fromJson(const QJsonObject &json);
};

class DFMRenameEvent : public DFMEvent
{
public:
    explicit DFMRenameEvent(const QObject *sender, const DUrl &from, const DUrl &to);

    inline DUrl fromUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).first;}
    inline DUrl toUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).second;}

    DUrlList handleUrlList() const Q_DECL_OVERRIDE;

    inline static QVariant makeData(const DUrl &from, const DUrl &to)
    { return QVariant::fromValue(QPair<DUrl, DUrl>(from, to));}

    static QSharedPointer<DFMRenameEvent> fromJson(const QJsonObject &json);
};

class DFMDeleteEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMDeleteEvent(const QObject *sender, const DUrlList &list, bool silent = false);

    bool silent() const;

    static QSharedPointer<DFMDeleteEvent> fromJson(const QJsonObject &json);
};

class DFMMoveToTrashEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMMoveToTrashEvent(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMMoveToTrashEvent> fromJson(const QJsonObject &json);
};

class DFMRestoreFromTrashEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMRestoreFromTrashEvent(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMRestoreFromTrashEvent> fromJson(const QJsonObject &json);
};

class DFMPasteEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMPasteEvent(const QObject *sender, DFMGlobal::ClipboardAction action,
                           const DUrl &targetUrl, const DUrlList &list);

    DFMGlobal::ClipboardAction action() const;
    DUrl targetUrl() const;

    DUrlList handleUrlList() const Q_DECL_OVERRIDE;

    static QSharedPointer<DFMPasteEvent> fromJson(const QJsonObject &json);
};

class DFMNewFolderEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMNewFolderEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMNewFolderEvent> fromJson(const QJsonObject &json);
};

class DFMNewFileEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMNewFileEvent(const QObject *sender, const DUrl &url, const QString &suffix);

    QString fileSuffix() const;

    static QSharedPointer<DFMNewFileEvent> fromJson(const QJsonObject &json);
};

class DFMOpenFileLocation : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenFileLocation(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMOpenFileLocation> fromJson(const QJsonObject &json);
};

class DFMCreateSymlinkEvent : public DFMEvent
{
public:
    explicit DFMCreateSymlinkEvent(const QObject *sender, const DUrl &fileUrl, const DUrl &toUrl);

    inline DUrl fileUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).first;}
    inline DUrl toUrl() const { return qvariant_cast<QPair<DUrl, DUrl>>(m_data).second;}

    DUrlList handleUrlList() const Q_DECL_OVERRIDE;

    inline static QVariant makeData(const DUrl &url, const DUrl &to)
    { return QVariant::fromValue(QPair<DUrl, DUrl>(url, to));}

    static QSharedPointer<DFMCreateSymlinkEvent> fromJson(const QJsonObject &json);
};

class DFMFileShareEvnet : public DFMUrlBaseEvent
{
public:
    explicit DFMFileShareEvnet(const QObject *sender, const DUrl &url, const QString &name, bool isWritable = false, bool allowGuest = false);

    QString name() const;
    bool isWritable() const;
    bool allowGuest() const;

    static QSharedPointer<DFMFileShareEvnet> fromJson(const QJsonObject &json);
};

class DFMCancelFileShareEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMCancelFileShareEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMCancelFileShareEvent> fromJson(const QJsonObject &json);
};

class DFMOpenInTerminalEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenInTerminalEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMOpenInTerminalEvent> fromJson(const QJsonObject &json);
};

class DFMGetChildrensEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMGetChildrensEvent(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags);
    explicit DFMGetChildrensEvent(const QObject *sender, const DUrl &url,
                                  const QStringList &nameFilters, QDir::Filters filters);

    QStringList nameFilters() const;
    QDir::Filters filters() const;
    QDirIterator::IteratorFlags flags() const;

    static QSharedPointer<DFMGetChildrensEvent> fromJson(const QJsonObject &json);
};

Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDirIterator::IteratorFlags)

class DFMCreateDiriterator : public DFMGetChildrensEvent
{
public:
    explicit DFMCreateDiriterator(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags);
    explicit DFMCreateDiriterator(const QObject *sender, const DUrl &url,
                                  const QStringList &nameFilters, QDir::Filters filters);

    static QSharedPointer<DFMCreateDiriterator> fromJson(const QJsonObject &json);
};

class DFMCreateGetChildrensJob : public DFMCreateDiriterator
{
public:
    explicit DFMCreateGetChildrensJob(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                      QDir::Filters filters, QDirIterator::IteratorFlags flags);
    explicit DFMCreateGetChildrensJob(const QObject *sender, const DUrl &url,
                                      const QStringList &nameFilters, QDir::Filters filters);

    static QSharedPointer<DFMCreateGetChildrensJob> fromJson(const QJsonObject &json);
};

class DFMCreateFileInfoEvnet : public DFMUrlBaseEvent
{
public:
    explicit DFMCreateFileInfoEvnet(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMCreateFileInfoEvnet> fromJson(const QJsonObject &json);
};

class DFMCreateFileWatcherEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMCreateFileWatcherEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMCreateFileWatcherEvent> fromJson(const QJsonObject &json);
};

class DFMChangeCurrentUrlEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMChangeCurrentUrlEvent(const QObject *sender, const DUrl &url, const QWidget *window);

    const QWidget *window() const;

    static QSharedPointer<DFMChangeCurrentUrlEvent> fromJson(const QJsonObject &json);
};

class DFMOpenNewWindowEvent : public DFMUrlListBaseEvent
{
public:
    //! force为false时，如果当前已打开窗口中有currentUrl==url的窗口时时不会打开新的窗口，只会激活此窗口。
    explicit DFMOpenNewWindowEvent(const QObject *sender, const DUrlList &list, bool force = true);

    bool force() const;

    static QSharedPointer<DFMOpenNewWindowEvent> fromJson(const QJsonObject &json);
};

class DFMOpenNewTabEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenNewTabEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMOpenNewTabEvent> fromJson(const QJsonObject &json);
};

class DFMOpenUrlEvent : public DFMUrlListBaseEvent
{
public:
    enum DirOpenMode {
        OpenInCurrentWindow,
        OpenNewWindow,
        ForceOpenNewWindow
    };

    explicit DFMOpenUrlEvent(const QObject *sender, const DUrlList &list, DirOpenMode mode);

    DirOpenMode dirOpenMode() const;

    static QSharedPointer<DFMOpenUrlEvent> fromJson(const QJsonObject &json);
};
Q_DECLARE_METATYPE(DFMOpenUrlEvent::DirOpenMode)

class DFileMenu;
class DFMMenuActionEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMMenuActionEvent(const QObject *sender, const DFileMenu *menu, const DUrl &currentUrl,
                                const DUrlList &selectedUrls, DFMGlobal::MenuAction action);

    const DFileMenu *menu() const;
    const DUrl currentUrl() const;
    const DUrlList selectedUrls() const;
    DFMGlobal::MenuAction action() const;

    static QSharedPointer<DFMMenuActionEvent> fromJson(const QJsonObject &json);
};

class DFMBackEvent : public DFMEvent
{
public:
    explicit DFMBackEvent(const QObject *sender);

    static QSharedPointer<DFMBackEvent> fromJson(const QJsonObject &json);
};

class DFMForwardEvent : public DFMEvent
{
public:
    explicit DFMForwardEvent(const QObject *sender);

    static QSharedPointer<DFMForwardEvent> fromJson(const QJsonObject &json);
};

#endif // FMEVENT_H

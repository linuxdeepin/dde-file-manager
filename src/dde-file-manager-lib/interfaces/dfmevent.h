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
#include <QModelIndex>

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
        Mkdir,
        TouchFile,
        OpenFileLocation,
        AddToBookmark,
        RemoveBookmark,
        CreateSymlink,
        FileShare,
        CancelFileShare,
        OpenInTerminal,
        GetChildrens,
        CreateFileInfo,
        CreateDiriterator,
        CreateGetChildrensJob,
        CreateFileWatcher,
        CreateFileDevice,
        CreateFileHandler,
        CreateStorageInfo,
        SetFileExtraProperties,
        SetPermission,

        // other
        ChangeCurrentUrl,
        OpenNewWindow,
        OpenNewTab,
        OpenUrl,
        MenuAction,
        Back,
        Forward,
        // save operator
        SaveOperator,
        // Ctrl+Z
        CleanSaveOperator,
        Revocation,

        ///###: tag protocol.
        Tag,
        Untag,
        ChangeTagColor,
        GetTagsThroughFiles,
        OpenFiles,  //fix 修改多文件选中右键打开只启动一次应用，传多个参数
        OpenFilesByApp,//fix 修改选择多个文件时，选择默认程序，只传了一个
        // user custom
        CustomBase = 1000                            // first user event id
    };

    explicit DFMEvent(const QObject *sender = nullptr);
    explicit DFMEvent(Type type, const QObject *sender);

    DFMEvent(const DFMEvent &other);

    virtual ~DFMEvent();

    DFMEvent &operator =(const DFMEvent &other);

    static Type nameToType(const QString &name);
    static QString typeToName(Type type);
    inline Type type() const
    {
        return static_cast<Type>(m_type);
    }
    inline void setType(Type type)
    {
        m_type = type;
    }
    inline QPointer<const QObject> sender() const
    {
        return m_sender;
    }
    inline void setSender(const QObject *sender)
    {
        m_sender = sender;
    }

    inline void setAccepted(bool accepted)
    {
        m_accept = accepted;
    }
    inline bool isAccepted() const
    {
        return m_accept;
    }

    inline void accept()
    {
        m_accept = true;
    }
    inline void ignore()
    {
        m_accept = false;
    }

    static quint64 windowIdByQObject(const QObject *object);
    quint64 windowId() const;
    void setWindowId(quint64 id);

    //! 在DFileServices中通过此url列表来获取处理此事件的Controller
    virtual DUrlList handleUrlList() const;
    inline void setData(const QVariant &data)
    {
        m_data = data;
    }
    template<typename T>
    void setData(T &&data)
    {
        m_data = QVariant::fromValue(std::forward<T>(data));
    }
    inline QVariant data() const
    {
        return m_data;
    }
    template<typename T>
    inline T data() const
    {
        return qvariant_cast<T>(m_data);
    }
    //! 设置剪切文件在回收站的路径
    inline void setCutData(const QVariant &data)
    {
        m_cutData = data;
    }
    template<typename T>
    void setCutData(T &&data)
    {
        m_cutData = QVariant::fromValue(std::forward<T>(data));
    }
    inline QVariant cutData() const
    {
        return m_cutData;
    }
    template<typename T>
    inline T cutData() const
    {
        return qvariant_cast<T>(m_cutData);
    }

//    template<typename T>
//    inline T data()const
//    {
//        return m_data.value<T>();
//    }

    inline DUrl fileUrl() const
    {
        return data<DUrl>();
    }
    inline DUrlList fileUrlList() const
    {
        return data<DUrlList>();
    }

    inline QVariantMap properties() const
    {
        return m_properties;
    }
    inline QVariant property(const QString &name, const QVariant &defaultValue = QVariant()) const
    {
        return m_properties.value(name, defaultValue);
    }
    template<typename T>
    T property(const QString &name, T &&defaultValue) const
    {
        return qvariant_cast<T>(m_properties.value(name, QVariant::fromValue(std::forward<T>(defaultValue))));
    }
    template<typename T>
    T property(const QString &name) const
    {
        return qvariant_cast<T>(property(name));
    }
    inline void setProperty(const QString &name, const QVariant &value)
    {
        m_properties[name] = value;
    }
    template<typename T>
    void setProperty(const QString &name, T &&value)
    {
        m_properties[name] = QVariant::fromValue(std::forward<T>(value));
    }

    static const QSharedPointer<DFMEvent> fromJson(Type type, const QJsonObject &json);
    static const QSharedPointer<DFMEvent> fromJson(const QJsonObject &json);

protected:
    ushort m_type;
    QVariant m_data;
    QVariant m_cutData; //! 剪切原路径
    QVariantMap m_properties;
    QPointer<const QObject> m_sender;

private:
    ushort m_accept : 1;
    quint64 m_id;
};

Q_DECLARE_METATYPE(DFMEvent)
Q_DECLARE_METATYPE(QSharedPointer<DFMEvent>)

template<typename T>
const T dfmevent_cast(const DFMEvent &event)
{
    if (!QString(typeid(T).name()).contains(DFMEvent::typeToName(event.type()))) {
        DFMEvent e;

        return *reinterpret_cast<T *>(&e);
    }

    return *reinterpret_cast<const T *>(&event);
}

template<typename T>
T dfmevent_cast(DFMEvent &event)
{
    if (!QString(typeid(T).name()).contains(DFMEvent::typeToName(event.type()))) {
        DFMEvent e;

        return *reinterpret_cast<T *>(&e);
    }

    return *reinterpret_cast<T *>(&event);
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DFMEvent &info);
QT_END_NAMESPACE

class DFMUrlBaseEvent : public DFMEvent
{
public:
    explicit DFMUrlBaseEvent(const QObject *sender, const DUrl &url);
    explicit DFMUrlBaseEvent(Type type, const QObject *sender, const DUrl &url);

    inline DUrl url() const
    {
        return qvariant_cast<DUrl>(m_data);
    }

    static QSharedPointer<DFMUrlBaseEvent> fromJson(Type type, const QJsonObject &json);
};

class DFMUrlListBaseEvent : public DFMEvent
{
public:
    explicit DFMUrlListBaseEvent();
    explicit DFMUrlListBaseEvent(const QObject *sender, const DUrlList &list);
    explicit DFMUrlListBaseEvent(Type type, const QObject *sender, const DUrlList &list);

    inline DUrlList urlList() const
    {
        return qvariant_cast<DUrlList>(m_data);
    }

    static QSharedPointer<DFMUrlListBaseEvent> fromJson(Type type, const QJsonObject &json);
};
Q_DECLARE_METATYPE(DFMUrlListBaseEvent)

template<class T, typename... Args>
QSharedPointer<T> dMakeEventPointer(Args &&... args)
{
    return QSharedPointer<T>(new T(std::forward<Args>(args)...));
}

class DFMOpenFileEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenFileEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMOpenFileEvent> fromJson(const QJsonObject &json);
};

class DFMOpenFilesEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMOpenFilesEvent(const QObject *sender, const DUrlList &list, const bool isEnter = false);

    bool isEnter() const;

    static QSharedPointer<DFMOpenFilesEvent> fromJson(const QJsonObject &json);
};

class DFMOpenFileByAppEvent : public DFMOpenFileEvent
{
public:
    explicit DFMOpenFileByAppEvent(const QObject *sender, const QString &appName, const DUrl &url);

    QString appName() const;

    static QSharedPointer<DFMOpenFileByAppEvent> fromJson(const QJsonObject &json);
};

class DFMOpenFilesByAppEvent : public DFMOpenFilesEvent
{
public:
    explicit DFMOpenFilesByAppEvent(const QObject *sender, const QString &appName, const QList<DUrl> &url, const bool isEnter = false);

    QString appName() const;

    static QSharedPointer<DFMOpenFilesByAppEvent> fromJson(const QJsonObject &json);
};

class DFMCompressEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMCompressEvent(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMCompressEvent> fromJson(const QJsonObject &json);
};

class DFMDecompressEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMDecompressEvent(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMDecompressEvent> fromJson(const QJsonObject &json);
};

class DFMDecompressHereEvent : public DFMDecompressEvent
{
public:
    explicit DFMDecompressHereEvent(const QObject *sender, const DUrlList &list);

    static QSharedPointer<DFMDecompressHereEvent> fromJson(const QJsonObject &json);
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
    explicit DFMRenameEvent(const QObject *sender, const DUrl &from, const DUrl &to, const bool silent = false);

    inline DUrl fromUrl() const
    {
        return qvariant_cast<QPair<DUrl, DUrl>>(m_data).first;
    }
    inline DUrl toUrl() const
    {
        return qvariant_cast<QPair<DUrl, DUrl>>(m_data).second;
    }

    DUrlList handleUrlList() const override;

    bool silent() const;

    inline static QVariant makeData(const DUrl &from, const DUrl &to)
    {
        return QVariant::fromValue(QPair<DUrl, DUrl>(from, to));
    }

    static QSharedPointer<DFMRenameEvent> fromJson(const QJsonObject &json);
};

class DFMDeleteEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMDeleteEvent(const QObject *sender, const DUrlList &list, bool silent = false, bool force = false);

    bool silent() const;
    bool force() const;

    static QSharedPointer<DFMDeleteEvent> fromJson(const QJsonObject &json);
};

class DFMMoveToTrashEvent : public DFMUrlListBaseEvent
{
public:
    //! 新增剪切回收站路径cutList
    explicit DFMMoveToTrashEvent(const QObject *sender, const DUrlList &list, bool silent = false, const DUrlList &cutList = DUrlList());

    bool silent() const;
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

    DUrlList handleUrlList() const override;

    static QSharedPointer<DFMPasteEvent> fromJson(const QJsonObject &json);
};

class DFMMkdirEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMMkdirEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMMkdirEvent> fromJson(const QJsonObject &json);
};

class DFMTouchFileEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMTouchFileEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMTouchFileEvent> fromJson(const QJsonObject &json);
};

class DFMOpenFileLocation : public DFMUrlBaseEvent
{
public:
    explicit DFMOpenFileLocation(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMOpenFileLocation> fromJson(const QJsonObject &json);
};

class DFMAddToBookmarkEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMAddToBookmarkEvent(const QObject *sender, const DUrl &url);
};

class DFMRemoveBookmarkEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMRemoveBookmarkEvent(const QObject *sender, const DUrl &url);
};

class DFMCreateSymlinkEvent : public DFMEvent
{
public:
    explicit DFMCreateSymlinkEvent(const QObject *sender, const DUrl &fileUrl, const DUrl &toUrl, bool force = false);

    bool force() const;

    inline DUrl fileUrl() const
    {
        return qvariant_cast<QPair<DUrl, DUrl>>(m_data).first;
    }
    inline DUrl toUrl() const
    {
        return qvariant_cast<QPair<DUrl, DUrl>>(m_data).second;
    }

    DUrlList handleUrlList() const override;

    inline static QVariant makeData(const DUrl &url, const DUrl &to)
    {
        return QVariant::fromValue(QPair<DUrl, DUrl>(url, to));
    }

    static QSharedPointer<DFMCreateSymlinkEvent> fromJson(const QJsonObject &json);
};

class DFMFileShareEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMFileShareEvent(const QObject *sender, const DUrl &url, const QString &name, bool isWritable = false, bool allowGuest = false);

    QString name() const;
    bool isWritable() const;
    bool allowGuest() const;

    static QSharedPointer<DFMFileShareEvent> fromJson(const QJsonObject &json);
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
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags, bool slient = false, bool canconst = false);
    explicit DFMGetChildrensEvent(const QObject *sender, const DUrl &url,
                                  const QStringList &nameFilters, QDir::Filters filters, bool slient = false, bool canconst = false);

    QStringList nameFilters() const;
    QDir::Filters filters() const;
    QDirIterator::IteratorFlags flags() const;
    bool silent() const;
    bool canconst() const;

    static QSharedPointer<DFMGetChildrensEvent> fromJson(const QJsonObject &json);
};

Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDirIterator::IteratorFlags)

class DFMCreateDiriterator : public DFMGetChildrensEvent
{
public:
    explicit DFMCreateDiriterator(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                  QDir::Filters filters, QDirIterator::IteratorFlags flags, bool slient = false, bool isgvfs = false);
    explicit DFMCreateDiriterator(const QObject *sender, const DUrl &url,
                                  const QStringList &nameFilters, QDir::Filters filters, bool slient = false, bool isgvfs = false);
    bool isGvfsFile() const;
    static QSharedPointer<DFMCreateDiriterator> fromJson(const QJsonObject &json);
};

class DFMCreateGetChildrensJob : public DFMCreateDiriterator
{
public:
    explicit DFMCreateGetChildrensJob(const QObject *sender, const DUrl &url, const QStringList &nameFilters,
                                      QDir::Filters filters, QDirIterator::IteratorFlags flags, bool silent = false, const bool isgvfsfile = false);
    explicit DFMCreateGetChildrensJob(const QObject *sender, const DUrl &url,
                                      const QStringList &nameFilters, QDir::Filters filters, bool silent = false, const bool isgvfsfile = false);
    static QSharedPointer<DFMCreateGetChildrensJob> fromJson(const QJsonObject &json);
};

class DFMCreateFileInfoEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMCreateFileInfoEvent(const QObject *sender, const DUrl &url);

    static QSharedPointer<DFMCreateFileInfoEvent> fromJson(const QJsonObject &json);
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

    explicit DFMOpenUrlEvent(const QObject *sender, const DUrlList &list, DirOpenMode mode, const bool isEnter = false);

    DirOpenMode dirOpenMode() const;
    bool isEnter() const;

    static QSharedPointer<DFMOpenUrlEvent> fromJson(const QJsonObject &json);
};
Q_DECLARE_METATYPE(DFMOpenUrlEvent::DirOpenMode)

class DFileMenu;
class DFMMenuActionEvent : public DFMUrlListBaseEvent
{
public:
    explicit DFMMenuActionEvent(const QObject *sender, const DFileMenu *menu, const DUrl &currentUrl,
                                const DUrlList &selectedUrls, DFMGlobal::MenuAction action, const QModelIndex &index = QModelIndex());

    const DFileMenu *menu() const;
    const DUrl currentUrl() const;
    const DUrlList selectedUrls() const;
    DFMGlobal::MenuAction action() const;
    const QModelIndex clickedIndex() const; //当前右键点击的index

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


class DFMSaveOperatorEvent : public DFMEvent
{
public:
    explicit DFMSaveOperatorEvent(const QSharedPointer<DFMEvent> &iniaiator, const QSharedPointer<DFMEvent> &event, bool async = false);
    // is split type
    explicit DFMSaveOperatorEvent();

    QSharedPointer<DFMEvent> iniaiator() const;
    QSharedPointer<DFMEvent> event() const;
    bool async() const;
    // 用于实现可回退一系列操作的分隔符, 遇到此类事件时，会一直重复执行撤销动作，直到再次遇到此分隔符或事件栈为空
    bool split() const;
};

class DFMCleanSaveOperatorEvent : public DFMEvent
{
public:
    explicit DFMCleanSaveOperatorEvent(const QObject *sender);
};

class DFMRevocationEvent : public DFMEvent
{
public:
    explicit DFMRevocationEvent(const QObject *sender);
};

class DFMSetPermissionEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMSetPermissionEvent(const QObject *sender, const DUrl &url, const QFileDevice::Permissions &permissions);
    QFileDevice::Permissions permissions() const;
};

class DFMSetFileTagsEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMSetFileTagsEvent(const QObject *sender, const DUrl &url, const QList<QString> &tags);

    static QSharedPointer<DFMSetFileTagsEvent> fromJson(const QJsonObject &json);

    QList<QString> tags() const;
};

class DFMRemoveTagsOfFileEvent : public DFMUrlBaseEvent
{
public:
    explicit DFMRemoveTagsOfFileEvent(const QObject *sender, const DUrl &url, const QList<QString> &tags);

    static QSharedPointer<DFMRemoveTagsOfFileEvent> fromJson(const QJsonObject &json);

    QList<QString> tags() const;
};


class DFMChangeTagColorEvent : public DFMEvent
{
public:
    explicit DFMChangeTagColorEvent(const QObject *sender, const QColor &color, const DUrl &tagUrl);
    DFMChangeTagColorEvent(const DFMChangeTagColorEvent &other) = delete;
    DFMChangeTagColorEvent &operator=(const DFMChangeTagColorEvent &other) = delete;

    static QSharedPointer<DFMChangeTagColorEvent> fromJson(const QJsonObject &json);

    QColor m_newColorForTag{};
    DUrl m_tagUrl{};
};


class DFMGetTagsThroughFilesEvent final : public DFMUrlListBaseEvent
{
public:
    explicit DFMGetTagsThroughFilesEvent(const QObject *sender, const QList<DUrl> &files);

    static QSharedPointer<DFMGetTagsThroughFilesEvent> fromJson(const QJsonObject &json);
};

class DFMSetFileExtraProperties : public DFMUrlBaseEvent
{
public:
    explicit DFMSetFileExtraProperties(const QObject *sender, const DUrl &url, const QVariantHash &ep);

    QVariantHash extraProperties() const;
};

#endif // FMEVENT_H

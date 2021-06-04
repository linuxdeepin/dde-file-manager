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

#ifndef DFMGLOBAL_H
#define DFMGLOBAL_H

#include "durl.h"

#include <QObject>
#include <QFontMetrics>
#include <QTextOption>
#include <QTextLayout>
#include <QCoreApplication>
#include <QSemaphore>
#include <QThread>
#include <QUrl>
#include <QDebug>
#include <QtWidgets/QLabel>

#include <functional>
#include <cmath>
#include <iostream>

#define CEIL(x) (static_cast<int>(std::ceil(x)))
#define FLOOR(x) (static_cast<int>(std::floor(x)))

// begin file item global define
#define TEXT_LINE_HEIGHT 18
#define MAX_THREAD_COUNT 1000
#define MAX_FILE_NAME_CHAR_COUNT 255
#define DDE_TRASH_ID "dde-trash"
#define DDE_COMPUTER_ID "dde-computer"
#define REVOCATION_TIMES 2
#define MIME_USER_ID "userID_for_MoveAction"

#define ASYN_CALL(Fun, Code, captured...) {\
        QDBusPendingCallWatcher * watcher = new QDBusPendingCallWatcher(Fun);\
        auto onFinished = [watcher, captured]{\
                                              const QVariantList & args = watcher->reply().arguments();\
                                              Q_UNUSED(args);\
                                              Code;\
                                              watcher->deleteLater();\
                                             };\
        if(watcher->isFinished()) onFinished();\
        else QObject::connect(watcher, &QDBusPendingCallWatcher::finished, watcher, onFinished);}

#if QT_VERSION >= 0x050500
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
        QTimer::singleShot(Time, [captured] {Code;});\
    }
#else
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
        QTimer *timer = new QTimer;\
        timer->setSingleShot(true);\
        timer->setInterval(Time);\
        timer->moveToThread(qApp->thread());\
        QObject::connect(timer, &QTimer::timeout, timer, [timer, captured] {\
                                                                            timer->deleteLater();\
                                                                            Code;\
                                                                           });\
        if (QThread::currentThread() == qApp->thread()) timer->start();\
        else QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);\
    }
#endif

#define TIMER_SINGLESHOT_CONNECT_TYPE(Obj, Time, Code, ConnectType, captured...){ \
        QTimer *timer = new QTimer;\
        timer->setSingleShot(true);\
        timer->setInterval(Time);\
        timer->moveToThread(qApp->thread());\
        QObject::connect(timer, &QTimer::timeout, Obj, [timer, captured] {\
                                                                          timer->deleteLater();\
                                                                          Code;\
                                                                         }, ConnectType);\
        if (QThread::currentThread() == qApp->thread()) timer->start();\
        else QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);\
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define TIMER_SINGLESHOT_OBJECT(Obj, Time, Code, captured...)\
    TIMER_SINGLESHOT_CONNECT_TYPE(Obj, Time, Code, Qt::AutoConnection, captured)
#else
#define TIMER_SINGLESHOT_OBJECT(Obj, Time, Code, captured...)\
    QTimer::singleShot(Time, Obj, [captured]{Code;});
#endif

#define ASYN_CALL_SLOT(obj, fun, args...) \
    TIMER_SINGLESHOT_CONNECT_TYPE(obj, 0, {obj->fun(args);}, Qt::QueuedConnection, obj, args)

#ifdef QT_STRINGIFY
#undef QT_STRINGIFY
#endif
#define QT_STRINGIFY(x...) #x

#define DFM_NAMESPACE dde_file_manager
#define DFM_BEGIN_NAMESPACE namespace DFM_NAMESPACE {
#define DFM_END_NAMESPACE }
#define DFM_USE_NAMESPACE using namespace DFM_NAMESPACE;

class DFMGlobal : public QObject
{
    Q_OBJECT

public:
    enum ClipboardAction {
        CutAction,
        CopyAction,
        UnknowAction
    };

    enum Icon {
        LinkIcon,
        LockIcon,
        UnreadableIcon,
        ShareIcon
    };

    enum MenuExtension {
        SingleFile,
        MultiFiles,
        SingleDir,
        MultiDirs,
        MultiFileDirs,
        EmptyArea,
        UnknowMenuExtension
    };

    Q_ENUM(MenuExtension)

    enum MenuAction {
        Unknow,
        Open,
        OpenDisk,
        OpenInNewWindow,
        OpenInNewTab,
        OpenDiskInNewWindow,
        OpenDiskInNewTab,
        OpenAsAdmin,
        OpenWith,
        OpenWithCustom,
        OpenFileLocation,
        Compress,
        Decompress,
        DecompressHere,
        Cut,
        Copy,
        Paste,
        Rename,
        BookmarkRename,
        BookmarkRemove,
        CreateSymlink,
        SendToDesktop,
        SendToRemovableDisk,
        SendToBluetooth,
        AddToBookMark,
        Delete,
        Property,
        NewFolder,
        NewWindow,
        SelectAll,
        Separator,
        ClearRecent,
        ClearTrash,
        DisplayAs, /// sub menu
        SortBy, /// sub menu
        NewDocument, /// sub menu
        NewWord, /// sub menu
        NewExcel, /// sub menu
        NewPowerpoint, /// sub menu
        NewText, /// sub menu
        OpenInTerminal,
        Restore,
        RestoreAll,
        CompleteDeletion,
        Mount,
        Unmount,
        Eject,
        SafelyRemoveDrive,
        Name,
        Size,
        Type,
        CreatedDate,
        LastModifiedDate,
        LastRead,
        DeletionDate,
        SourcePath,
        AbsolutePath,
        Settings,
        Exit,
        IconView,
        ListView,
        ExtendView,
        SetAsWallpaper,
        ForgetPassword,
        Share,
        UnShare,
        SetUserSharePassword,
        FormatDevice,
        OpticalBlank,
        Vault,
        ConnectToServer,

        ///###: tag protocol.
        TagInfo,
        TagFilesUseColor,
        ChangeTagColor,
        DeleteTags,
        RenameTag,

#ifdef SW_LABEL
        SetLabel,
        ViewLabel,
        EditLabel,
        PrivateFileToPublic,
#endif

        // recent
        RemoveFromRecent,

        MountImage,
        StageFileForBurning,

        // Vault
        LockNow,
        AutoLock,
        Never,
        FiveMinutes,
        TenMinutes,
        TwentyMinutes,
        DeleteVault,

        UnLock,
        UnLockByKey,

        UserMenuAction = Unknow + 2000
    };

    Q_ENUM(MenuAction)

    static DFMGlobal *instance();

    static QStringList PluginLibraryPaths;
    static bool IsFileManagerDiloagProcess;
    static QAtomicInteger<bool> IsInitAppOver;
    static QAtomicInteger<bool> IsAppQuiting;

    static QString organizationName();
    static QString applicationName();
    // libdde-file-manager version
    static QString applicationVersion();
    static bool installTranslator();

    static void setUrlsToClipboard(const QList<QUrl> &list, DFMGlobal::ClipboardAction action, QMimeData *mimeData = nullptr);
    static const QList<QUrl> fetchUrlsFromClipboard();
    static DFMGlobal::ClipboardAction fetchClipboardAction();
    static void clearClipboard();
    static void clearTrash();

    static void addPluginLibraryPath(const QString &path);
    static void addPluginLibraryPaths(const QStringList &paths);

    static void autoLoadDefaultPlugins();

    /* Global instance initialization */
    static void initPluginManager();
    static void initSearchHistoryManager();
    static void initBookmarkManager();
    static void initFileMenuManager();
    static void initFileSiganlManager();
    static void initDialogManager();
    static void initAppcontroller();
    static void initFileService();
    static void initDeviceListener();
    static void initMimesAppsManager();
    static void initSystemPathManager();
    static void initMimeTypeDisplayManager();
    static void initNetworkManager();
    static void initGvfsMountManager();
    static void initSecretManager();
    static void initUserShareManager();
    static void initOperatorRevocation();
    static void initTagManagerConnect();
    static void initThumbnailConnection();
    static void initBluetoothManager();
    static void initRootFileManager();

    static QString getUser();
    static int getUserId();
    static bool isRootUser();
    static bool isServerSys();
    static bool isDesktopSys();
    static bool isOpenAsAdmin();
    static bool isDeveloperMode();

    //check if is trash/computer desktop file containing Deepin_id of dde-trash/dde-computer
    static bool isDesktopFile(const DUrl &url);
    static bool isTrashDesktopFile(const DUrl &url);
    static bool isComputerDesktopFile(const DUrl &url);

    //check the unique trash/computer dekstop file on desktop
    static bool isTrashDesktopFileUrl(const DUrl &url);
    static bool isComputerDesktopFileUrl(const DUrl &url);

    /**
     * @brief isWayLand 判断是否是wayland
     * @return true or false
     */
    static bool isWayLand();

    QList<QUrl> clipboardFileUrlList() const;
    QList<quint64> clipboardFileInodeList() const;
    ClipboardAction clipboardAction() const;
    QIcon standardIcon(Icon iconType) const;

    static QString wordWrapText(const QString &text, qreal width,
                                QTextOption::WrapMode wrapMode,
                                const QFont &font,
                                qreal lineHeight,
                                qreal *height = nullptr);

    static QString elideText(const QString &text, const QSizeF &size,
                             QTextOption::WrapMode wordWrap,
                             const QFont &font,
                             Qt::TextElideMode mode,
                             qreal lineHeight,
                             qreal flags = 0);

    static void wordWrapText(QTextLayout *layout, qreal width,
                             QTextOption::WrapMode wrapMode,
                             qreal lineHeight,
                             QStringList *lines = nullptr);

    static void elideText(QTextLayout *layout, const QSizeF &size,
                          QTextOption::WrapMode wordWrap,
                          Qt::TextElideMode mode, qreal lineHeight,
                          int flags = 0, QStringList *lines = nullptr,
                          QPainter *painter = nullptr, QPointF offset = QPoint(0, 0),
                          const QColor &shadowColor = QColor(),
                          const QPointF &shadowOffset = QPointF(0, 1),
                          const QBrush &background = QBrush(Qt::NoBrush),
                          qreal backgroundRadius = 4,
                          QList<QRectF> *boundingRegion = nullptr);


    /**
     * @brief setToolTip 设置tooltip显示
     * @param label 需要显示的控件
     * @param bAlwaysShow 是否总是显示
     */
    static void setToolTip(QLabel* label, bool bAlwaysShow = false);

    /**
     * @brief showAlertMessage 显示气泡提示信息
     * @param globalPoint 气泡框展示的全局坐标
     * @param backgroundColor 气泡框的背景色
     * @param text 气泡框展示的文字
     * @param duration  显示时间，默认3秒
     */
    static void showAlertMessage(QPoint globalPoint,
                                 const QColor &backgroundColor,
                                 const QString &text,
                                 int duration = 3000);

    static QString toPinyin(const QString &text);
    static bool startWithHanzi(const QString &text);
    template<typename T>
    static bool startWithHanzi(T)
    { return false;}
    /**
     * @brief startWithSymbol 判断字符串是由符号开头
     * @param text
     * @return
     */
    static bool startWithSymbol(const QString &text);

    static bool keyShiftIsPressed();
    static bool keyCtrlIsPressed();
    static bool fileNameCorrection(const QString &filePath);
    static bool fileNameCorrection(const QByteArray &filePath);


    static void showMultiFilesRenameDialog(const QList<DUrl> &selectedFiles);

    static void showFilePreviewDialog(const DUrlList &selectUrls, const DUrlList &entryUrls);

    ///###: this function is special, if select files is more than 16 show a dialog for multiple files.
    static void showPropertyDialog(QObject *const sender, const QList<DUrl> &selectedFiles);

    static QString toUnicode(const QByteArray &data, const QString &fileName = QString());
    static QString cutString(const QString &text, int dataByteSize, const QTextCodec *codec);

    ///###: this function detect what the charset of str is.
    static QByteArray detectCharset(const QByteArray &data, const QString &fileName = QString{});
    static QString preprocessingFileName(QString name);

    static void setMimeDataUserID(QMimeData *mime);
    static bool isMimeDatafromCurrentUser(const QMimeData *mime);
    static void setInitAppOver();
    static bool isInitAppOver();
    static void setAppQuiting();
    static bool isAppQuiting();
signals:
    void clipboardDataChanged();

protected:
    DFMGlobal();

private:
    void onClipboardDataChanged();
    static void refreshPlugins();
};

Q_DECLARE_METATYPE(DFMGlobal::ClipboardAction)
Q_DECLARE_METATYPE(DFMGlobal::MenuAction)

namespace DThreadUtil {
typedef std::function<void()> FunctionType;

class FunctionCallProxy : public QObject
{
    Q_OBJECT
public:
    explicit FunctionCallProxy(QThread *thread);

signals:
    void callInLiveThread(FunctionType *func);
};

template <typename ReturnType>
class _TMP
{
public:
    template <typename Fun, typename... Args>
    static ReturnType runInThread(QSemaphore *s, QThread *thread, Fun fun, Args &&... args)
    {
        if (QThread::currentThread() == thread)
            return fun(std::forward<Args>(args)...);

        ReturnType result;
        FunctionType proxyFun = [&]() {
            result = fun(std::forward<Args>(args)...);
            s->release();
        };

        FunctionCallProxy proxy(thread);
        proxy.moveToThread(thread);

        if (thread->loopLevel() <= 0) {
            qWarning() << thread << ", the thread no event loop";
        }

        proxy.callInLiveThread(&proxyFun);
        s->acquire();

        return result;
    }
};
template <>
class _TMP<void>
{
public:
    template <typename Fun, typename... Args>
    static void runInThread(QSemaphore *s, QThread *thread, Fun fun, Args &&... args)
    {
        if (QThread::currentThread() == thread)
            return fun(std::forward<Args>(args)...);

        FunctionType proxyFun = [&]() {
            fun(std::forward<Args>(args)...);
            s->release();
        };

        FunctionCallProxy proxy(thread);
        proxy.moveToThread(thread);

        if (thread->loopLevel() <= 0) {
            qWarning() << thread << ", the thread no event loop";
        }

        proxy.callInLiveThread(&proxyFun);
        s->acquire();
    }
};

template <typename Fun, typename... Args>
auto runInThread(QSemaphore *s, QThread *thread, Fun fun, Args &&... args) -> decltype(fun(args...))
{
    return _TMP<decltype(fun(args...))>::runInThread(s, thread, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
typename QtPrivate::FunctionPointer<Fun>::ReturnType runInThread(QSemaphore *s, QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args &&... args)
{
    return _TMP<typename QtPrivate::FunctionPointer<Fun>::ReturnType>::runInThread(s, thread, [&] {
        return (obj->*fun)(std::forward<Args>(args)...);
    });
}

template <typename Fun, typename... Args>
auto runInThread(QThread *thread, Fun fun, Args &&... args) -> decltype(fun(args...))
{
    QSemaphore s;

    return runInThread(&s, thread, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
typename QtPrivate::FunctionPointer<Fun>::ReturnType runInThread(QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args &&... args)
{
    QSemaphore s;

    return runInThread(&s, thread, obj, fun, std::forward<Args>(args)...);
}

template <typename Fun, typename... Args>
auto runInMainThread(Fun fun, Args &&... args) -> decltype(fun(args...))
{
    if (!QCoreApplication::instance()) {
        return fun(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
typename QtPrivate::FunctionPointer<Fun>::ReturnType runInMainThread(typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args &&... args)
{
    if (!QCoreApplication::instance()) {
        return (obj->*fun)(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), obj, fun, std::forward<Args>(args)...);
}
}

// namespace dfm_util: 通用的工具
namespace dfm_util {
template <typename T>
// 工具函数：判断一个值是否在一个集合中
inline bool isContains(const T &source)
{
    (void)source;
    return false;
}

template <typename T, typename ...Args>
inline bool isContains(const T &source, const T &cmp, const Args &...args)
{
    if (source == cmp)
        return true;
    else
        return isContains(source, args...);
}
}


#endif // DFMGLOBAL_H

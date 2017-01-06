#ifndef DFMGLOBAL_H
#define DFMGLOBAL_H

#include <QObject>
#include <QFontMetrics>
#include <QTextOption>
#include <QTextLayout>

// begin file item global define
#define TEXT_LINE_HEIGHT 18
#define MAX_THREAD_COUNT 1000
#define MAX_FILE_NAME_CHAR_COUNT 255

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

#define DFM_BEGIN_NAMESPACE namespace dde_file_manager {
#define DFM_END_NAMESPACE }
#define DFM_USE_NAMESPACE using namespace dde_file_manager;

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
        Remove,
        CreateSymlink,
        SendToDesktop,
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
        Name,
        Size,
        Type,
        CreatedDate,
        LastModifiedDate,
        DeletionDate,
        SourcePath,
        AbsolutePath,
        Settings,
        Help,
        About,
        Exit,
        IconView,
        ListView,
        ExtendView,
        SetAsWallpaper,
        ForgetPassword,
        Share,
        UnShare,
        SetUserSharePassword,
        Unknow
    };

    Q_ENUM(MenuAction)

    static DFMGlobal *instance();

    static QStringList PluginLibraryPaths;
    static QStringList MenuExtensionPaths;

    static QString organizationName();
    static QString applicationName();
    // libdde-file-manager version
    static QString applicationVersion();
    static bool installTranslator();

    static void setUrlsToClipboard(const QList<QUrl> &list, DFMGlobal::ClipboardAction action);
    static void clearClipboard();

    static void addPluginLibraryPath(const QString& path);
    static void addPluginLibraryPaths(const QStringList& paths);
    static void addMenuExtensionPath(const QString& path);
    static void addMenuExtensionPaths(const QStringList& paths);

    static void autoLoadDefaultPlugins();
    static void autoLoadDefaultMenuExtensions();

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
    static void initGvfsMountClient();
    static void initSecretManager();
    static void initUserShareManager();

    QList<QUrl> clipboardFileUrlList() const;
    ClipboardAction clipboardAction() const;
    QIcon standardIcon(Icon iconType) const;

    static QString wordWrapText(const QString &text, int width,
                                QTextOption::WrapMode wrapMode,
                                const QFont &font = QFont(),
                                int lineHeight = TEXT_LINE_HEIGHT,
                                int *height = 0);

    static QString elideText(const QString &text, const QSize &size,
                             const QFontMetrics &fontMetrics,
                             QTextOption::WrapMode wordWrap,
                             Qt::TextElideMode mode,
                             int lineHeight = TEXT_LINE_HEIGHT,
                             int flags = 0);

    static QString toPinyin(const QString &text);
    static bool startWithHanzi(const QString &text);
    template<typename T>
    static bool startWithHanzi(T)
    { return false;}

    static bool keyShiftIsPressed();
    static bool keyCtrlIsPressed();
    static bool fileNameCorrection(const QString &filePath);
    static bool fileNameCorrection(const QByteArray &filePath);

signals:
    void clipboardDataChanged();

protected:
    DFMGlobal();

private:
    void onClipboardDataChanged();
    static void refreshPlugins();
};

#endif // DFMGLOBAL_H

#ifndef GLOBAL_H
#define GLOBAL_H

#include "widgets/singleton.h"

#include "utils/utils.h"

#include "../views/filemenumanager.h"

#include "../dialogs/dialogmanager.h"

#include "../controllers/fileservices.h"
#include "../controllers/pathmanager.h"
#include "../models/abstractfileinfo.h"
#include "../../deviceinfo/devicelistener.h"
#include "../../deviceinfo/udisklistener.h"
#include "../shutil/mimesappsmanager.h"
#include "../shutil/mimetypedisplaymanager.h"
#include "../shutil/iconprovider.h"
#include "../controllers/appcontroller.h"
#include "../shutil/thumbnailmanager.h"

#include <QFontMetrics>
#include <QTextOption>
#include <QTextLayout>

#define fileManagerApp Singleton<FileManagerApp>::instance()
#define searchHistoryManager  Singleton<SearchHistroyManager>::instance()
#define bookmarkManager  Singleton<BookMarkManager>::instance()
#define trashManager  Singleton<TrashManager>::instance()
#define fileMenuManger  Singleton<FileMenuManager>::instance()
#define fileSignalManager Singleton<FileSignalManager>::instance()
#define dialogManager Singleton<DialogManager>::instance()
#define appController fileManagerApp->getAppController()
#define fileIconProvider Singleton<IconProvider>::instance()
#define fileService FileServices::instance()
#define deviceListener Singleton<UDiskListener>::instance()
#define mimeAppsManager Singleton<MimesAppsManager>::instance()
#define systemPathManager Singleton<PathManager>::instance()
#define mimeTypeDisplayManager Singleton<MimeTypeDisplayManager>::instance()
#define thumbnailManager Singleton<ThumbnailManager>::instance()

#define defaut_icon ":/images/images/default.png"
#define defaut_computerIcon ":/images/images/computer.png"
#define defaut_trashIcon ":/images/images/user-trash-full.png"

#define ComputerUrl "computer://"

#define TRASH_SCHEME "trash"
#define RECENT_SCHEME "recent"
#define BOOKMARK_SCHEME "bookmark"
#define FILE_SCHEME "file"
#define COMPUTER_SCHEME "computer"
#define SEARCH_SCHEME "search"

#define TRASH_ROOT TRASH_SCHEME":///"
#define RECENT_ROOT RECENT_SCHEME":///"
#define BOOKMARK_ROOT BOOKMARK_SCHEME ":///"
#define FILE_ROOT FILE_SCHEME ":///"
#define COMPUTER_ROOT COMPUTER_SCHEME ":///"

#define TRASHPATH QDir::homePath() + "/.local/share/Trash"
#define TRASHFILEPATH TRASHPATH + "/files"
#define TRASHINFOPATH TRASHPATH + "/info"

#define TEXT_LINE_HEIGHT 15

#define MAX_THREAD_COUNT 1000

#define ASYN_CALL(Fun, Code, captured...) { \
    QDBusPendingCallWatcher * watcher = new QDBusPendingCallWatcher(Fun); \
    auto onFinished = [watcher, captured]{ \
        const QVariantList & args = watcher->reply().arguments(); \
        Q_UNUSED(args);\
        Code \
        watcher->deleteLater(); \
    };\
    if(watcher->isFinished()) onFinished();\
    else QObject::connect(watcher, &QDBusPendingCallWatcher::finished, onFinished);}

#if QT_VERSION >= 0x050500
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
    QTimer::singleShot(Time, [captured] {Code});\
}
#else
#define TIMER_SINGLESHOT(Time, Code, captured...){ \
    QTimer *timer = new QTimer;\
    timer->setSingleShot(true);\
    QObject::connect(timer, &QTimer::timeout, [timer, captured] {\
        timer->deleteLater();\
        Code\
    });\
    timer->start(Time);\
}
#endif

#define TIMER_SINGLESHOT_CONNECT_TYPE(Obj, Time, Code, ConnectType, captured...){ \
    QTimer *timer = new QTimer;\
    timer->setSingleShot(true);\
    QObject::connect(timer, &QTimer::timeout, Obj, [timer, captured] {\
        timer->deleteLater();\
        Code\
    }, ConnectType);\
    timer->start(Time);\
}

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
#define TIMER_SINGLESHOT_OBJECT(Obj, Time, Code, captured...)\
    TIMER_SINGLESHOT(Obj, Time, Code, Qt::AutoConnection, captured)
#else
#define TIMER_SINGLESHOT_OBJECT(Obj, Time, Code, captured...)\
    QTimer::singleShot(Time, Obj, [captured]{Code}});
#endif

#define ASYN_CALL_SLOT(obj, fun, args...) \
    TIMER_SINGLESHOT_CONNECT_TYPE(obj, 0, {obj->fun(args);}, Qt::QueuedConnection, obj, args)

class Global
{
public:
    static QString wordWrapText(const QString &text, int width,
                         QTextOption::WrapMode wrapMode,
                         int *height = 0)
    {
        QTextLayout textLayout(text);
        QTextOption &text_option = *const_cast<QTextOption*>(&textLayout.textOption());
        text_option.setWrapMode(wrapMode);

        textLayout.beginLayout();

        QTextLine line = textLayout.createLine();
        QString  str;

        int text_height = 0;

        while (line.isValid()) {
            line.setLineWidth(width);
            str += text.mid(line.textStart(), line.textLength());
            text_height += TEXT_LINE_HEIGHT;
            line = textLayout.createLine();

            if(line.isValid())
                str.append("\n");
        }

        textLayout.endLayout();

        if(height)
            *height = text_height;

        return str;
    }

    static QString elideText(const QString &text, const QSize &size,
                      const QFontMetrics &fontMetrics,
                      QTextOption::WrapMode wordWrap,
                      Qt::TextElideMode mode,
                      int flags = 0)
    {
        qreal height = 0;

        QTextLayout textLayout(text);
        QString str;

        const_cast<QTextOption*>(&textLayout.textOption())->setWrapMode(wordWrap);

        textLayout.beginLayout();

        QTextLine line = textLayout.createLine();

        while (line.isValid()) {
            height += TEXT_LINE_HEIGHT;

            if(height + TEXT_LINE_HEIGHT >= size.height()) {
                str += fontMetrics.elidedText(text.mid(line.textStart() + line.textLength() + 1),
                                              mode, size.width(), flags);
                break;
            }

            line.setLineWidth(size.width());
            str += text.mid(line.textStart(), line.textLength());

            line = textLayout.createLine();

            if(line.isValid())
                str.append("\n");
        }

        textLayout.endLayout();

        return str;
    }
};

#endif // GLOBAL_H

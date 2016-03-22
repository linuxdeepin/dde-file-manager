#ifndef GLOBAL_H
#define GLOBAL_H

#include "widgets/singleton.h"
#include "signalmanager.h"
#include "../controllers/dbuscontroller.h"
#include "utils/utils.h"
#include "filesignalmanager.h"
#include "../controllers/searchhistroymanager.h"

#include <QFontMetrics>
#include <QTextOption>
#include <QTextLayout>

//#define gridManager  Singleton<GridManager>::instance()
#define signalManager  Singleton<SignalManager>::instance()
#define searchHistoryManager  Singleton<SearchHistroyManager>::instance()
//#define dbusController Singleton<DBusController>::instance()
#define fileSignalManager Singleton<FileSignalManager>::instance()

#define defaut_icon ":/images/images/default.png"
#define defaut_computerIcon ":/images/images/computer.png"
#define defaut_trashIcon ":/images/images/user-trash-full.png"


#define ComputerUrl "computer://"
#define TrashUrl "trash://"
#define FilePrefix "file://"
#define Recent "recent://"
#define TrashDir "~/.local/share/Trash/files"

#define TEXT_LINE_HEIGHT 13

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

#include "dragmoniter.h"

#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDBusConnection>
#include <QDebug>

using namespace dfm_drag;

DragMoniter::DragMoniter(QObject *parent)
    : QObject { parent }
    , QDBusContext()
{

}

void DragMoniter::registerDBus()
{
    QDBusConnection con = QDBusConnection::sessionBus();
    if (con.registerService("org.deepin.filemanager.drag")) {
        con.registerObject("/org/deepin/filemanager/drag",
                           "org.deepin.filemanager.drag", this
                           , QDBusConnection::ExportAllSignals);

        qApp->installEventFilter(this);
    }
}

void DragMoniter::unRegisterDBus()
{
    qApp->removeEventFilter(this);
    QDBusConnection con = QDBusConnection::sessionBus();
    con.unregisterObject("/org/deepin/filemanager/drag");
    con.unregisterService("org.deepin.filemanager.drag");
}

bool DragMoniter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent *e = dynamic_cast<QDragEnterEvent *>(event);
        if (e) {
            if (auto mime = e->mimeData()) {
                if (mime->hasUrls()) {
                    auto urls = mime->urls();
                    QStringList str;
                    for (const QUrl &u : urls)
                        str << u.toString();
                    QMetaObject::invokeMethod(this, "dragEnter", Qt::QueuedConnection, Q_ARG(QStringList, str));
                }
            }
        }
    }

    return false;
}

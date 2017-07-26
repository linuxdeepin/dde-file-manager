#include "frame.h"
#include "constants.h"
#include "wallpaperlist.h"
#include "wallpaperitem.h"
#include "dbus/appearancedaemon_interface.h"
#include "dbus/deepin_wm.h"
#include "dbus/dbusxmousearea.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QScreen>

Frame::Frame(QFrame *parent)
    : DBlurEffectWidget(parent),
      m_wallpaperList(new WallpaperList(this)),
      m_closeButton(new DImageButton(":/images/close_normal.png",
                                 ":/images/close_hover.png",
                                 ":/images/close_press.png", this)),
      m_dbusAppearance(new AppearanceDaemonInterface(AppearanceServ,
                                                     AppearancePath,
                                                     QDBusConnection::sessionBus(),
                                                     this)),
      m_dbusDeepinWM(new DeepinWM(DeepinWMServ,
                                  DeepinWMPath,
                                  QDBusConnection::sessionBus(),
                                  this)),
      m_dbusMouseArea(new DBusXMouseArea(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    setMaskColor(DBlurEffectWidget::DarkColor);

    initSize();

    connect(m_dbusMouseArea, &DBusXMouseArea::ButtonPress, [this](int button, int x, int y, const QString &id){
        if (id != m_mouseAreaKey) return;

        if (button == 4) {
            m_wallpaperList->prevPage();
        } else if (button == 5) {
            m_wallpaperList->nextPage();
        } else {
            qDebug() << "button pressed on blank area, quit.";

            if (!rect().contains(x - this->x(), y - this->y())) {
                hide();
            }
        }
    });

    m_closeButton->hide();
    connect(m_wallpaperList, &WallpaperList::needCloseButton,
            this, &Frame::handleNeedCloseButton);
    connect(m_wallpaperList, &WallpaperList::needPreviewWallpaper,
            m_dbusDeepinWM, &DeepinWM::SetTransientBackground);

    QTimer::singleShot(0, this, &Frame::initListView);
}

Frame::~Frame()
{

}

void Frame::show()
{
    m_dbusDeepinWM->RequestHideWindows();
    QDBusPendingCall call = m_dbusMouseArea->RegisterFullScreen();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, call] {
         if (call.isError()) {
             qWarning() << "failed to register full screen mousearea: " << call.error().message();
         } else {
             QDBusReply<QString> reply = call.reply();
             m_mouseAreaKey = reply.value();
         }
    });

    DBlurEffectWidget::show();
}

void Frame::handleNeedCloseButton(QString path, QPoint pos)
{
    if (!path.isEmpty()) {
        m_closeButton->move(pos.x() - 10, pos.y() - 10);
        m_closeButton->show();
        m_closeButton->disconnect();

        connect(m_closeButton, &DImageButton::clicked, m_wallpaperList->getWallpaperByPath(path), [this, path] {
            m_dbusAppearance->Delete("background", path);
            m_wallpaperList->removeWallpaper(path);
            m_closeButton->hide();
        });
    } else {
        m_closeButton->hide();
    }
}

void Frame::showEvent(QShowEvent * event)
{
    activateWindow();

    refreshList();

    DBlurEffectWidget::showEvent(event);
}

void Frame::hideEvent(QHideEvent *event)
{
    DBlurEffectWidget::hideEvent(event);

    m_dbusDeepinWM->CancelHideWindows();
    m_dbusMouseArea->UnregisterArea(m_mouseAreaKey);

    if (!m_wallpaperList->desktopWallpaper().isEmpty())
        m_dbusAppearance->Set("background", m_wallpaperList->desktopWallpaper());
    else
        m_dbusDeepinWM->SetTransientBackground("");

    if (!m_wallpaperList->lockWallpaper().isEmpty())
        m_dbusAppearance->Set("greeterbackground", m_wallpaperList->lockWallpaper());

    emit done();
}

void Frame::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "escape key pressed, quit.";
        hide();
    }

    DBlurEffectWidget::keyPressEvent(event);
}

void Frame::initSize()
{
    const QRect primaryRect = qApp->primaryScreen()->geometry();

    setFixedSize(primaryRect.width(), FrameHeight);
    qDebug() << "move befor: " << this->geometry() << m_wallpaperList->geometry();
    move(primaryRect.x(), primaryRect.y() + primaryRect.height() - FrameHeight);
    qDebug() << "this move : " << this->geometry() << m_wallpaperList->geometry();
    m_wallpaperList->setFixedSize(primaryRect.width(), ListHeight);
    m_wallpaperList->move(0, (FrameHeight - ListHeight) / 2);
    qDebug() << "m_wallpaperList move: " << this->geometry() << m_wallpaperList->geometry();
}

void Frame::initListView()
{

}

void Frame::refreshList()
{
    m_wallpaperList->clear();
    m_wallpaperList->hide();

    QDBusPendingCall call = m_dbusAppearance->List("background");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, call] {
        if (call.isError()) {
            qWarning() << "failed to get all backgrounds: " << call.error().message();
        } else {
            QDBusReply<QString> reply = call.reply();
            QString value = reply.value();
            QStringList strings = processListReply(value);

            foreach (QString path, strings) {
                WallpaperItem * item = m_wallpaperList->addWallpaper(path);
                item->setDeletable(m_deletableInfo.value(path));
            }

            m_wallpaperList->setFixedWidth(width());
            m_wallpaperList->show();
        }
    });
}

QStringList Frame::processListReply(const QString &reply)
{
    QStringList result;

    QJsonDocument doc = QJsonDocument::fromJson(reply.toUtf8());
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        foreach (QJsonValue val, arr) {
            QJsonObject obj = val.toObject();
            QString id = obj["Id"].toString();
            result.append(id);
            m_deletableInfo[id] = obj["Deletable"].toBool();
        }
    }

    return result;
}

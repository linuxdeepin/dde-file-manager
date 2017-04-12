#include "dvideowidget.h"

#include <QUrl>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QColor>

static void wakeup(void *ctx)
{
    // This callback is invoked from any mpv thread (but possibly also
    // recursively from a thread that is calling the mpv API). Just notify
    // the Qt GUI thread to wake up (so that it can process events with
    // mpv_wait_event()), and return as quickly as possible.
    DVideoWidget *w = (DVideoWidget *)ctx;
    emit w->mpvEvents();
}

DVideoWidget::DVideoWidget(QWidget *parent) : QWidget(parent)
{

    std::setlocale(LC_NUMERIC, "C");

    m_mpv = mpv_create();
    if (!m_mpv)
        throw std::runtime_error("can't create mpv instance");

    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);

    initConnections();
}

DVideoWidget::~DVideoWidget()
{
    if(m_mpv){
        mpv_terminate_destroy(m_mpv);
        m_mpv = Q_NULLPTR;
    }
}

void DVideoWidget::initConnections()
{
    // From this point on, the wakeup function will be called. The callback
    // can come from any thread, so we use the QueuedConnection mechanism to
    // relay the wakeup in a thread-safe way.
    connect(this, &DVideoWidget::mpvEvents, this, &DVideoWidget::onMpvEvents,
            Qt::QueuedConnection);

    qDebug() << "proxy hook winId " << this->winId();
    auto evRelay = new EventRelayer2(windowHandle(), Q_NULLPTR, this);
    connect(evRelay, &EventRelayer2::subwindowCreated, this, &DVideoWidget::onSubwindowCreated);
    connect(evRelay, &EventRelayer2::subwindowMapped, this, &DVideoWidget::onSubwindowMapped);
    connect(this, &QObject::destroyed, [evRelay]{
        evRelay->deleteLater();
    });
}

void DVideoWidget::loadFile(const QString &uri)
{
    QString filename = uri;

    int64_t wid = winId();
    mpv_set_option(m_mpv, "wid", MPV_FORMAT_INT64, &wid);

    // Enable default bindings, because we're lazy. Normally, a player using
    // mpv as backend would implement its own key bindings.
    mpv_set_option_string(m_mpv, "input-default-bindings", "no");

    // Enable keyboard input on the X11 window. For the messy details, see
    // --input-vo-keyboard on the manpage.
    mpv_set_option_string(m_mpv, "input-vo-keyboard", "yes");

    // Let us receive property change events with MPV_EVENT_PROPERTY_CHANGE if
    // this property changes.
    mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);

    mpv_observe_property(m_mpv, 0, "track-list", MPV_FORMAT_NODE);
    mpv_observe_property(m_mpv, 0, "chapter-list", MPV_FORMAT_NODE);

//    mpv_set_option_string(m_mpv, "no-input-cursor", "yes");
//    mpv_set_option_string(m_mpv, "on-all-workspaces", "yes");
    mpv_set_option_string(m_mpv, "geometry", "100:100");

    mpv_set_wakeup_callback(m_mpv, wakeup, this);

    if (mpv_initialize(m_mpv) < 0){
        throw std::runtime_error("mpv failed to initialize");
        return;
    }

    if (m_mpv) {
        const QByteArray c_filename = filename.toUtf8();
        const char *args[] = {"loadfile", c_filename.data(), NULL};
        mpv_command_async(m_mpv, 0, args);
    }
}

void DVideoWidget::onMpvEvents()
{
    // Process all events, until the event queue is empty.
    while (m_mpv) {
        mpv_event *event = mpv_wait_event(m_mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;
        handleMpvEvent(event);
    }
}

void DVideoWidget::seekPos(const qint64 &timePos)
{
    if(!m_mpv)
        return;

    char pos[16];
    sprintf(pos, "%d", int(timePos));
    qDebug () << timePos;
    const char* args[] = {"seek", pos, "absolute", NULL};
    mpv_command_async(m_mpv, 0, args);
}

void DVideoWidget::resume()
{
    if(m_mpv){
        mpv_resume(m_mpv);
        m_isPaused = false;
        m_state = Paused;
        emit stateChanged(m_state);
    }
}

void DVideoWidget::pause()
{
    if(m_mpv){
        mpv_suspend(m_mpv);
        m_isPaused = true;
        m_state = Playing;
        emit stateChanged(m_state);
    }
}

void DVideoWidget::onSubwindowCreated(xcb_window_t winid)
{
    auto l = qApp->allWindows();
    auto it = std::find_if(l.begin(), l.end(), [=](QWindow* w) { return w->winId() == winid; });
    if (it == l.end()) {
        qDebug() << __func__ << QString("wrap 0x%1 into QWindow").arg(winid, 0, 16);
        auto *w = QWindow::fromWinId(winid);
        w->setParent(windowHandle());
        auto evRelay = new EventRelayer2(w, Q_NULLPTR, this);

        connect(this, &QObject::destroyed, [evRelay]{
            evRelay->deleteLater();
        });
    }
}

void DVideoWidget::onSubwindowMapped(xcb_window_t winid)
{
    qDebug() << __func__;
    auto l = qApp->allWindows();
    auto it = std::find_if(l.begin(), l.end(), [=](QWindow* w) { return w->winId() == winid; });
    if (it != l.end()) {
        qDebug() << "------- found child window";
    }
}

void DVideoWidget::handleMpvEvent(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                m_pos = (int)time;
                emit positionChanged(m_pos);
            } else if (prop->format == MPV_FORMAT_NONE) {
                // The property is unavailable, which probably means playback
                // was stopped.
            }
        } else if (strcmp(prop->name, "chapter-list") == 0 ||
                   strcmp(prop->name, "track-list") == 0)
        {
            // Dump the properties as JSON for demo purposes.
        }
        break;
    }
    case MPV_EVENT_VIDEO_RECONFIG: {
        // Retrieve the new video size.
        int64_t w, h;
        if (mpv_get_property(m_mpv, "dwidth", MPV_FORMAT_INT64, &w) >= 0 &&
            mpv_get_property(m_mpv, "dheight", MPV_FORMAT_INT64, &h) >= 0 &&
            w > 0 && h > 0)
        {
            //TODO
        }
        break;
    }
    case MPV_EVENT_SHUTDOWN: {
        mpv_terminate_destroy(m_mpv);
        m_mpv = NULL;
        break;
    }
    case MPV_EVENT_FILE_LOADED:{
        mpv_get_property(m_mpv, "length", MPV_FORMAT_INT64, &m_duration);
//        pause();
        emit fileLoaded();
        m_state = Playing;
        emit stateChanged(m_state);
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

qint64 DVideoWidget::pos() const
{
    return m_pos;
}

qint64 DVideoWidget::duration() const
{
    return m_duration;
}

EventRelayer2::EventRelayer2(QWindow *src, QWindow *dest, QObject *parent):
    QObject(parent), QAbstractNativeEventFilter(), _source(src), _target(dest)
{
    xcb_connection_t *conn = QX11Info::connection();
//    int screen = 0;
//    xcb_screen_t *s = xcb_aux_get_screen(conn, screen);

    auto cookie = xcb_get_window_attributes(conn, _source->winId());
    auto reply = xcb_get_window_attributes_reply(conn, cookie, NULL);

    const uint32_t data[] = {
        reply->your_event_mask |
            XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_POINTER_MOTION

    };
    xcb_change_window_attributes (QX11Info::connection(), _source->winId(),
            XCB_CW_EVENT_MASK, data);

    xcb_aux_sync(QX11Info::connection());
    qApp->installNativeEventFilter(this);
}

bool EventRelayer2::nativeEventFilter(const QByteArray &eventType, void *message, long *data)
{
    if(Q_LIKELY(eventType == "xcb_generic_event_t")) {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t *>(message);
        switch (event->response_type & ~0x80) {
            case XCB_CREATE_NOTIFY:
                {
                    xcb_create_notify_event_t *cne = (xcb_create_notify_event_t*)event;
                    if (cne->parent == _source->winId()) {
                        qDebug() << "----------------->create notify "
                            << QString("0x%1").arg(cne->window, 0, 16)
                            << QString("0x%1").arg(cne->parent, 0, 16);
                        emit subwindowCreated(cne->window);
                    }

                    break;
                }
            case XCB_MAP_NOTIFY:
                {
                    xcb_map_notify_event_t *mne = (xcb_map_notify_event_t*)event;
                    if (mne->event == _source->winId()) {
                        qDebug() << "map notify "
                            << QString("0x%1").arg(mne->window, 0, 16)
                            << QString("0x%1").arg(mne->event, 0, 16);
                        emit subwindowMapped(mne->window);
                    }

                    break;
                }
            case XCB_MOTION_NOTIFY:
                {
                    xcb_motion_notify_event_t *cne = (xcb_motion_notify_event_t*)event;
                    qDebug() << "motion notify "
                        << QString("0x%1").arg(cne->event, 0, 16);

                    break;
                }
            case XCB_BUTTON_PRESS:
                {
                    xcb_button_press_event_t *cne = (xcb_button_press_event_t*)event;
                    qDebug() << "btn press "
                        << QString("0x%1").arg(cne->event, 0, 16);

                    break;
                }

            default:
                break;
        }
    }

    return false;
}

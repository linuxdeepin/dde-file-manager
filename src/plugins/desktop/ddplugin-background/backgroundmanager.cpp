// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef COMPILE_ON_V2X
#    include "backgrounddde.h"
#else
#    include "backgroundwm.h"
#endif
#include "backgroundmanager.h"
#include "backgroundmanager_p.h"
#include "backgrounddefault.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/utils/universalutils.h>

#include <QImageReader>
#include <QtConcurrent>

DFMBASE_USE_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

inline QString getScreenName(QWidget *win)
{
    Q_ASSERT(win);
    return win->property(DesktopFrameProperty::kPropScreenName).toString();
}

static QMap<QString, QWidget *> rootMap()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    QMap<QString, QWidget *> ret;
    for (QWidget *win : root) {
        QString name = getScreenName(win);
        if (name.isEmpty())
            continue;
        ret.insert(name, win);
    }

    return ret;
}

BackgroundManagerPrivate::BackgroundManagerPrivate(BackgroundManager *qq)
    : QObject(qq), q(qq)
{
}

BackgroundManagerPrivate::~BackgroundManagerPrivate()
{
    backgroundWidgets.clear();
    backgroundPaths.clear();

    delete bridge;
    bridge = nullptr;
}

bool BackgroundManagerPrivate::isEnableBackground()
{
    //return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
    // fix bug152473
    return enableBackground;
}

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent), d(new BackgroundManagerPrivate(this))
{
    d->service =
#ifdef COMPILE_ON_V2X
            new BackgroundDDE(this);
#else
            new BackgroundWM(this);
#endif

    d->bridge = new BackgroundBridge(d);
}

BackgroundManager::~BackgroundManager()
{
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &BackgroundManager::onDetachWindows);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &BackgroundManager::onBackgroundBuild);
    CanvasCoreUnsubscribe(signal_DesktopFrame_GeometryChanged, &BackgroundManager::onGeometryChanged);
}

void BackgroundManager::init()
{
    restBackgroundManager();

    CanvasCoreSubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &BackgroundManager::onDetachWindows);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowBuilded, &BackgroundManager::onBackgroundBuild);
    CanvasCoreSubscribe(signal_DesktopFrame_GeometryChanged, &BackgroundManager::onGeometryChanged);
}

QMap<QString, BackgroundWidgetPointer> BackgroundManager::allBackgroundWidgets()
{
    return d->backgroundWidgets;
}

BackgroundWidgetPointer BackgroundManager::backgroundWidget(const QString &screen)
{
    return d->backgroundWidgets.value(screen);
}

QMap<QString, QString> BackgroundManager::allBackgroundPath()
{
    return d->backgroundPaths;
}

QString BackgroundManager::backgroundPath(const QString &screen)
{
    return d->backgroundPaths.value(screen);
}

void BackgroundManager::onBackgroundBuild()
{
    // get wallpapers
    if (d->bridge->isForce() && d->bridge->isRunning())
        fmWarning() << "a force requestion is running. skip to get wallpaper.";
    else
        d->bridge->request(d->backgroundPaths.isEmpty());

    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    if (root.size() == 1) {
        QWidget *primary = root.first();
        if (primary == nullptr) {
            // get screen failed,clear all widget
            d->backgroundWidgets.clear();
            fmCritical() << "get primary screen failed return.";
            return;
        }

        const QString &screeName = getScreenName(primary);
        if (screeName.isEmpty()) {
            fmWarning() << "can not get screen name from root window";
            return;
        }

        BackgroundWidgetPointer bwp = d->backgroundWidgets.value(screeName);
        d->backgroundWidgets.clear();
        if (!bwp.isNull()) {
            QRect geometry = d->relativeGeometry(primary->geometry());   // scaled area
            if (bwp->geometry() != geometry)
                bwp->setGeometry(geometry);
            bwp->setParent(primary);
        } else {
            bwp = createBackgroundWidget(primary);
        }

        d->backgroundWidgets.insert(screeName, bwp);
    } else {
        // check whether to add
        for (QWidget *win : root) {

            const QString screenName = getScreenName(win);
            if (screenName.isEmpty()) {
                fmWarning() << "can not get screen name from root window";
                continue;
            }

            BackgroundWidgetPointer bwp = d->backgroundWidgets.value(screenName);
            if (!bwp.isNull()) {
                // update widget
                QRect geometry = d->relativeGeometry(win->geometry());   // scaled area
                if (bwp->geometry() != geometry)
                    bwp->setGeometry(geometry);
                bwp->setParent(win);
            } else {
                // add new widget
                fmInfo() << "screen:" << screenName << "  added, create it.";
                bwp = createBackgroundWidget(win);
                d->backgroundWidgets.insert(screenName, bwp);
            }
        }

        // clean up invalid widget
        {
            auto winMap = rootMap();
            for (const QString &sp : d->backgroundWidgets.keys()) {
                if (!winMap.contains(sp)) {
                    d->backgroundWidgets.take(sp);
                    fmInfo() << "remove screen:" << sp;
                }
            }
        }
    }
}

void BackgroundManager::onDetachWindows()
{
    for (const BackgroundWidgetPointer &bwp : d->backgroundWidgets.values())
        bwp->setParent(nullptr);
}

void BackgroundManager::restBackgroundManager()
{
    if (d->isEnableBackground()) {
        connect(d->service, &BackgroundService::backgroundChanged, this, &BackgroundManager::onBackgroundChanged);
        if (ddplugin_desktop_util::desktopFrameRootWindows().isEmpty())
            d->bridge->forceRequest();
        else
            onBackgroundBuild();
    } else {
        disconnect(d->service, &BackgroundService::backgroundChanged, this, &BackgroundManager::onBackgroundChanged);
        d->backgroundWidgets.clear();
        d->backgroundPaths.clear();
        d->bridge->terminate(true);
    }
}

bool BackgroundManager::useColorBackground()
{
    bool used { false };
    for (auto path : d->backgroundPaths.values()) {
        QString localPath = UniversalUtils::covertUrlToLocalPath(path);
        // these dirs save solid color wallpapers.
        used = localPath.startsWith("/usr/share/wallpapers/custom-solidwallpapers")
                || localPath.startsWith("/usr/share/wallpapers/deepin-solidwallpapers");

        if (used)
            return used;
    }

    return used;
}

void BackgroundManager::onBackgroundChanged()
{
    if (d->bridge->isRunning()) {
        fmWarning() << "there is running requetion, redo after it's finished.";
        d->bridge->setRepeat();
    } else {
        d->bridge->request(true);
    }
}

void BackgroundManager::onGeometryChanged()
{
    bool changed = false;
    auto winMap = rootMap();
    for (auto itor = d->backgroundWidgets.begin(); itor != d->backgroundWidgets.end(); ++itor) {
        BackgroundWidgetPointer bw = itor.value();
        auto *win = winMap.value(itor.key());
        if (win == nullptr) {
            fmCritical() << "can not get root " << itor.key() << getScreenName(win);
            continue;
        }

        if (bw.get() != nullptr) {
            QRect geometry = d->relativeGeometry(win->geometry());   // scaled area
            if (bw->geometry() == geometry) {
                fmWarning() << "background geometry is equal to root widget geometry,and discard changes" << bw->geometry()
                            << win->geometry() << win->property(DesktopFrameProperty::kPropScreenName).toString()
                            << win->property(DesktopFrameProperty::kPropScreenGeometry).toRect() << win->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect()
                            << win->property(DesktopFrameProperty::kPropScreenAvailableGeometry);
                continue;
            }
            fmInfo() << "background geometry change from" << bw->geometry() << "to" << geometry
                     << "screen name" << getScreenName(win) << "screen geometry" << win->geometry();
            bw->setGeometry(geometry);
            changed = true;
        }
    }

    // rescale images.
    if (changed)
        d->bridge->request(false);
}

BackgroundWidgetPointer BackgroundManager::createBackgroundWidget(QWidget *root)
{
    const QString screenName = getScreenName(root);
    BackgroundWidgetPointer bwp(new BackgroundDefault(screenName, root));
    bwp->setParent(root);
    bwp->setProperty(DesktopFrameProperty::kPropScreenName, getScreenName(root));
    bwp->setProperty(DesktopFrameProperty::kPropWidgetName, "background");
    bwp->setProperty(DesktopFrameProperty::kPropWidgetLevel, 5.0);

    QRect geometry = d->relativeGeometry(root->geometry());   // scaled area
    bwp->setGeometry(geometry);
    fmDebug() << "screen name" << screenName << "geometry" << geometry << root->geometry() << bwp.get();

    return bwp;
}

BackgroundBridge::BackgroundBridge(BackgroundManagerPrivate *ptr)
    : QObject(), d(ptr)
{
}

BackgroundBridge::~BackgroundBridge()
{
    fmInfo() << "wait for finishing";
    getting = false;
    future.waitForFinished();
}

void BackgroundBridge::request(bool refresh)
{
    terminate(true);

    QList<Requestion> requestion;
    for (QWidget *root : ddplugin_desktop_util::desktopFrameRootWindows()) {
        Requestion req;
        req.screen = getScreenName(root);

        if (req.screen.isEmpty()) {
            fmWarning() << "can not get screen name from root window";
            continue;
        }

        // use the resolution before screen zooming
        req.size = root->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect().size();

        if (!refresh)
            req.path = d->backgroundPaths.value(req.screen);
        requestion.append(req);
    }

    if (!requestion.isEmpty()) {
        getting = true;
        future = QtConcurrent::run(&BackgroundBridge::runUpdate, this, requestion);
    }
}

void BackgroundBridge::forceRequest()
{
    terminate(true);

    QList<Requestion> requestion;
    for (ScreenPointer sc : ddplugin_desktop_util::screenProxyScreens()) {
        Requestion req;
        req.screen = sc->name();

        if (req.screen.isEmpty()) {
            fmWarning() << "can not get screen name from root window";
            continue;
        }

        // use the resolution before screen zooming
        req.size = sc->handleGeometry().size();
        requestion.append(req);
    }

    if (!requestion.isEmpty()) {
        getting = true;
        force = true;
        future = QtConcurrent::run(&BackgroundBridge::runUpdate, this, requestion);
    }
}

void BackgroundBridge::terminate(bool wait)
{
    fmInfo() << "terminate last requestion, wait:" << wait << "running:" << getting << future.isRunning() << "force" << force;
    if (!getting)
        return;

    getting = false;
    if (wait)
        future.waitForFinished();

    force = false;
}

QPixmap BackgroundBridge::getPixmap(const QString &path, const QPixmap &defalutPixmap)
{
    if (path.isEmpty())
        return defalutPixmap;

    QString currentWallpaper = path.startsWith("file:") ? QUrl(path).toLocalFile() : path;
    QPixmap backgroundPixmap(currentWallpaper);
    // fix whiteboard shows when a jpeg file with filename xxx.png
    // content formart not epual to extension
    if (backgroundPixmap.isNull()) {
        QImageReader reader(currentWallpaper);
        reader.setDecideFormatFromContent(true);
        backgroundPixmap = QPixmap::fromImage(reader.read());
    }

    return backgroundPixmap.isNull() ? defalutPixmap : backgroundPixmap;
}

void BackgroundBridge::onFinished(void *pData)
{
    fmInfo() << "finished to get backround.." << pData << "force:" << force;
    force = false;

    QList<Requestion> *images = reinterpret_cast<QList<Requestion> *>(pData);
    Q_ASSERT(images);
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    d->backgroundPaths.clear();
    for (Requestion &req : *images) {
        for (auto it = d->backgroundWidgets.begin(); it != d->backgroundWidgets.end(); ++it) {
            if (it.key() == req.screen) {
                BackgroundWidgetPointer bw = it.value();
                req.pixmap.setDevicePixelRatio(bw->devicePixelRatioF());
                bw->setPixmap(req.pixmap);
                d->backgroundPaths.insert(req.screen, req.path);
                break;
            }
        }
    }

    delete images;

    if (repeat) {
        fmInfo() << "need to request again.";
        repeat = false;
        request(true);
    }

    dpfSignalDispatcher->publish("ddplugin_background", "signal_Background_BackgroundSetted");
}

void BackgroundBridge::runUpdate(BackgroundBridge *self, QList<Requestion> reqs)
{
    fmInfo() << "getting background in work thread...." << QThread::currentThreadId();
    QList<Requestion> recorder;
    for (Requestion &req : reqs) {
        // check stop
        if (!self->getting)
            return;

        if (req.path.isEmpty())
            req.path = self->d->service->background(req.screen);

        QPixmap backgroundPixmap = BackgroundBridge::getPixmap(req.path);
        if (backgroundPixmap.isNull()) {
            fmCritical() << "screen " << req.screen << "backfround path" << req.path
                         << "can not read!";
            continue;
        }

        // check stop
        if (!self->getting)
            return;

        QSize trueSize = req.size;
        auto pix = backgroundPixmap.scaled(trueSize,
                                           Qt::KeepAspectRatioByExpanding,
                                           Qt::SmoothTransformation);

        // check stop
        if (!self->getting)
            return;

        if (pix.width() > trueSize.width() || pix.height() > trueSize.height()) {
            pix = pix.copy(QRect(static_cast<int>((pix.width() - trueSize.width()) / 2.0),
                                 static_cast<int>((pix.height() - trueSize.height()) / 2.0),
                                 trueSize.width(),
                                 trueSize.height()));
        }

        fmDebug() << req.screen << "background path" << req.path << "truesize" << trueSize;
        req.pixmap = pix;
        recorder.append(req);
    }

    // check stop
    if (!self->getting)
        return;

    QList<Requestion> *pRecorder = new QList<Requestion>;
    *pRecorder = std::move(recorder);
    QMetaObject::invokeMethod(self, "onFinished", Qt::QueuedConnection, Q_ARG(void *, pRecorder));
    self->getting = false;
}

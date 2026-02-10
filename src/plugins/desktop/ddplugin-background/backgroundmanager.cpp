// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgrounddde.h"
#include "backgroundmanager.h"
#include "backgroundmanager_p.h"
#include "backgrounddefault.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/utils/universalutils.h>

#include <QImageReader>
#include <QtConcurrent>
#include <QPixmapCache>

DFMBASE_USE_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

// Generate cache key for wallpaper: wallpaper:path@widthxheight
static QString generateCacheKey(const QString &path, const QSize &size)
{
    return QString("wallpaper:%1@%2x%3")
            .arg(path)
            .arg(size.width())
            .arg(size.height());
}

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
    // return windowManagerHelper->windowManagerName() == DWindowManagerHelper::KWinWM || !windowManagerHelper->hasComposite();
    //  fix bug152473
    return enableBackground;
}

BackgroundManager::BackgroundManager(QObject *parent)
    : QObject(parent), d(new BackgroundManagerPrivate(this))
{
    d->service = new BackgroundDDE(this);

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
    // Set wallpaper cache limit to 100MB (approx 3-5 4K wallpapers)
    // QPixmapCache uses KB as unit: 100MB = 100 * 1024 KB
    static std::once_flag flag;
    std::call_once(flag, QPixmapCache::setCacheLimit, 100 * 1024);

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
    updateBackgroundWidgets();
    // get wallpapers
    if (d->bridge->isForce() && d->bridge->isRunning())
        fmWarning() << "a force requestion is running. skip to get wallpaper.";
    else
        d->bridge->request(d->backgroundPaths.isEmpty());
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

        if (used) {
            fmInfo() << "Color background detected at path:" << localPath;
            return used;
        }
    }

    fmDebug() << "No color backgrounds found, using regular wallpapers";
    return used;
}

void BackgroundManager::onBackgroundChanged()
{
    if (d->bridge->isRunning()) {
        fmWarning() << "Background request already running, scheduling repeat after completion";
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
            fmCritical() << "Cannot get root window for screen:" << itor.key() << "widget screen name:" << getScreenName(win);
            continue;
        }

        if (bw.get() != nullptr) {
            QRect screenGeometry = win->property(DesktopFrameProperty::kPropScreenGeometry).toRect();
            QRect geometry = d->relativeGeometry(screenGeometry);   // scaled area
            if (bw->geometry() == geometry) {
                fmWarning() << "Background geometry equals root widget geometry, discarding changes - geometry:" << bw->geometry()
                            << "window geometry:" << win->geometry() << "screen name:" << win->property(DesktopFrameProperty::kPropScreenName).toString()
                            << "screen geometry:" << win->property(DesktopFrameProperty::kPropScreenGeometry).toRect()
                            << "handle geometry:" << win->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect()
                            << "available geometry:" << win->property(DesktopFrameProperty::kPropScreenAvailableGeometry);
                continue;
            }

            fmInfo() << "Background geometry changed from" << bw->geometry() << "to" << geometry
                     << "for screen:" << getScreenName(win) << "window geometry:" << win->geometry()
                     << "screen geometry from property:" << screenGeometry;
            bw->setGeometry(geometry);
            changed = true;
        }
    }

    // rescale images.
    if (changed) {
        fmInfo() << "Geometry changes detected, requesting background rescaling";
        d->bridge->request(false);
    }
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
    fmInfo() << "Background widget created successfully - screen:" << screenName << "geometry:" << geometry
             << "root geometry:" << root->geometry() << "widget pointer:" << bwp.get();

    return bwp;
}

void BackgroundManager::updateBackgroundWidgets()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    if (root.size() == 1) {
        fmInfo() << "Single screen mode detected";
        QWidget *primary = root.first();
        if (primary == nullptr) {
            // get screen failed,clear all widget
            d->backgroundWidgets.clear();
            fmCritical() << "Failed to get primary screen, clearing all background widgets";
            return;
        }

        const QString &screeName = getScreenName(primary);
        if (screeName.isEmpty()) {
            fmWarning() << "Cannot get screen name from root window in single screen mode";
            return;
        }

        BackgroundWidgetPointer bwp = d->backgroundWidgets.value(screeName);
        d->backgroundWidgets.clear();
        if (!bwp.isNull()) {
            QRect screenGeometry = primary->property(DesktopFrameProperty::kPropScreenGeometry).toRect();
            QRect geometry = d->relativeGeometry(screenGeometry);   // scaled area
            if (bwp->geometry() != geometry) {
                fmInfo() << "Updating background widget geometry from" << bwp->geometry() << "to" << geometry;
                bwp->setGeometry(geometry);
            }
            bwp->setParent(primary);
            fmDebug() << "Reused existing background widget for screen:" << screeName;
        } else {
            fmInfo() << "Creating new background widget for primary screen:" << screeName;
            bwp = createBackgroundWidget(primary);
        }

        d->backgroundWidgets.insert(screeName, bwp);
        fmInfo() << "Single screen background setup completed for:" << screeName;
    } else {
        fmInfo() << "Multi-screen mode detected with" << root.size() << "screens";
        // check whether to add
        for (QWidget *win : root) {

            const QString screenName = getScreenName(win);
            if (screenName.isEmpty()) {
                fmWarning() << "Cannot get screen name from root window in multi-screen mode";
                continue;
            }

            BackgroundWidgetPointer bwp = d->backgroundWidgets.value(screenName);
            if (!bwp.isNull()) {
                // update widget
                QRect screenGeometry = win->property(DesktopFrameProperty::kPropScreenGeometry).toRect();
                QRect geometry = d->relativeGeometry(screenGeometry);   // scaled area
                if (bwp->geometry() != geometry) {
                    fmInfo() << "Updating background widget geometry for screen" << screenName << "from" << bwp->geometry() << "to" << geometry;
                    bwp->setGeometry(geometry);
                }
                bwp->setParent(win);
                fmDebug() << "Updated existing background widget for screen:" << screenName;
            } else {
                // add new widget
                fmInfo() << "Screen" << screenName << "added, creating new background widget";
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
                    fmInfo() << "Removed background widget for disconnected screen:" << sp;
                }
            }
        }
        fmInfo() << "Multi-screen background setup completed, active screens:" << d->backgroundWidgets.keys();
    }
}

BackgroundBridge::BackgroundBridge(BackgroundManagerPrivate *ptr)
    : QObject(), d(ptr)
{
}

BackgroundBridge::~BackgroundBridge()
{
    fmInfo() << "BackgroundBridge destroying, waiting for background requests to finish";
    getting = false;
    future.waitForFinished();
}

void BackgroundBridge::queryCacheAndClassify(Requestion &req, QList<Requestion> &cachedRequests, QList<Requestion> &uncachedRequests)
{
    if (req.path.isEmpty()) {
        fmDebug() << "Empty background path for screen:" << req.screen;
        return;
    }

    // Query cache with file path
    QString wallpaperPath = req.path.startsWith("file:") ? QUrl(req.path).toLocalFile() : req.path;
    QString cacheKey = generateCacheKey(wallpaperPath, req.size);
    QPixmap cached;

    if (QPixmapCache::find(cacheKey, &cached)) {
        fmInfo() << "Cache hit in main thread - screen:" << req.screen << "path:" << wallpaperPath;
        req.pixmap = cached;
        cachedRequests.append(req);
    } else {
        fmDebug() << "Cache miss - queuing for async loading - screen:" << req.screen;
        uncachedRequests.append(req);
    }
}

void BackgroundBridge::processRequests(const QList<Requestion> &cachedRequests, const QList<Requestion> &uncachedRequests, bool forceMode)
{
    // Process cached requests immediately in main thread
    if (!cachedRequests.isEmpty()) {
        QList<Requestion> *pCached = new QList<Requestion>(cachedRequests);
        onFinished(pCached);
    }

    // Process uncached requests asynchronously
    if (!uncachedRequests.isEmpty()) {
        getting = true;
        if (forceMode)
            force = true;
        future = QtConcurrent::run(&BackgroundBridge::runUpdate, this, uncachedRequests);
    }
}

void BackgroundBridge::request(bool refresh)
{
    terminate(true);

    // Pre-query cache in main thread (QPixmapCache is only usable from the application's main thread)
    QList<Requestion> cachedRequests;
    QList<Requestion> uncachedRequests;

    for (QWidget *root : ddplugin_desktop_util::desktopFrameRootWindows()) {
        Requestion req;
        req.screen = getScreenName(root);

        if (req.screen.isEmpty()) {
            fmWarning() << "Cannot get screen name from root window, skipping";
            continue;
        }

        // use the resolution before screen zooming
        req.size = root->property(DesktopFrameProperty::kPropScreenHandleGeometry).toRect().size();

        // Get wallpaper path
        if (!refresh)
            req.path = d->backgroundPaths.value(req.screen);
        if (req.path.isEmpty())
            req.path = d->service->background(req.screen);

        queryCacheAndClassify(req, cachedRequests, uncachedRequests);
    }

    processRequests(cachedRequests, uncachedRequests);
}

void BackgroundBridge::forceRequest()
{
    terminate(true);

    // Pre-query cache in main thread (QPixmapCache is only usable from the application's main thread)
    QList<Requestion> cachedRequests;
    QList<Requestion> uncachedRequests;

    for (ScreenPointer sc : ddplugin_desktop_util::screenProxyScreens()) {
        Requestion req;
        req.screen = sc->name();

        if (req.screen.isEmpty()) {
            fmWarning() << "Cannot get screen name from screen proxy, skipping";
            continue;
        }

        // use the resolution before screen zooming
        req.size = sc->handleGeometry().size();

        // Get wallpaper path
        req.path = d->service->background(req.screen);

        queryCacheAndClassify(req, cachedRequests, uncachedRequests);
    }

    processRequests(cachedRequests, uncachedRequests, true);
}

void BackgroundBridge::terminate(bool wait)
{
    if (!getting) {
        fmDebug() << "No active background request to terminate";
        return;
    }

    fmInfo() << "Terminating background request - wait:" << wait << "running:" << getting
             << "future running:" << future.isRunning() << "force mode:" << force;

    getting = false;
    if (wait)
        future.waitForFinished();

    force = false;
}

QPixmap BackgroundBridge::getPixmap(const QString &path, const QSize &targetSize, const QPixmap &defaultPixmap)
{
    if (path.isEmpty()) {
        fmDebug() << "Empty background path provided, using default pixmap";
        return defaultPixmap;
    }

    QString currentWallpaper = path.startsWith("file:") ? QUrl(path).toLocalFile() : path;

    // Initialize image reader with format detection
    QImageReader reader(currentWallpaper);
    reader.setDecideFormatFromContent(true);
    QSize imageSize = reader.size();
    if (!imageSize.isValid()) {
        fmWarning() << "Failed to get image size:" << currentWallpaper << "error:" << reader.errorString();
        return defaultPixmap;
    }

    // Calculate target size maintaining aspect ratio
    QSize scaledSize = imageSize.scaled(targetSize, Qt::KeepAspectRatioByExpanding);
    // For large images, let QImageReader scale during decoding to save memory
    if (imageSize.width() > scaledSize.width() * 2 || imageSize.height() > scaledSize.height() * 2) {
        fmInfo() << "Large image optimization - decoding at scaled size:" << scaledSize;
        reader.setScaledSize(scaledSize);
    }

    // Read and decode image
    QImage image = reader.read();
    if (image.isNull()) {
        fmCritical() << "Failed to decode image:" << currentWallpaper << "error:" << reader.errorString();
        return defaultPixmap;
    }

    QPixmap backgroundPixmap = QPixmap::fromImage(std::move(image));

    // Final scaling if needed
    if (backgroundPixmap.size() != scaledSize) {
        fmDebug() << "Scaling wallpaper from" << backgroundPixmap.size() << "to" << scaledSize;
        backgroundPixmap = backgroundPixmap.scaled(scaledSize,
                                                   Qt::KeepAspectRatioByExpanding,
                                                   Qt::SmoothTransformation);
    }

    fmDebug() << "Wallpaper loaded in worker thread - path:" << currentWallpaper << "size:" << backgroundPixmap.size();
    return backgroundPixmap;
}

void BackgroundBridge::onFinished(void *pData)
{
    fmInfo() << "Background update finished - data pointer:" << pData << "force mode:" << force;
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

                // Insert into cache
                QString wallpaperPath = req.path.startsWith("file:") ? QUrl(req.path).toLocalFile() : req.path;
                QString cacheKey = generateCacheKey(wallpaperPath, req.size);
                if (!QPixmapCache::find(cacheKey, nullptr)) {
                    bool cached = QPixmapCache::insert(cacheKey, req.pixmap);
                    fmDebug() << "Wallpaper cached in main thread - path:" << wallpaperPath
                              << "screen:" << req.screen << "cached:" << (cached ? "yes" : "no (too large)");
                }

                break;
            }
        }
    }

    delete images;

    if (repeat) {
        fmInfo() << "Repeat flag set, requesting background update again";
        repeat = false;
        request(true);
    }

    dpfSignalDispatcher->publish("ddplugin_background", "signal_Background_BackgroundSetted");
}

void BackgroundBridge::runUpdate(BackgroundBridge *self, QList<Requestion> reqs)
{
    fmInfo() << "Starting background update in worker thread - thread ID:" << QThread::currentThreadId()
             << "processing" << reqs.size() << "requests";

    QList<Requestion> recorder;
    for (Requestion &req : reqs) {
        // check stop
        if (!self->getting) {
            fmInfo() << "Background update cancelled during processing";
            return;
        }

        if (req.path.isEmpty())
            req.path = self->d->service->background(req.screen);

        QSize trueSize = req.size;
        QPixmap backgroundPixmap = BackgroundBridge::getPixmap(req.path, trueSize);
        if (backgroundPixmap.isNull()) {
            fmCritical() << "Failed to read background for screen:" << req.screen << "path:" << req.path;
            continue;
        }

        // check stop
        if (!self->getting) {
            fmInfo() << "Background update cancelled after pixmap loading";
            return;
        }

        // Crop the center part if needed (image is already scaled)
        QPixmap pix = backgroundPixmap;
        if (pix.width() > trueSize.width() || pix.height() > trueSize.height()) {
            pix = pix.copy(QRect(static_cast<int>((pix.width() - trueSize.width()) / 2.0),
                                 static_cast<int>((pix.height() - trueSize.height()) / 2.0),
                                 trueSize.width(),
                                 trueSize.height()));
        }

        fmInfo() << "Successfully processed background for screen:" << req.screen << "path:" << req.path << "size:" << trueSize;
        req.pixmap = pix;
        recorder.append(req);
    }

    // check stop
    if (!self->getting) {
        fmInfo() << "Background update cancelled before completion";
        return;
    }

    QList<Requestion> *pRecorder = new QList<Requestion>;
    *pRecorder = std::move(recorder);
    QMetaObject::invokeMethod(self, "onFinished", Qt::QueuedConnection, Q_ARG(void *, pRecorder));
    self->getting = false;
}

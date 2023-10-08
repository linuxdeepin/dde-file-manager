// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaperprovider.h"

#include <QThread>
#include <QApplication>
#include <QScreen>
#include <QImageReader>
#include <QPainter>

using namespace dfm_wallpapersetting;

#define APPEARANCE_SERVICE "com.deepin.daemon.Appearance"
#define APPEARANCE_PATH "/com/deepin/daemon/Appearance"

static inline QString covertUrlToLocalPath(const QString &url)
{
    if (url.startsWith("/"))
        return url;
    else
        return QUrl(QUrl::fromPercentEncoding(url.toUtf8())).toLocalFile();
}

WallpaperProvider::WallpaperProvider(QObject *parent) : QObject(parent)
{
    workThread = new QThread(this);

    qDebug() << "create com.deepin.wm interface.";
    wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm",
                          QDBusConnection::sessionBus(), this);
    qDebug() << "end com.deepin.wm interface.";

    qDebug() << QString("create %1.").arg(APPEARANCE_SERVICE);
    appearanceIfs = new AppearanceIfs(APPEARANCE_SERVICE,
                                      APPEARANCE_PATH,
                                      QDBusConnection::sessionBus(), this);
    appearanceIfs->setTimeout(5000);
    qDebug() << QString("end %1.").arg(APPEARANCE_SERVICE);

    worker = new InterfaceWorker(appearanceIfs);
    worker->moveToThread(workThread);
    workThread->start();

    // DirectConnection to set datas
    connect(worker, &InterfaceWorker::pushBackground, this, &WallpaperProvider::setWallpaper, Qt::DirectConnection);

    //queue
    connect(worker, &InterfaceWorker::pushThumbnail, this, &WallpaperProvider::setThumbnail, Qt::QueuedConnection);
    connect(worker, &InterfaceWorker::pushBackgroundStat, this, &WallpaperProvider::updateWallpaper, Qt::QueuedConnection);

    // listen to wallpaper signals
    connect(appearanceIfs, &__Appearance::Changed, this, &WallpaperProvider::onAppearanceChanged);
}

WallpaperProvider::~WallpaperProvider()
{
    worker->terminate();
    workThread->quit();
    workThread->wait(1000);

    if (workThread->isRunning())
        workThread->terminate();

    delete worker;
    worker = nullptr;
}

void WallpaperProvider::fecthData()
{
    // get picture
    if (wallpaperMtx.tryLock(1)) {
        fecthing = true;
        QMetaObject::invokeMethod(worker, "onListBackground", Qt::QueuedConnection);
    } else {
        qWarning() << "wallpaper is locked...";
    }
}

bool WallpaperProvider::waitWallpaper(int ms) const
{
    bool ret = wallpaperMtx.tryLock(ms);
    if (ret)
        wallpaperMtx.unlock();
    return ret;
}

QList<ItemNodePtr> WallpaperProvider::pictures() const
{
    QList<ItemNodePtr> list;
    for (const ItemNodePtr & ptr : wallpaper) {
        if (!isColor(ptr->item))
            list.append(ptr);
    }
    return list;
}

QList<ItemNodePtr> WallpaperProvider::colors() const
{
    QList<ItemNodePtr> list;
    for (const ItemNodePtr & ptr : wallpaper) {
        if (isColor(ptr->item))
            list.append(ptr);
    }
    return list;
}

bool WallpaperProvider::isColor(const QString &path)
{
    // these dirs save solid color wallpapers.
    return path.startsWith("/usr/share/wallpapers/custom-solidwallpapers")
            || path.startsWith("/usr/share/wallpapers/deepin-solidwallpapers");
}

QString WallpaperProvider::getSlideshow(const QString &screen)
{
    if (screen.isEmpty())
        return QString();

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(screen);

    // com::deepin::daemon::Appearance do not export GetWallpaperSlideShow.
    QString wallpaperSlideShow = QDBusPendingReply<QString>(appearanceIfs->asyncCallWithArgumentList(
                                                                QStringLiteral("GetWallpaperSlideShow"), argumentList));

    qDebug() << "Appearance GetWallpaperSlideShow : " << wallpaperSlideShow;
    return wallpaperSlideShow;
}

void WallpaperProvider::setSlideshow(const QString &screen, const QString &value)
{
    qInfo() << "set wallpaper slideshow" << screen << value;
    if (screen.isEmpty())
        return;

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(screen) << QVariant::fromValue(value);
    appearanceIfs->asyncCallWithArgumentList(QStringLiteral("SetWallpaperSlideShow"), argumentList);
}

void WallpaperProvider::setBackgroundForMonitor(const QString &screenName, const QString &path)
{
    qInfo() << "Appearance SetMonitorBackground " << screenName << path;
    if (screenName.isEmpty() || path.isEmpty())
        return;

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(screenName) << QVariant::fromValue(path);
    auto reply = appearanceIfs->asyncCallWithArgumentList(QStringLiteral("SetMonitorBackground"), argumentList);
    reply.waitForFinished();
    qDebug() << "Appearance SetMonitorBackground end" << !reply.isError();
}

QString WallpaperProvider::getBackgroundForMonitor(const QString &screenName)
{
    QString ret;
    if (screenName.isEmpty())
        return ret;

    ret = wmInter->GetCurrentWorkspaceBackgroundForMonitor(screenName);
    return covertUrlToLocalPath(ret);
}

bool WallpaperProvider::deleteBackground(const ItemNodePtr &ptr)
{
    if (!ptr)
        return false;

    qInfo() << "try to delete wallpaper" << ptr->item;
    auto reply = appearanceIfs->Delete("background", ptr->item);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "can not delete " << ptr->item;
        return false;
    }

    qDebug() << "wallpaper is deleted";
    wallpaperMap.remove(ptr->item);
    wallpaper.removeOne(ptr);
    return true;
}

void WallpaperProvider::setBackgroundToGreeter(const QString &path)
{
    qInfo() << "Appearance set greeterbackground " << path;
    appearanceIfs->Set("greeterbackground", path);
}

void WallpaperProvider::updateBackgroundStat()
{
    QMetaObject::invokeMethod(worker, "onUpdateStat", Qt::QueuedConnection);
}

void WallpaperProvider::onAppearanceChanged(const QString &key, const QString &value)
{
    if (fecthing)
        return;

    if (QStringLiteral("background") == key) {
        updateBackgroundStat();
        emit currentWallaperChanged();
    } else if (QStringLiteral("background-add") == key) {
        // find new
        QStringList list = value.split(';');
        for (QString it : list) {
            it = covertUrlToLocalPath(it);
            if (it.isEmpty())
                continue;
            if (wallpaperMap.contains(it))
                emit wallpaperActived(it);
            else
                addNewItem(it);
        }
    } else if (QStringLiteral("background-delete") == key) {
        QStringList list = value.split(';');
        for (QString it : list) {
            it = covertUrlToLocalPath(it);
            if (it.isEmpty() || !wallpaperMap.contains(it))
                continue;

            removeItem(it);
        }
    }
}

void WallpaperProvider::addNewItem(const QString &path)
{
    qInfo() << "add new wallpaper" << path;
    auto it = createItem(path, false);
    if (it.isNull())
        return;

    wallpaper.prepend(it);
    wallpaperMap.insert(path, it);

    updateItem(path);

    emit wallpaperAdded(it);
}

void WallpaperProvider::removeItem(const QString &path)
{
    qInfo() << "wallpaper was deleted" << path;
    auto it = wallpaperMap.value(path);

    wallpaperMap.remove(path);
    wallpaper.removeOne(it);

    emit wallpaperRemoved(path);
}

void WallpaperProvider::updateItem(const QString &path)
{
    auto it = wallpaperMap.value(path);
    if (!it.isNull()) {
        const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
        const QSize size(LISTVIEW_ICON_WIDTH * ratio, LISTVIEW_ICON_HEIGHT * ratio);
        bool pic;
        QVariant val;
        if (InterfaceWorker::generateThumbnail(path, size, pic, val)) {
            if (pic) {
                auto pix = val.value<QPixmap>();
                pix.setDevicePixelRatio(ratio);
                it->pixmap = pix;
            } else {
                it->color = val.value<QColor>();
            }
        }
    }
}

void WallpaperProvider::setWallpaper(const QList<ItemNodePtr> &items)
{
    qDebug() << "get wallpaper list" << items.size() << "current thread" << QThread::currentThread() << "main:" << qApp->thread();
    wallpaper = items;
    wallpaperMap.clear();
    for(auto it : items)
        wallpaperMap.insert(it->item, it);

    wallpaperMtx.unlock();
    fecthing = false;
}

void WallpaperProvider::updateWallpaper(const QMap<QString, bool> &stat)
{
    qDebug() << "update background stat" << stat.size();
    for (auto it = stat.begin(); it != stat.end(); ++it) {
        if (auto ptr = wallpaperMap.value(it.key()))
            ptr->deletable = it.value();
    }
}

void WallpaperProvider::setThumbnail(const QString &item, bool pic, const QVariant &val)
{
    auto node = wallpaperMap.value(item);
    if (node.isNull())
        return;

    if (pic)
        node->pixmap = val.value<QPixmap>();
    else
        node->color = val.value<QColor>();

    emit imageChanged(item);
}

ItemNodePtr WallpaperProvider::createItem(const QString &id, bool del)
{
    ItemNodePtr ret;
    if (id.isEmpty())
        return ret;

    ret = ItemNodePtr(new ItemNode{id, QPixmap(), QColor("#777777"), true, del});
    return ret;
}

QList<ItemNodePtr> InterfaceWorker::processListReply(const QString &reply)
{
    QList<ItemNodePtr> result;
    QJsonDocument doc = QJsonDocument::fromJson(reply.toUtf8());
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        foreach (QJsonValue val, arr) {

            if (!running)
                return result;

            QJsonObject obj = val.toObject();
            QString id = obj["Id"].toString(); //url
            id = covertUrlToLocalPath(id);
            if (auto ptr = WallpaperProvider::createItem(id, obj["Deletable"].toBool()))
                result.append(ptr);
        }
    }

    return result;
}

InterfaceWorker::InterfaceWorker(AppearanceIfs *ifs, QObject *parent)
    : QObject(parent)
    , appearanceIfs(ifs)

{

}

void InterfaceWorker::terminate()
{
    running = false;
}

bool InterfaceWorker::generateThumbnail(const QString &path, const QSize &size, bool &pic, QVariant &val)
{
    bool ret = true;

    QImage image(path);
    if (WallpaperProvider::isColor(path)) {
        pic = false;
        if (image.byteCount() > 0)
            val = QVariant::fromValue(image.pixelColor(0, 0));
        else
            ret = false;
    } else {
        QPixmap pix = QPixmap::fromImage(image.scaled(size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        const QRect r(QPoint(0, 0), size);

        if (pix.width() > size.width() || pix.height() > size.height())
            pix = pix.copy(QRect(pix.rect().center() - r.center(), size));

        pic = true;
        val = QVariant::fromValue(pix);
    }

    return ret;
}

void InterfaceWorker::onListBackground()
{
    qDebug() << "list backgrounds";
    running = true;
    QDBusPendingCall call = appearanceIfs->List("background");
    call.waitForFinished();
    QList<ItemNodePtr> result;
    if (call.isError()) {
        qWarning() << "failed to get all backgrounds: " << call.error().message();
    } else {
        QDBusReply<QString> reply = call.reply();
        QString value = reply.value();
        result = processListReply(value);
    }

    if (running) {
        emit pushBackground(result);

        QStringList paths;
        for (const ItemNodePtr &it : result)
            paths.append(it->item);

        generateThumbnails(paths);
    }

    running = false;
}

void InterfaceWorker::onUpdateStat()
{
    qDebug() << "update background stat";
    QDBusPendingCall call = appearanceIfs->List("background");
    call.waitForFinished();

    if (call.isError()) {
        qWarning() << "failed to get all background stat: " << call.error().message();
    } else {
        QDBusReply<QString> reply = call.reply();
        QString value = reply.value();
        QJsonDocument doc = QJsonDocument::fromJson(value.toUtf8());
        if (doc.isArray()) {
            QJsonArray arr = doc.array();
            QMap<QString, bool> del;
            foreach (QJsonValue val, arr) {
                QJsonObject obj = val.toObject();
                QString id = obj["Id"].toString(); //url
                id = covertUrlToLocalPath(id);
                del.insert(id, obj["Deletable"].toBool());
            }

            emit pushBackgroundStat(del);
        }
    }
}

void InterfaceWorker::generateThumbnails(const QStringList &paths)
{
    const qreal ratio = qApp->primaryScreen()->devicePixelRatio();
    const int itemWidth = static_cast<int>(LISTVIEW_ICON_WIDTH * ratio);
    const int itemHeight = static_cast<int>(LISTVIEW_ICON_HEIGHT * ratio);
    const QSize size(itemWidth, itemHeight);
    for (const QString &path : paths){
        bool pic;
        QVariant val;
        if (generateThumbnail(path, size, pic, val)) {
            if (pic) {
                auto pix = val.value<QPixmap>();
                pix.setDevicePixelRatio(ratio);
                emit pushThumbnail(path, true, QVariant::fromValue(pix));
            } else {
                emit pushThumbnail(path, false, val);
            }
        }

        if (!running)
            return;
    }
}

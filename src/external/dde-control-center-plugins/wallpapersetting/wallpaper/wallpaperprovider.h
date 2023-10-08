// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERPROVIDER_H
#define WALLPAPERPROVIDER_H

#include "common/commondefine.h"

#include <QObject>

#include <com_deepin_wm.h>
#include <com_deepin_daemon_appearance.h>

namespace dfm_wallpapersetting {

using WMInter = com::deepin::wm;
using AppearanceIfs = com::deepin::daemon::Appearance;

class InterfaceWorker : public QObject
{
    Q_OBJECT
public:
    explicit InterfaceWorker(AppearanceIfs *ifs, QObject *parent = nullptr);
    void terminate();
    static bool generateThumbnail(const QString &path, const QSize &size, bool &pic, QVariant &val);
signals:
    void pushBackground(const QList<ItemNodePtr> &items);
    void pushBackgroundStat(const QMap<QString, bool> &stat);
    void pushThumbnail(const QString &path, bool pic, const QVariant &val);
public slots:
    void onListBackground();
    void onUpdateStat();
    void generateThumbnails(const QStringList &paths);
private:
    QList<ItemNodePtr> processListReply(const QString &reply);
private:
    AppearanceIfs *appearanceIfs = nullptr;
    volatile bool running = false;
};

class WallpaperProvider : public QObject
{
    Q_OBJECT
public:
    explicit WallpaperProvider(QObject *parent = nullptr);
    ~WallpaperProvider();
    void fecthData();
    bool waitWallpaper(int ms = 0) const;
    QList<ItemNodePtr> pictures() const;
    QList<ItemNodePtr> colors() const;
    static bool isColor(const QString &path);
    static ItemNodePtr createItem(const QString &id, bool del);
signals:
    void currentWallaperChanged();
    void wallpaperAdded(const ItemNodePtr &it);
    void wallpaperRemoved(const QString &it);
    void wallpaperActived(const QString &it);
    void imageChanged(const QString &item);
public slots:
    QString getSlideshow(const QString &screen);
    void setSlideshow(const QString &screen, const QString &value);
    void setBackgroundForMonitor(const QString &screenName, const QString &path);
    QString getBackgroundForMonitor(const QString &screenName);
    bool deleteBackground(const ItemNodePtr &ptr);
    void setBackgroundToGreeter(const QString &path);
    void updateBackgroundStat();
    void onAppearanceChanged(const QString& key, const QString &value);
protected:
    void addNewItem(const QString &path);
    void removeItem(const QString &path);
    void updateItem(const QString &path);
private slots:
    void setWallpaper(const QList<ItemNodePtr> &items);
    void updateWallpaper(const QMap<QString, bool> &stat);
    void setThumbnail(const QString &item, bool pic, const QVariant &val);
private:
    WMInter *wmInter = nullptr;
    AppearanceIfs *appearanceIfs = nullptr;
    QThread *workThread = nullptr;
    InterfaceWorker *worker = nullptr;
    mutable QMutex wallpaperMtx;
    QList<ItemNodePtr> wallpaper;
    QMap<QString, ItemNodePtr> wallpaperMap;
    bool fecthing = true;
};

}

#endif // WALLPAPERPROVIDER_H

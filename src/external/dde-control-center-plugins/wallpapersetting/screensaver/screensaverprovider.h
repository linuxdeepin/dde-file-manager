// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSAVERPROVIDER_H
#define SCREENSAVERPROVIDER_H

#include "common/commondefine.h"
#include "screensaver/screensaver_interface.h"

#include <QObject>

namespace dfm_wallpapersetting {

struct ScreensaverItem : ItemNode {
    bool configurable;
    QString coverPath;
};

using ScreenSaverIfs = com::deepin::ScreenSaver;

class ScreensaverWorker : public QObject
{
    Q_OBJECT
public:
    explicit ScreensaverWorker(ScreenSaverIfs *ifs, QObject *parent = nullptr);
    void terminate();
signals:
    void pushScreensaver(const QList<ItemNodePtr> &items);
    void pushThumbnail(const QString &item, const QPixmap &pix);
public slots:
    void requestResource();
    void generateThumbnail(const QMap<QString, QString> &paths);
private:
    ScreenSaverIfs *screensaverIfs = nullptr;
    volatile bool running = false;
};

class ScreensaverProvider : public QObject
{
    Q_OBJECT
public:
    explicit ScreensaverProvider(QObject *parent = nullptr);
    ~ScreensaverProvider() override;
    void fecthData();
    bool waitScreensaver(int ms) const;
    inline QList<ItemNodePtr> allScreensaver() {
        return screensavers;
    }
signals:
    void imageChanged(const QString &item);
public slots:
    int getCurrentIdle();
    void setCurrentIdle(int sec);
    bool getIsLock();
    void setIsLock(bool l);
    QString current();
    void setCurrent(const QString &name);
    void configure(const QString &name);
    void startPreview(const QString &name);
    void stopPreview();
private slots:
    void setScreensaver(const QList<ItemNodePtr> &items);
    void setThumbnail(const QString &item, const QPixmap &pix);
private:
    QThread *workThread = nullptr;
    ScreensaverWorker *worker = nullptr;
    ScreenSaverIfs *screensaverIfs = nullptr;
    mutable QMutex screensaverMtx;
    QList<ItemNodePtr> screensavers;
};

}

#endif // SCREENSAVERPROVIDER_H

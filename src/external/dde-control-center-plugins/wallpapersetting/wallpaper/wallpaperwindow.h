// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERWINDOW_H
#define WALLPAPERWINDOW_H

#include "common/previewwidget.h"
#include "modebuttonbox.h"
#include "intervalcombox.h"
#include "common/listview.h"
#include "common/itemmodel.h"
#include "screencombox.h"

#include <QWidget>
#include <QVBoxLayout>

namespace dfm_wallpapersetting {
class WallpaperProvider;
class WallpaperWindow : public QWidget
{
    Q_OBJECT
public:
    explicit WallpaperWindow(QWidget *parent = nullptr);
    ~WallpaperWindow();
    void initialize(WallpaperProvider *ptr);
    void reset(QString screen = "", int mode = -1);
    inline bool isPictureMode() const {
        return modeBox->currentMode() == ModeButtonBox::Picture;
    }
    void scrollToCurrent();
public slots:
    void addNewWallpaper(const ItemNodePtr &ptr);
    void removeWallpaper(const QString &path);
    void activateWallpaper(const QString &path);
    void switchMode(int m);
    void onThmeChanged();
signals:

protected:
    void getNewWallpaper();
    void getNewColor();
    void setWallpaper(const QString &path);
protected slots:
    void itemClicked(const ItemNodePtr &ptr);
    void previewItem(const QString &item, const QColor &bkg);
    void onScreenChanged(const QString &name);
    void onScreenRested();
    void onDeleteItem(const ItemNodePtr &ptr);
    void onViewMenu(const QPoint &pos);
    void repaintItem(const QString &);
private:
    ItemNodePtr createAddButton() const;
private:
    WallpaperProvider *provider = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    PreviewWidget *preview = nullptr;
    ModeButtonBox *modeBox = nullptr;
    IntervalCombox *intervalCombox = nullptr;
    ListView *listView = nullptr;
    ItemModel *itemModel = nullptr;
    ScreenComBox *screenBox = nullptr;
    QString currentScreen;
};
}

#endif // WALLPAPERWINDOW_H

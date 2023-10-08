// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSAVERWINDOW_H
#define SCREENSAVERWINDOW_H

#include "screensaverpreview.h"
#include "common/listview.h"
#include "common/itemmodel.h"
#include "custombutton.h"
#include "idletime.h"

#include <DRegionMonitor>

#include <QWidget>
#include <QVBoxLayout>

namespace dfm_wallpapersetting {

class ScreensaverProvider;

class ScreensaverWindow : public QWidget
{
public:
    explicit ScreensaverWindow(QWidget *parent = nullptr);
    ~ScreensaverWindow();
    void initialize(ScreensaverProvider *ptr);
    void reset();
    void scrollToCurrent();
protected slots:
    void itemClicked(const ItemNodePtr &ptr);
    void previewItem(const QString &img, const QColor &bkg);
    void configItem();
    void startPreview();
    void stopPreview();
    void onThmeChanged();
    void repaintItem(const QString &);
private:
    ScreensaverProvider *provider = nullptr;
    QVBoxLayout *mainLayout = nullptr;
    ScreensaverPreview *preview = nullptr;
    ListView *listView = nullptr;
    ItemModel *itemModel = nullptr;
    CustomButton *customBtn = nullptr;
    IdleTime *idle = nullptr;
    DTK_GUI_NAMESPACE::DRegionMonitor *region = nullptr;
};
}

#endif // SCREENSAVERWINDOW_H

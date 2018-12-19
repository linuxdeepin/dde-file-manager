/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMPUTERVIEW_H
#define COMPUTERVIEW_H

#include <QScrollArea>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QStringList>
#include <QMouseEvent>
#include <QShowEvent>
#include <QIcon>
#include <QFrame>
#include <dflowlayout.h>

#include "fileitem.h"
#include "progressline.h"
#include "durl.h"
#include "dabstractfileinfo.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "dstatusbar.h"
#include "dfmbaseview.h"


DFM_USE_NAMESPACE

class TitleLine: public QFrame
{
    Q_OBJECT

public:
    explicit TitleLine(const QString& title, QWidget *parent = 0);

    void initUI();

protected:
    void resizeEvent(QResizeEvent* event);

private:
    QString m_title;
    QLabel* m_titleLable=NULL;
    QFrame* m_lineLable=NULL;

};

class DScrollArea: public QScrollArea{
public:
    explicit DScrollArea(QWidget* parent =0);

protected:
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
};

class DFMUrlBaseEvent;
class ComputerViewItem: public FileIconItem
{
    Q_OBJECT

public:
    explicit ComputerViewItem(QWidget *parent = 0);

    QIcon getIcon(int size);
    DAbstractFileInfoPointer info() const;
    void setInfo(const DAbstractFileInfoPointer &info);

    UDiskDeviceInfoPointer deviceInfo() const;
    void setDeviceInfo(UDiskDeviceInfoPointer deviceInfo);

    inline ProgressLine* getProgressLine()
    { return m_progressLine; }

    int windowId();

    int iconSize() const;
    void setIconSize(int size);

    void setIconIndex(int index);

    bool checked() const;
    void setChecked(bool checked);

    QString displayName() const;
    void setDisplayName(const QString &displayName);

    QString name() const;
    void setName(const QString &name);

    bool getHasMemoryInfo() const;
    void setHasMemoryInfo(bool hasMemoryInfo);

    int getPixelWidth() const;
    void setPixelWidth(int pixelWidth);
    void updateIconPixelWidth();

    DUrl getUrl() const;

public slots:
    void updateStatus();

signals:
    void checkChanged(bool isChecked);

protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool event(QEvent *event);

private:
    void adjustPosition();
    void setIconSizeState(int iconSize, QIcon::Mode mode = QIcon::Normal);
    void openUrl();

    ProgressLine* m_progressLine;
    QLabel* m_sizeLabel;
    DAbstractFileInfoPointer m_info;
    UDiskDeviceInfoPointer m_deviceInfo;
    int m_iconSize = 64;
    int m_iconIndex = 1;
    bool m_checked = false;
    QString m_name;
    QString m_displayName;
    bool m_hasMemoryInfo = false;
    int m_pixelWidth;
    bool m_isLocked = false;
    QLabel* m_lockedLabel = Q_NULLPTR;
};


class ComputerView : public QFrame, public DFMBaseView
{
    Q_OBJECT

public:
    explicit ComputerView(QWidget *parent = 0);
    ~ComputerView();

    static int ViewInstanceCount;

    static QString scheme();
    QString viewId() const;

    void initData();
    void initUI();
    void initConnect();

    void loadSystemItems();
    void loadNativeItems();
    void loadCustomItems();
    void loadCustomItemsByNameUrl(const QString& id, const QString& rootUrl);
    void updateStatusBar();

    void loadViewState();
    void saveViewState();

    ComputerViewItem* findDeviceViewItemByUrl(const DUrl& url);
    QWidget *widget() const Q_DECL_OVERRIDE;
    DUrl rootUrl() const Q_DECL_OVERRIDE;
    bool setRootUrl(const DUrl &url) Q_DECL_OVERRIDE;

public slots:
    void volumeAdded(UDiskDeviceInfoPointer device);
    void volumeRemoved(UDiskDeviceInfoPointer device);
    void mountAdded(UDiskDeviceInfoPointer device);
    void mountRemoved(UDiskDeviceInfoPointer device);
    void updateComputerItemByDevice(UDiskDeviceInfoPointer device);
    void enlargeIcon();
    void shrinkIcon();
    void resizeAllItemsBySizeIndex(int index);
    void updateItemBySizeIndex(const int& index, ComputerViewItem* item);
    void onRequestEdit(const DFMUrlBaseEvent &event);

protected:
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void showEvent(QShowEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mouseMoveEvent(QMouseEvent *event);

    void setIconSizeBySizeIndex(int index);

private:
    QString m_viewId;
    TitleLine* m_systemTitleLine=NULL;
    DFlowLayout* m_systemFlowLayout = nullptr;
    TitleLine* m_nativeTitleLine=NULL;
    DFlowLayout* m_nativeFlowLayout = nullptr;
    TitleLine* m_removableTitleLine=NULL;
    DFlowLayout* m_removableFlowLayout = nullptr;
    QTimer* m_testTimer;

    DStatusBar* m_statusBar;
    DScrollArea* m_contentArea;

    QList<int> m_iconSizes;
    int m_currentIconSizeIndex = 1;

    QStringList m_systemPathKeys;

    QMap<QString, ComputerViewItem*> m_systemItems;
    QMap<QString, ComputerViewItem*> m_nativeItems;
    QMap<QString, ComputerViewItem*> m_removableItems;
};

#endif // COMPUTERVIEW_H

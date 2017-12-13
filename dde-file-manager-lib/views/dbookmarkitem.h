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

#ifndef DBOOKMARKITEM_H
#define DBOOKMARKITEM_H

#include <QObject>
#include <QGraphicsWidget>
#include <QGraphicsTextItem>
#include <QFont>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDrag>
#include <QLineEdit>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>

#include "durl.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "models/bookmark.h"
#include "themeconfig.h"

#define MARGIN_IMAGE_BOUND 5

class DBookmarkItemGroup;
class UDiskDeviceInfo;
class BookMark;
class DDragWidget;
class DFMEvent;
class DBookmarkMountedIndicatorItem;

DFM_USE_NAMESPACE

class DBookmarkItem : public QGraphicsWidget
{
    Q_OBJECT
public:
    DBookmarkItem(const QString &key = QString());

    DBookmarkItem(UDiskDeviceInfoPointer deviceInfo);

    DBookmarkItem(BookMark * bookmark);

    static int DEFAULT_ICON_SIZE;
    void setDeviceInfo(UDiskDeviceInfoPointer deviceInfo);
    QRectF boundingRect() const;
    void setTightMode(bool v);
    bool isTightModel();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setPressedIcon(const QString& iconPath);
    void setPressedIcon(const QIcon& icon);
    void setHoverIcon(const QString& iconPath);
    void setHoverIcon(const QIcon& icon);
    void setReleaseIcon(const QString& iconPath);
    void setReleaseIcon(const QIcon& icon);
    void setCheckedIcon(const QString& iconPath);
    void setCheckedIcon(const QIcon& icon);

    void setDraggable(bool b);
    void setBounds(int x, int y, int w, int h);
    void setHoverEnableFlag(bool flag);
    void setAdjust(qreal value);
    bool isPressed();
    bool isHovered();
    void setHovered(bool flag);
    void setBackgroundEnable(bool flag);
    int boundX();
    int boundY();
    int boundWidth();
    int boundHeight();
    QString text();
    void setText(const QString & text);

    void setPress(bool b);
    void setPressBackgroundEnable(bool b);
    void setReleaseBackgroundEnable(bool b);
    void setHoverBackgroundEnable(bool b);
    void setHighlightDiskBackgroundEnable(bool b);
    void setHighlightDisk(bool isHighlight);
    bool isHighlightDisk();
    void setUrl(const DUrl &url);
    DUrl getUrl();
    void setCheckable(bool b);
    void setChecked(bool b);
    bool isChecked();
    void setItemGroup(DBookmarkItemGroup * group);
    void setWidth(double w);
    void setHeight(double h);
    void setDefaultItem(bool v);
    bool isDefaultItem();
    bool isDiskItem();
    QPixmap toPixmap();
    bool isMounted();
    void setMounted(bool v);
    void setDeviceLabel(const QString &label);
    QString getDeviceLabel();
    void setDeviceID(const QString &deviceID);
    QString getDeviceID();
    int windowId();
    BookMarkPointer getBookmarkModel();
    void setBookmarkModel(BookMarkPointer bookmark);

    static DBookmarkMountedIndicatorItem* makeMountBookmark(DBookmarkItem* parentItem);
    void editMode();
    void updateMountIndicator();

    bool isMountedIndicator() const;
    void setIsMountedIndicator(bool isMountedIndicator);

    bool getMountBookmark() const;
    void setMountBookmark(bool mountBookmark);

    bool getIsCustomBookmark() const;
    void setIsCustomBookmark(bool isCustomBookmark);

    void changeIconThroughColor(const QColor& color)noexcept;
signals:

    void clicked();
    void doubleClicked();

    void dragEntered();
    void dragLeft();
    void dropped();
    void dragFinished(const QPointF &point, const QPointF &scenePoint, DBookmarkItem * item);

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QGraphicsSceneDragDropEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;
private:
    void init();
    QPixmap getPixmap(const QString &key, ThemeConfig::State state = ThemeConfig::Normal) const;

    QString m_key;

    QLineEdit * m_lineEdit;
    QGraphicsProxyWidget * m_widget = NULL;
    UDiskDeviceInfoPointer m_deviceInfo;
    DBookmarkItemGroup * m_group = NULL;

    DBookmarkMountedIndicatorItem* m_mountBookmarkItem = NULL;

    /* default */
    bool m_isDraggable = false;
    bool m_pressBackgroundEnabled = false;
    bool m_releaseBackgroundEnabled = false;
    bool m_hoverBackgroundEnabled = false;
    bool m_highlightDiskBackgroundEnabled = false;
    int m_xOffset = 0;
    int m_yOffset = 0;
    int m_xPos = 0;
    int m_yPos = 0;
    int m_xPress = 0;
    int m_yPress = 0;
    int m_xOffsetImage = 0;
    int m_yOffsetImage = 0;
    int m_x_axis = 0;
    int m_y_axis = 0;
    int m_width = 0;
    int m_height = 0;
    int m_text_x = 0;
    int m_text_y = 0;
    bool m_checkable = false;
    bool m_checked = false;
    bool m_pressed = false;
    bool m_hovered = false;
    bool m_isHighlightDisk = false;
    bool m_backgroundEnabled = false;
    bool m_isMenuOpened = false;
    bool m_isTightMode = false;
    QFont m_font;
    qreal m_adjust = 0;
    DUrl m_url;
//    QPixmap m_pressImage;
//    QPixmap m_releaseImage;
//    QPixmap m_hoverImage;
//    QPixmap m_checkedImage;
//    QPixmap m_pressImageBig;
//    QPixmap m_releaseImageBig;
//    QPixmap m_hoverImageBig;
//    QPixmap m_checkedImageBig;
    QString m_textContent;
//    QColor m_hoverBackgroundColor;
//    QColor m_releaseBackgroundColor;
//    QColor m_pressBackgroundColor;
//    QColor m_highlightDiskBackgroundColor;
//    QColor m_textColor;
//    QColor m_textHoverColor;
//    QColor m_textPressColor;
//    QColor m_textCheckedColor;
//    QColor m_backGroundColor;
    DDragWidget * drag = NULL;
    /* bookmark */
    bool m_isDefault = false;
    bool m_isMountedIndicator = false;

    /* device */
    bool m_isDisk = false;
    bool m_isMounted = false;
    QString m_deviceLabel;
    QString m_deviceID;

    bool m_isCustomBookmark = false;
    bool m_mountBookmark = false;

    /*bookmark model map */
    BookMarkPointer m_bookmarkModel;

public:
    static std::atomic<DBookmarkItem*> ClickedItem;

public slots:
    void editFinished();
    void checkMountedItem(const DFMEvent& event);
    void playAnimation();
};

#endif // DBOOKMARKITEM_H

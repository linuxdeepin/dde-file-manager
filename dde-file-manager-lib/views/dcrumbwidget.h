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

#ifndef DCRUMBWIDGET_H
#define DCRUMBWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

#include "deviceinfo/udiskdeviceinfo.h"
#include "durl.h"
#include "dbusinterface/dbustype.h"

class DFMEvent;
class DCrumbButton;
class DStateButton;
class DCrumbWidget;

class ListWidgetPrivate : public QListWidget
{
public:
    ListWidgetPrivate(DCrumbWidget * crumbWidget);

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event)  Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    DCrumbWidget * m_crumbWidget;
    QPoint oldGlobalPos;
};

class DCrumbWidget : public QFrame
{
    Q_OBJECT
public:
    explicit DCrumbWidget(QWidget *parent = 0);
    void addCrumb(const QStringList &list);
    void setCrumb(const DUrl &url);
    DUrl backUrl();
    void clear();
    QString path();
    DUrl getUrl();
    DUrl getCurrentUrl();
private:
    void addRecentCrumb();
    void addComputerCrumb();
    void addTrashCrumb();
    void addNetworkCrumb();
    void addSmbCrumb();
    void addUserShareCrumb();
    void addPluginViewCrumb(const DUrl &url);

    DCrumbButton* createDeviceCrumbButtonByType(UDiskDeviceInfo::MediaType type, const QString& mountPoint);

    void addCrumbs(const DUrl & url);
    void initUI();
    void prepareCrumbs(const DUrl &path);
    bool hasPath(const QString& path);
    bool isInHome(const QString& path);
    bool isHomeFolder(const QString& path);
    bool isInDevice(const QString& path);
    bool isDeviceFolder(const QString& path);
    bool isRootFolder(QString path);
    void createCrumbs();
    void createArrows();
    void checkArrows();
    QHBoxLayout * m_buttonLayout;
    QButtonGroup m_group;
    DUrl m_url;
    QString m_homePath;
    QPushButton * m_leftArrow = NULL;
    QPushButton * m_rightArrow = NULL;
    ListWidgetPrivate * m_listWidget = NULL;
    QList<QPushButton *> m_buttons;
    bool m_needArrows = false;
    int m_prevCheckedId = -1;
    QList<QListWidgetItem*> m_items;
    int m_crumbTotalLen = 0;

public slots:
    void buttonPressed();
    void crumbMoveToLeft();
    void crumbMoveToRight();
signals:
    void crumbSelected(const DFMEvent &event);
    void searchBarActivated();

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
};

#endif // DCRUMBWIDGET_H

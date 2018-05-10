/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbitem.h"
#include "dfmcrumbinterface.h"

#include "dfileservices.h"

#include "views/themeconfig.h"
#include "deviceinfo/udisklistener.h"

#include <QPainter>
#include <DThemeManager>
#include <QMimeData>

#include <QDebug>

#include "singleton.h"

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMCrumbItemPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbItem)

public:
    DFMCrumbItemPrivate(DFMCrumbItem *qq);

    QPixmap icon() const;
    ThemeConfig::State getState() const;

    QPoint clickedPos;
    CrumbData data;

    DFMCrumbItem *q_ptr = nullptr;

private:

};

DFMCrumbItemPrivate::DFMCrumbItemPrivate(DFMCrumbItem *qq)
    : q_ptr(qq)
{
    qq->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    qq->setObjectName("DCrumbButton");

    qq->connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, qq, [this, qq](){
        qq->setIcon(icon());
    });
}

QPixmap DFMCrumbItemPrivate::icon() const
{
    return ThemeConfig::instace()->pixmap(data.iconName, data.iconKey, getState());
}

ThemeConfig::State DFMCrumbItemPrivate::getState() const
{
    Q_Q(const DFMCrumbItem);

    if (q->isChecked()) {
        return ThemeConfig::Checked;
    }

    return ThemeConfig::Normal;
}


// blumia: We may need remove this and only keep the construct function
//         with `CrumbData` as argument.
//         We can not always get a proper display name through a url,
//         so that's what
//         DAbstractFileInfoPointer::fileDisplayName()
DFMCrumbItem::DFMCrumbItem(DUrl url, QWidget* parent)
    : QPushButton(parent)
    , d_ptr(new DFMCrumbItemPrivate(this))
{
    Q_UNUSED(url);
    //this->setStyleSheet("background: red");
    setText("Mamacat Placeholder Long Text");
    setUrl(DUrl::fromTrashFile("/"));
    this->setIconFromThemeConfig("CrumbIconButton.Home");
}

DFMCrumbItem::DFMCrumbItem(CrumbData data, QWidget* parent)
    : QPushButton(parent)
    , d_ptr(new DFMCrumbItemPrivate(this))
{
    d_ptr->data = data;

    if (!data.displayText.isEmpty()) {
        this->setText(data.displayText);
    }

    if (!data.iconName.isEmpty()) {
        this->setIconFromThemeConfig(data.iconName, data.iconKey);
    }
}

DFMCrumbItem::~DFMCrumbItem()
{

}

DUrl DFMCrumbItem::url() const
{
    Q_D(const DFMCrumbItem);
    return d->data.url;
}

void DFMCrumbItem::setText(const QString &text)
{
    Q_D(DFMCrumbItem);
    d->data.displayText = text;
    QPushButton::setText(text);
}

void DFMCrumbItem::setUrl(const DUrl &url)
{
    Q_D(DFMCrumbItem);
    d->data.url = url;
}

void DFMCrumbItem::setIconFromThemeConfig(const QString &group, const QString &key)
{
    Q_D(DFMCrumbItem);

    d->data.iconName = group;
    d->data.iconKey = key;

    // Do widget UI update.
    this->setIcon(d->icon());
}

Qt::DropAction DFMCrumbItem::canDropMimeData(const QMimeData *data, Qt::DropActions actions) const
{
    Q_D(const DFMCrumbItem);

    if (data->urls().empty()) {
        return Qt::IgnoreAction;
    }

    for (const DUrl &url : data->urls()) {
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, url);
        if (!fileInfo || !fileInfo->isReadable()) {
            return Qt::IgnoreAction;
        }
    }

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, d->data.url);

    if (!info || !info->canDrop()) {
        return Qt::IgnoreAction;
    }

    const Qt::DropActions support_actions = info->supportedDropActions() & actions;

    if (support_actions.testFlag(Qt::CopyAction)) {
        return Qt::CopyAction;
    }

    if (support_actions.testFlag(Qt::MoveAction)) {
        return Qt::MoveAction;
    }

    if (support_actions.testFlag(Qt::LinkAction)) {
        return Qt::LinkAction;
    }

    return Qt::IgnoreAction;
}

bool DFMCrumbItem::dropMimeData(const QMimeData *data, Qt::DropAction action) const
{
    Q_D(const DFMCrumbItem);

    DUrl destUrl = d->data.url;
    DUrlList oriUrlList = DUrl::fromQUrlList(data->urls());
    const DAbstractFileInfoPointer &destInfo = DFileService::instance()->createFileInfo(this, destUrl);

    // convert destnation url to real path if it's a symbol link.
    if (destInfo->isSymLink()) {
        destUrl = destInfo->rootSymLinkTarget();
    }

    switch (action) {
    case Qt::CopyAction:
        if (oriUrlList.count() > 0) {
            bool isInSameDevice = Singleton<UDiskListener>::instance()->isInSameDevice(oriUrlList.at(0).toLocalFile(), destUrl.toLocalFile());
            if (isInSameDevice && !DFMGlobal::keyCtrlIsPressed()) {
                DFileService::instance()->pasteFile(this, DFMGlobal::CutAction, destUrl, oriUrlList);
            } else {
                DFileService::instance()->pasteFile(this, DFMGlobal::CopyAction, destUrl, oriUrlList);
            }
        }
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        DFileService::instance()->pasteFile(this, DFMGlobal::CutAction, destUrl, oriUrlList);
        break;
    default:
        return false;
    }

    return true;
}

void DFMCrumbItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbItem);
    d->clickedPos = event->globalPos();

    return;
}

void DFMCrumbItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbItem);

    if (d->clickedPos == event->globalPos() && !d->data.url.isEmpty()) {
        emit crumbClicked();
    }

    QWidget::mouseReleaseEvent(event);
}

void DFMCrumbItem::paintEvent(QPaintEvent *event)
{
    QPainter pa(this);

    //pa.fillRect(rect(), Qt::blue);

    QPushButton::paintEvent(event);
}

DFM_END_NAMESPACE

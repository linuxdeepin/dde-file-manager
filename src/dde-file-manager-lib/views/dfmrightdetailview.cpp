/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#include "dfmrightdetailview.h"
#include "dtagactionwidget.h"
#include "dfileservices.h"
#include "views/dfmfilebasicinfowidget.h"
#include "app/define.h"
#include "dfmtagwidget.h"
#include "singleton.h"
#include "controllers/pathmanager.h"
#include "controllers/vaultcontroller.h"
#include "accessibility/ac-lib-file-manager.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <dobject.h>
#include <dtkwidget_global.h>
#include <QScrollArea>
#include <DHorizontalLine>

DWIDGET_USE_NAMESPACE

class DFMRightDetailViewPrivate
{
public:
    explicit DFMRightDetailViewPrivate(DFMRightDetailView *qq, const DUrl &url);
    virtual ~DFMRightDetailViewPrivate();

    DUrl m_url;
    QVBoxLayout *mainLayout = nullptr;
    QLabel *iconLabel = nullptr;
    QFrame *baseInfoWidget = nullptr;
    QFrame *separatorLine1 = nullptr;
    QFrame *separatorLine2 = nullptr;
    DFMTagWidget *tagInfoWidget = nullptr;

    QScrollArea *scrollArea = nullptr;

    DFMRightDetailView *q_ptr = nullptr;
    D_DECLARE_PUBLIC(DFMRightDetailView)
};

DFMRightDetailViewPrivate::DFMRightDetailViewPrivate(DFMRightDetailView *qq, const DUrl &url)
    : m_url(url), q_ptr(qq)
{
}

DFMRightDetailViewPrivate::~DFMRightDetailViewPrivate()
{
}

DFMRightDetailView::DFMRightDetailView(const DUrl &fileUrl, QWidget *parent)
    : QFrame(parent), d_private(new DFMRightDetailViewPrivate(this, fileUrl))
{
    AC_SET_OBJECT_NAME(this, AC_DM_RIGHT_VIEW_DETAIL_VIEW);
    AC_SET_ACCESSIBLE_NAME(this, AC_DM_RIGHT_VIEW_DETAIL_VIEW);

    initUI();
    setUrl(fileUrl);
}

DFMRightDetailView::~DFMRightDetailView()
{
}

static QFrame *createLine()
{
    DHorizontalLine *line = new DHorizontalLine();
    AC_SET_OBJECT_NAME(line, AC_DM_RIGHT_VIEW_MAIN_FRAME_LINE);
    AC_SET_ACCESSIBLE_NAME(line, AC_DM_RIGHT_VIEW_MAIN_FRAME_LINE);
    line->setFixedHeight(1);
    line->setWindowFlags(Qt::WindowTransparentForInput);
    return line;
}

void DFMRightDetailView::initUI()
{
    Q_D(DFMRightDetailView);
    d->scrollArea = new QScrollArea(this);
    d->scrollArea->setObjectName("DFMRightDetailView-QScrollArea");
    AC_SET_ACCESSIBLE_NAME(d->scrollArea, AC_DM_RIGHT_VIEW_MAIN_FRAME_SCROLLAREA);
    d->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    d->scrollArea->setAlignment(Qt::AlignTop);
    d->scrollArea->setFrameShape(Shape::NoFrame);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(d->scrollArea);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);

    d->mainLayout = new QVBoxLayout;
    d->mainLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    d->mainLayout->setSpacing(5);

    QFrame *mainFrame = new QFrame;
    AC_SET_OBJECT_NAME(mainFrame, AC_DM_RIGHT_VIEW_MAIN_FRAME);
    AC_SET_ACCESSIBLE_NAME(mainFrame, AC_DM_RIGHT_VIEW_MAIN_FRAME);
    mainFrame->setLayout(d->mainLayout);

    d->iconLabel = new QLabel(this);
    AC_SET_OBJECT_NAME(d->iconLabel, AC_DM_RIGHT_VIEW_MAIN_FRAME_ICON_LABEL);
    AC_SET_ACCESSIBLE_NAME(d->iconLabel, AC_DM_RIGHT_VIEW_MAIN_FRAME_ICON_LABEL);

    d->iconLabel->setFixedSize(160, 160);
    d->mainLayout->addWidget(d->iconLabel, 1, Qt::AlignHCenter);

    d->mainLayout->addWidget(d->separatorLine1 = createLine());
    d->mainLayout->addWidget(d->separatorLine2 = createLine());
    d->separatorLine1->setVisible(false);
    d->separatorLine2->setVisible(false);

    initTagWidget();

    // 默认值在此场景下无效
    d->mainLayout->addStretch(9999);

    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->scrollArea->setWidget(mainFrame);
    d->scrollArea->setWidgetResizable(true);
}

void DFMRightDetailView::initTagWidget()
{
    Q_D(DFMRightDetailView);
    d->tagInfoWidget = new DFMTagWidget(d->m_url, this);
    d->tagInfoWidget->tagTitle()->setHidden(true);
    d->tagInfoWidget->tagLeftTitle()->setHidden(false);
    d->tagInfoWidget->setMaximumHeight(100);
    d->mainLayout->addWidget(d->tagInfoWidget);
    d->tagInfoWidget->setVisible(false);
}

void DFMRightDetailView::setUrl(const DUrl &url)
{
    Q_D(DFMRightDetailView);

    if (!url.isValid())
        return;

    d->m_url = url;
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, d->m_url);
    if (d->scrollArea)
        d->scrollArea->setVisible(fileInfo);
    if (d->separatorLine1)
        d->separatorLine1->setVisible(fileInfo);
    if (!fileInfo)
        return;
    bool shouldShowTags = fileInfo->canTag();
    setTagWidgetVisible(shouldShowTags);
    if (d->tagInfoWidget) {
        d->tagInfoWidget->loadTags(d->m_url);
    }
    if (d->iconLabel) {
        QString iconName;
        if (url == DUrl(RECENT_ROOT)) {
            iconName = systemPathManager->getSystemPathIconName("Recent");
        } else if (url == DUrl(TRASH_ROOT)) {
            iconName = systemPathManager->getSystemPathIconName("Trash");
        } else if (url.isNetWorkFile() || url.isSMBFile()) {
            iconName = systemPathManager->getSystemPathIconName("Network");
        } else if (url.isUserShareFile()) {
            iconName = systemPathManager->getSystemPathIconName("UserShare");
        } else if (VaultController::isRootDirectory(url.path())) {
            iconName = systemPathManager->getSystemPathIconName("Vault");
        }
        QIcon fileIcon = iconName.isEmpty() ? fileInfo->fileIcon() : QIcon::fromTheme(iconName);
        QList<QSize> iconSizeList = fileIcon.availableSizes();
        //缩放处理的图标,系统图标都是成队出现
        if (iconSizeList.size() == 1
            && iconSizeList.first().width() != iconSizeList.first().height()) {
            QSize avaliSize = iconSizeList.first();
            QSize targetSize = avaliSize.scaled(d->iconLabel->height(),
                                                d->iconLabel->height(),
                                                Qt::KeepAspectRatio);
            d->iconLabel->setPixmap(fileIcon.pixmap(targetSize));
            //自适应layout居中
            d->iconLabel->setFixedWidth(targetSize.width());
        } else {
            d->iconLabel->setFixedSize(d->iconLabel->height(), d->iconLabel->height());
            d->iconLabel->setPixmap(fileIcon.pixmap(d->iconLabel->size()));
        }
    }

    if (d->baseInfoWidget) {
        d->mainLayout->removeWidget(d->baseInfoWidget);
        d->baseInfoWidget->setHidden(true);
        d->baseInfoWidget->deleteLater();
    }

    DFMFileBasicInfoWidget *basicInfoWidget = new DFMFileBasicInfoWidget(this);
    d->baseInfoWidget = basicInfoWidget;
    basicInfoWidget->setShowFileName(true);
    basicInfoWidget->setShowMediaInfo(true);
    basicInfoWidget->setShowSummary(fileInfo->isDir());
    basicInfoWidget->setUrl(d->m_url);
    d->mainLayout->insertWidget(2, d->baseInfoWidget);
}

void DFMRightDetailView::setTagWidgetVisible(bool visible)
{
    Q_D(DFMRightDetailView);
    if (d->tagInfoWidget) {
        d->tagInfoWidget->setVisible(visible);
    }

    if (d->separatorLine2) {
        d->separatorLine2->setVisible(visible);
    }
}

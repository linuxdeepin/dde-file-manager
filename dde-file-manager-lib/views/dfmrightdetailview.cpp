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
#include "app/define.h"
#include "singleton.h"
#include "shutil/mimetypedisplaymanager.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <dcrumbedit.h>
#include <dobject.h>
#include <dtkwidget_global.h>
#include<tag/tagmanager.h>
#include <QScrollArea>

DWIDGET_USE_NAMESPACE

class DFMRightDetailViewPrivate{
public:
    explicit DFMRightDetailViewPrivate(DFMRightDetailView *qq, const DUrl& url);
    virtual ~DFMRightDetailViewPrivate();

    DUrl    m_url;
    QVBoxLayout *mainLayout  {nullptr};
    QLabel      *iconLabel  {nullptr};
    QFrame      *baseInfoWidget {nullptr};
    QFormLayout *baseInfoLayout {nullptr};
    QLabel      *nameLabel{ nullptr };
    QLabel      *typeLabel{ nullptr };
    QLabel      *sizeLabel{ nullptr };
    QLabel      *pixelLabel{ nullptr };
    QLabel      *createTimeLabel{ nullptr };
    QLabel      *modifyTimeLabel{ nullptr };
    DCrumbEdit  *tagNamesCrumbEdit{ nullptr };
    QFrame      *tagInfoWidget{ nullptr };
    QScrollArea *scrollArea{ nullptr };

    DFMRightDetailView *q_ptr{ nullptr };
    D_DECLARE_PUBLIC(DFMRightDetailView)
};

DFMRightDetailViewPrivate::DFMRightDetailViewPrivate(DFMRightDetailView *qq, const DUrl& url)
    :m_url(url)
    ,q_ptr(qq)
{

}

DFMRightDetailViewPrivate::~DFMRightDetailViewPrivate()
{

}

DFMRightDetailView::DFMRightDetailView(const DUrl &fileUrl, QWidget *parent)
    :QFrame(parent)
    , d_private(new DFMRightDetailViewPrivate(this, fileUrl))
{
    initUI();
    setUrl(fileUrl);
}

DFMRightDetailView::~DFMRightDetailView()
{

}

static QLabel *createKeyLabel(QString text, QWidget *parent = nullptr)
{
    QLabel *key = new QLabel(text, parent);
    key->setObjectName("SectionKeyLabel");
    key->setMinimumWidth(100);
    key->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    //key->setStyleSheet("background-color:#666666;");
    return key;
}

static QLabel *createValueLabel(QString text, QWidget *parent = nullptr)
{
    QLabel *value = new QLabel(text, parent);
    value->setObjectName("SectionValueLabel");
    value->setMinimumWidth(100);
    //value->setStyleSheet("background-color:#999999;");
    value->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    return value;
}

static QFrame* createLine()
{
    auto line = new QFrame;
    line->setFixedHeight(1);
    line->setStyleSheet("background-color:#DFDFE0;");
    return line;
}

void DFMRightDetailView::initUI()
{
    Q_D(DFMRightDetailView);
    d->mainLayout = new QVBoxLayout;
    d->mainLayout->setAlignment(Qt::AlignCenter);
    d->mainLayout->setSpacing(10);
    setLayout(d->mainLayout);

    d->iconLabel = new QLabel(this);
    d->iconLabel->setFixedHeight(256);
    d->mainLayout->addWidget(d->iconLabel, 1, Qt::AlignHCenter);

    d->mainLayout->addWidget(createLine());

    d->baseInfoLayout = new QFormLayout;
    d->baseInfoLayout->setHorizontalSpacing(10);
    d->baseInfoLayout->setVerticalSpacing(10);

    d->nameLabel = createValueLabel("-");
    d->typeLabel = createValueLabel("-");
    d->sizeLabel = createValueLabel("-");
    d->pixelLabel = createValueLabel("-");
    d->createTimeLabel = createValueLabel("-");
    d->modifyTimeLabel = createValueLabel("-");

    d->baseInfoLayout->addRow(createKeyLabel("name"), d->nameLabel);
    d->baseInfoLayout->addRow(createKeyLabel("size"), d->sizeLabel);
    d->baseInfoLayout->addRow(createKeyLabel("pixel"), d->pixelLabel);
    d->baseInfoLayout->addRow(createKeyLabel("type"), d->typeLabel);
    d->baseInfoLayout->addRow(createKeyLabel("created time"), d->createTimeLabel);
    d->baseInfoLayout->addRow(createKeyLabel("modified time"), d->modifyTimeLabel);

    d->baseInfoWidget = new QFrame;
    d->baseInfoWidget->setLayout(d->baseInfoLayout);

    d->baseInfoLayout->addRow(createLine());

    d->tagInfoWidget = new QFrame;
    auto tagHolder = new QVBoxLayout;
    d->tagInfoWidget->setLayout(tagHolder);

    auto hl = new QHBoxLayout;
    QLabel *tagLable = createKeyLabel("tag");
    auto tagWidget =  new DTagActionWidget;
    tagWidget->setEnabled(false);
    tagWidget->setToolTipVisible(false);
    hl->addWidget(tagLable);
    hl->addWidget(tagWidget);

    tagHolder->addLayout(hl);
    d->tagNamesCrumbEdit = new DCrumbEdit(this);
    d->tagNamesCrumbEdit->setEnabled(false);
    tagHolder->addWidget(d->tagNamesCrumbEdit);
    d->baseInfoLayout->addRow(d->tagInfoWidget);

    d->scrollArea = new QScrollArea(this);
    d->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    d->scrollArea->setAlignment(Qt::AlignTop);
    d->scrollArea->setWidget(d->baseInfoWidget);
    d->scrollArea->setVisible(false);

    d->mainLayout->addWidget(d->scrollArea, 1, Qt::AlignHCenter);
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

    if (fileInfo) {
        if (d->iconLabel)
            d->iconLabel->setPixmap(fileInfo->fileIcon().pixmap(256, 256));

        if (mimeTypeDisplayManager->displayNameToEnum(fileInfo->mimeTypeName()) == DAbstractFileInfo::FileType::Images) {
            QPixmap pixmap;
            pixmap.load(fileInfo->filePath());
            if (!pixmap.isNull() && d->pixelLabel) {
                QString text = QString("%1X%2").arg(pixmap.width()).arg(pixmap.height());
                d->pixelLabel->setText(text);
            }
        }

        if (d->nameLabel) {
            QString text = fileInfo->fileDisplayName();
            d->nameLabel->setText(d->nameLabel->fontMetrics().elidedText(text, Qt::ElideMiddle, d->nameLabel->width()));
            d->nameLabel->setToolTip(text);
        }
        if (d->typeLabel)
            d->typeLabel->setText(fileInfo->mimeTypeDisplayName());
        if (d->sizeLabel)
            d->sizeLabel->setText(fileInfo->sizeDisplayName());
        if (d->createTimeLabel)
            d->createTimeLabel->setText(fileInfo->createdDisplayName());
        if (d->modifyTimeLabel)
            d->modifyTimeLabel->setText(fileInfo->lastModifiedDisplayName());

        const QStringList tag_name_list = TagManager::instance()->getTagsThroughFiles({url});
        QMap<QString, QColor> nameColors = TagManager::instance()->getTagColor({tag_name_list});
        if (d->tagNamesCrumbEdit) {
            d->tagNamesCrumbEdit->clear();
            for(auto it = nameColors.begin();it != nameColors.end(); ++it) {
                DCrumbTextFormat format = d->tagNamesCrumbEdit->makeTextFormat();
                format.setText(it.key());
                format.setBackground(QBrush(it.value()));
                format.setBackgroundRadius(5);
                d->tagNamesCrumbEdit->appendCrumb(format);
            }
        }
        if (d->tagInfoWidget)
            d->tagInfoWidget->setHidden(tag_name_list.isEmpty());
    }
}

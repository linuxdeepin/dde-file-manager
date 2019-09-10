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

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <dcrumbedit.h>
#include <dobject.h>
#include <dtkwidget_global.h>
#include <tag/tagmanager.h>
#include <QScrollArea>
#include <dabstractfilewatcher.h>


DWIDGET_USE_NAMESPACE

class DFMRightDetailViewPrivate{
public:
    explicit DFMRightDetailViewPrivate(DFMRightDetailView *qq, const DUrl& url);
    virtual ~DFMRightDetailViewPrivate();

    DUrl    m_url;
    QVBoxLayout *mainLayout  {nullptr};
    QLabel      *iconLabel  {nullptr};
    QFrame      *baseInfoWidget {nullptr};
    DCrumbEdit  *tagNamesCrumbEdit{ nullptr };
    DTagActionWidget *tagWidget{ nullptr };
    QFrame      *tagInfoWidget{ nullptr };
    QScrollArea *scrollArea{ nullptr };
    DAbstractFileWatcher *devicesWatcher{ nullptr };

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
    d->scrollArea = new QScrollArea(this);
    d->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    d->scrollArea->setAlignment(Qt::AlignTop);
    d->scrollArea->setFrameShape(Shape::NoFrame);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(d->scrollArea);
    setLayout(layout);

    d->mainLayout = new QVBoxLayout;
    d->mainLayout->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    d->mainLayout->setSpacing(10);

    QFrame *mainFrame = new QFrame;
    mainFrame->setLayout(d->mainLayout);

    d->iconLabel = new QLabel(this);
    d->iconLabel->setFixedHeight(160);
    d->mainLayout->addWidget(d->iconLabel, 1, Qt::AlignHCenter);

    d->mainLayout->addWidget(createLine());

    initTagWidget();

    d->mainLayout->addStretch();

    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->scrollArea->setWidget(mainFrame);
    d->scrollArea->setWidgetResizable(true);
}

void DFMRightDetailView::initTagWidget()
{
    Q_D(DFMRightDetailView);

    d->tagInfoWidget = new QFrame(this);
    QVBoxLayout *tagHolder = new QVBoxLayout;
    d->tagInfoWidget->setLayout(tagHolder);

    QHBoxLayout *hl = new QHBoxLayout;
    QLabel *tagLable = new QLabel("tag", this);
    tagLable->setMinimumWidth(100);
    tagLable->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    d->tagWidget =  new DTagActionWidget(d->tagInfoWidget);
    QObject::connect(d->tagWidget, &DTagActionWidget::checkedColorChanged, this, [this, d](const QColor &color){
        const QStringList tag_name_list = TagManager::instance()->getTagsThroughFiles({d->m_url});
        QMap<QString, QColor> nameColors = TagManager::instance()->getTagColor({tag_name_list});
        DUrlList urlList{d->m_url};
        QList<QColor> colors{ d->tagWidget->checkedColorList() };
        QSet<QString> dirtyTagFilter = TagManager::instance()->allTagOfDefaultColors();

        QSet<QString> sameColors = nameColors.keys(color).toSet();
        // 当有多个相同颜色名字不同的tag时， 取消tag优先取消默认颜色
        if (sameColors.count()>0 && !colors.contains(color) && !sameColors.intersects(dirtyTagFilter)) {
            dirtyTagFilter << *sameColors.begin();
        }

        QStringList new_tagNames;
        for (const QColor &color : colors) {
            QString tag_name = TagManager::instance()->getTagNameThroughColor(color);
            if (tag_name.isEmpty()) {
                continue;
            }
            new_tagNames << tag_name;
        }

        DFileService::instance()->makeTagsOfFiles(nullptr,urlList, new_tagNames, dirtyTagFilter);
        LoadFileTags();
    });


    //tagWidget->setEnabled(false);
    d->tagWidget->setToolTipVisible(false);
    hl->addWidget(tagLable);
    hl->addWidget(d->tagWidget);
    tagHolder->addLayout(hl);

    d->tagNamesCrumbEdit = new DCrumbEdit(this);
    d->tagNamesCrumbEdit->setEnabled(false);
    tagHolder->addWidget(d->tagNamesCrumbEdit);
    tagHolder->addStretch();
    d->tagNamesCrumbEdit->setMaximumHeight(100);
    d->tagNamesCrumbEdit->setHidden(true);

    d->mainLayout->addWidget(d->tagInfoWidget);
}

void DFMRightDetailView::LoadFileTags()
{
    Q_D(DFMRightDetailView);
    const QStringList tag_name_list = TagManager::instance()->getTagsThroughFiles({d->m_url});
    QMap<QString, QColor> nameColors = TagManager::instance()->getTagColor({tag_name_list});
    QList<QColor>  selectColors;
    if (d->tagNamesCrumbEdit) {
        d->tagNamesCrumbEdit->setPlainText("");
        for(auto it = nameColors.begin();it != nameColors.end(); ++it) {
            DCrumbTextFormat format = d->tagNamesCrumbEdit->makeTextFormat();
            format.setText(it.key());
            selectColors << it.value();
            format.setBackground(QBrush(it.value()));
            format.setBackgroundRadius(5);
            d->tagNamesCrumbEdit->appendCrumb(format);
        }
    }

    if (d->tagWidget)
        d->tagWidget->setCheckedColorList(selectColors);

    if (d->tagNamesCrumbEdit)
        d->tagNamesCrumbEdit->setHidden(tag_name_list.isEmpty());
}

void DFMRightDetailView::setUrl(const DUrl &url)
{
    Q_D(DFMRightDetailView);

    if (!url.isValid())
        return;

    d->m_url = url;

    if (d->devicesWatcher) {
        d->devicesWatcher->stopWatcher();
        d->devicesWatcher->deleteLater(); // should we?
        d->devicesWatcher = nullptr;
    }

    d->devicesWatcher = DFileService::instance()->createFileWatcher(nullptr, d->m_url, this);
    if (d->devicesWatcher) {
        d->devicesWatcher->startWatcher();

        connect(d->devicesWatcher, &DAbstractFileWatcher::fileAttributeChanged, this, [this, d](const DUrl &url) {
            if (url == d->m_url){
                LoadFileTags();
            }
        });
    }


    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, d->m_url);
    if (d->scrollArea)
        d->scrollArea->setVisible(fileInfo);

    if (fileInfo) {
        if (d->iconLabel)
            d->iconLabel->setPixmap(fileInfo->fileIcon().pixmap(256, 160));

        if (d->baseInfoWidget){
            d->mainLayout->removeWidget(d->baseInfoWidget);
            d->baseInfoWidget->setHidden(true);
            d->baseInfoWidget->deleteLater();
        }

        DFMFileBasicInfoWidget *basicInfoWidget = new DFMFileBasicInfoWidget(this);
        d->baseInfoWidget = basicInfoWidget;
        basicInfoWidget->setShowFileName(true);
        basicInfoWidget->setShowPicturePixel(true);
        basicInfoWidget->setUrl(d->m_url);

        d->mainLayout->insertWidget(2, d->baseInfoWidget);

        LoadFileTags();
    }
}

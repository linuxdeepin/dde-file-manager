/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "detailview.h"
#include "private/detailview_p.h"
#include "detailextendview.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/utils/fileutils.h"

#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QFileSystemModel>
#include <QTreeView>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

DetailView::DetailView(QWidget *parent)
    : DFrame(parent),
      detailViewPrivate(new DetailViewPrivate(this))
{
    this->setFixedWidth(320);
    this->setAutoFillBackground(true);
}

DetailView::~DetailView()
{
    if (detailViewPrivate) {
        detailViewPrivate->deleteLater();
    }
}

/*!
 * \brief               在最右文件信息窗口中追加新增控件
 * \param widget        新增控件对象
 * \return              是否成功
 */
bool DetailView::addCustomControl(QWidget *widget)
{
    if (widget) {
        QPushButton *btn = new QPushButton(this);
        btn->setMaximumHeight(1);
        detailViewPrivate->addCustomControl(widget);
        return true;
    }
    return false;
}

/*!
 * \brief               在最右文件信息窗口中指定位置新增控件
 * \param widget        新增控件对象
 * \return              是否成功
 */
bool DetailView::insertCustomControl(int index, QWidget *widget)
{
    if (widget) {
        detailViewPrivate->insertCustomControl(index, widget);
        return true;
    }
    return false;
}

void DetailView::setUrl(const QUrl &url)
{
    detailViewPrivate->url = url;
    if (this->isVisible())
        detalHandle(const_cast<QUrl &>(url));
}

void DetailView::detalHandle(QUrl &url)
{
    detailViewPrivate->setFileUrl(const_cast<QUrl &>(url));
}

void DetailView::showEvent(QShowEvent *event)
{
    detalHandle(detailViewPrivate->url);
    DFrame::showEvent(event);
}

DetailViewPrivate::DetailViewPrivate(DetailView *view)
    : detailView(view)
{
    initUI();
}

void DetailViewPrivate::setFileUrl(QUrl &url)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;
    QSize targetSize = iconLabel->size().scaled(iconLabel->width(), iconLabel->height(), Qt::KeepAspectRatio);
    iconLabel->setPixmap(info->fileIcon().pixmap(targetSize));
    baseInfoView->setFileUrl(url);
}

DetailViewPrivate::~DetailViewPrivate()
{
}

void DetailViewPrivate::initUI()
{
    QFrame *frame = new QFrame(detailView);
    splitter = new QVBoxLayout();
    splitter->setContentsMargins(15, 0, 15, 0);
    splitter->setSpacing(8);
    iconLabel = new QLabel(detailView);
    iconLabel->setFixedSize(160, 160);
    iconLabel->setAlignment(Qt::AlignCenter);

    baseInfoView = new FileBaseInfoView(detailView);

    QPushButton *btn = new QPushButton(detailView);
    btn->setMaximumHeight(1);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(15, 0, 15, 0);
    vlayout->addWidget(iconLabel, 1, Qt::AlignHCenter);
    vlayout->addWidget(btn);
    vlayout->addWidget(baseInfoView);

    QVBoxLayout *vlayout1 = new QVBoxLayout(frame);
    vlayout1->setMargin(0);
    vlayout1->addLayout(vlayout);
    vlayout1->addLayout(splitter);
    vlayout1->addStretch(1);

    scrollArea = new QScrollArea(detailView);
    scrollArea->setAlignment(Qt::AlignTop);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(scrollArea);

    detailView->setLayout(mainLayout);

    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidget(frame);
    scrollArea->setWidgetResizable(true);
}

/*!
 * \brief               在最右文件信息窗口中追加新增控件
 * \param widget        新增控件对象
 */
void DetailViewPrivate::addCustomControl(QWidget *widget)
{
    splitter->addWidget(widget);
    static_cast<DetailExtendView *>(widget)->setFileUrl(url);
}

/*!
 * \brief               在最右文件信息窗口中指定位置新增控件
 * \param widget        新增控件对象
 */
void DetailViewPrivate::insertCustomControl(int index, QWidget *widget)
{
    splitter->insertWidget(index, widget);
    static_cast<DetailExtendView *>(widget)->setFileUrl(url);
}

FileBaseInfoView::FileBaseInfoView(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

FileBaseInfoView::~FileBaseInfoView()
{
}

void FileBaseInfoView::initUI()
{
    fileName = new KeyValueLabel(this);
    fileName->setLeftValue(tr("Name"));
    fileSize = new KeyValueLabel(this);
    fileSize->setLeftValue(tr("Size"));
    fileViewSize = new KeyValueLabel(this);
    fileViewSize->setLeftValue(tr("Dimension"));
    fileDuration = new KeyValueLabel(this);
    fileDuration->setLeftValue(tr("Duration"));
    fileType = new KeyValueLabel(this);
    fileType->setLeftValue(tr("Type"));
    fileInterviewTime = new KeyValueLabel(this);
    fileInterviewTime->setLeftValue(tr("Accessed"));
    fileChangeTime = new KeyValueLabel(this);
    fileChangeTime->setLeftValue(tr("Modified"));
    QGridLayout *glayout = new QGridLayout;
    glayout->setSpacing(10);
    glayout->addWidget(fileName, 0, 0);
    glayout->addWidget(fileSize, 1, 0);
    glayout->addWidget(fileViewSize, 2, 0);
    glayout->addWidget(fileDuration, 3, 0);
    glayout->addWidget(fileType, 4, 0);
    glayout->addWidget(fileInterviewTime, 5, 0);
    glayout->addWidget(fileChangeTime, 6, 0);
    glayout->setColumnStretch(0, 1);
    setLayout(glayout);
}

/*!
 * \brief           通过文件url获取文件fileinfo对象，在通过MimDatabase获取文件类型。最后根据文件类型对不同文件设置信息
 * \param[in] url       文件url
 */
void FileBaseInfoView::setFileUrl(QUrl &url)
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;

    fileName->setRightValue(info->fileName(), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileInterviewTime->setRightValue(info->lastRead().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideMiddle, Qt::AlignLeft, true);
    fileChangeTime->setRightValue(info->lastModified().toString("yyyy/MM/dd hh:mm:ss"), Qt::ElideMiddle, Qt::AlignLeft, true);

    QMimeType mimeType = MimeDatabase::mimeTypeForUrl(url);
    MimeDatabase::FileType type = MimeDatabase::mimeFileTypeNameToEnum(mimeType.name());
    switch (type) {
    case MimeDatabase::FileType::Directory:
        fileType->setRightValue(tr("Directory"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(false);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    case MimeDatabase::FileType::Documents: {
        fileType->setRightValue(tr("Documents"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
    } break;
    case MimeDatabase::FileType::Videos: {
        fileType->setRightValue(tr("Videos"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        QVariant dimension = info->extraProperties().value(QString("Dimension"));
        fileViewSize->setRightValue(dimension.toString(), Qt::ElideNone, Qt::AlignLeft, true);
        QVariant duration = info->extraProperties().value(QString("Duration"));
        fileDuration->setRightValue(duration.toString(), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(true);
        fileViewSize->setVisible(true);
        fileDuration->setVisible(true);
    } break;
    case MimeDatabase::FileType::Images: {
        fileType->setRightValue(tr("Images"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setRightValue(QString::number(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        QVariant dimension = info->extraProperties().value(QString("Dimension"));
        fileViewSize->setRightValue(dimension.toString(), Qt::ElideNone, Qt::AlignLeft, true);
        QVariant duration = info->extraProperties().value(QString("Duration"));
        fileDuration->setRightValue(duration.toString(), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(true);
        fileViewSize->setVisible(true);
        fileDuration->setVisible(false);
    } break;
    case MimeDatabase::FileType::Audios: {
        fileType->setRightValue(tr("Audios"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        QVariant duration = info->extraProperties().value(QString("Duration"));
        fileDuration->setRightValue(duration.toString(), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(true);
    } break;
    case MimeDatabase::FileType::Executable:
        fileType->setRightValue(tr("Executable"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    case MimeDatabase::FileType::Archives:
        fileType->setRightValue(tr("Archives"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    case MimeDatabase::FileType::Unknown:
        fileType->setRightValue(tr("Unknown"), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setRightValue(FileUtils::formatSize(info->size()), Qt::ElideNone, Qt::AlignLeft, true);
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    }
}

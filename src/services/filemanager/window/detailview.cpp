/*
 * Copyright (C) 2021 ~ 2022 Deepin Technology Co., Ltd.
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

DSB_FM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DetailView::DetailView(QWidget *parent)
    : DFrame(parent),
      detailViewPrivate(new DetailViewPrivate(this))
{
    this->setMinimumWidth(300);
    this->setMaximumWidth(325);
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
    //    iconLabel->setPixmap(info.fileIcon().pixmap(64, 64));
    baseInfoView->setFileUrl(url);
}

DetailViewPrivate::~DetailViewPrivate()
{
}

void DetailViewPrivate::initUI()
{
    splitter = new Splitter(Qt::Orientation::Vertical, detailView);
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

    QVBoxLayout *vlayout1 = new QVBoxLayout(detailView);
    vlayout1->setMargin(0);
    vlayout1->addLayout(vlayout);
    vlayout1->addWidget(splitter);
    vlayout1->addStretch(1);
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

KeyValueLabel::KeyValueLabel(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

KeyValueLabel::~KeyValueLabel()
{
}

void KeyValueLabel::initUI()
{
    leftValueLabel = new QLabel(this);
    rightValueLabel = new QLabel(this);
    QGridLayout *glayout = new QGridLayout;
    glayout->setMargin(0);
    glayout->addWidget(leftValueLabel, 0, 0, 1, 1);
    glayout->addWidget(rightValueLabel, 0, 1, 1, 1);
    setLayout(glayout);
}

void KeyValueLabel::setLeftValue(QString value)
{
    leftValueLabel->setText(value);
}

void KeyValueLabel::setRightValue(QString value)
{
    rightValueLabel->setText(value);
}

void KeyValueLabel::setLeftRightValue(QString leftValue, QString rightValue)
{
    setLeftValue(leftValue);
    setRightValue(rightValue);
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
    fileName->setLeftValue(tr("name"));
    fileSize = new KeyValueLabel(this);
    fileSize->setLeftValue(tr("size"));
    fileViewSize = new KeyValueLabel(this);
    fileViewSize->setLeftValue(tr("dimension"));
    fileDuration = new KeyValueLabel(this);
    fileDuration->setLeftValue(tr("duration"));
    fileType = new KeyValueLabel(this);
    fileType->setLeftValue(tr("file type"));
    fileInterviewTime = new KeyValueLabel(this);
    fileInterviewTime->setLeftValue(tr("interview time"));
    fileChangeTime = new KeyValueLabel(this);
    fileChangeTime->setLeftValue(tr("change time"));
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

    fileName->setRightValue(info->fileName());
    fileInterviewTime->setRightValue(info->lastRead().toString("yyyy/MM/dd hh:mm:ss"));
    fileChangeTime->setRightValue(info->lastModified().toString("yyyy/MM/dd hh:mm:ss"));

    QMimeType mimeType = MimeDatabase::mimeTypeForUrl(url);
    MimeDatabase::FileType type = MimeDatabase::mimeFileTypeNameToEnum(mimeType.name());
    switch (type) {
    case MimeDatabase::FileType::Directory:
        fileType->setRightValue(tr("Directory"));
        fileSize->setVisible(false);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    case MimeDatabase::FileType::Documents: {
        fileType->setRightValue(tr("Documents"));
        fileSize->setRightValue(FileUtils::formatSize(info->size()));
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
    } break;
    case MimeDatabase::FileType::Videos: {
        fileType->setRightValue(tr("Videos"));
        fileSize->setRightValue(FileUtils::formatSize(info->size()));
        QVariant dimension = info->extraProperties().value(QString("Dimension"));
        fileViewSize->setRightValue(dimension.toString());
        QVariant duration = info->extraProperties().value(QString("Duration"));
        fileDuration->setRightValue(duration.toString());
        fileSize->setVisible(true);
        fileViewSize->setVisible(true);
        fileDuration->setVisible(true);
    } break;
    case MimeDatabase::FileType::Images: {
        fileType->setRightValue(tr("Images"));
        fileSize->setRightValue(QString::number(info->size()));
        QVariant dimension = info->extraProperties().value(QString("Dimension"));
        fileViewSize->setRightValue(dimension.toString());
        QVariant duration = info->extraProperties().value(QString("Duration"));
        fileDuration->setRightValue(duration.toString());
        fileSize->setVisible(true);
        fileViewSize->setVisible(true);
        fileDuration->setVisible(false);
    } break;
    case MimeDatabase::FileType::Audios: {
        fileType->setRightValue(tr("Audios"));
        fileSize->setRightValue(FileUtils::formatSize(info->size()));
        QVariant duration = info->extraProperties().value(QString("Duration"));
        fileDuration->setRightValue(duration.toString());
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(true);
    } break;
    case MimeDatabase::FileType::Executable:
        fileType->setRightValue(tr("Executable"));
        fileSize->setRightValue(FileUtils::formatSize(info->size()));
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    case MimeDatabase::FileType::Archives:
        fileType->setRightValue(tr("Archives"));
        fileSize->setRightValue(FileUtils::formatSize(info->size()));
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    case MimeDatabase::FileType::Unknown:
        fileType->setRightValue(tr("Unknown"));
        fileSize->setRightValue(FileUtils::formatSize(info->size()));
        fileSize->setVisible(true);
        fileViewSize->setVisible(false);
        fileDuration->setVisible(false);
        break;
    }
}

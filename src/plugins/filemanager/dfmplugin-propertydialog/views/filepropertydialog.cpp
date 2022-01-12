/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "filepropertydialog.h"
#include "dfm-base/base/schemefactory.h"

#include <DFontSizeManager>
#include <denhancedwidget.h>

#include <QDateTime>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QStackedWidget>
#include <QPushButton>
#include <QTextOption>
#include <QTextLayout>
#include <QLayout>
#include <QVBoxLayout>
#include <QFrame>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPPROPERTYDIALOG_USE_NAMESPACE

static const int kArrowExpandHight = 30;
static const int kArrowExpandSpacing = 10;

FilePropertyDialog::FilePropertyDialog(QWidget *parent)
    : DDialog(parent),
      platformWindowHandle(new DPlatformWindowHandle(this, this))
{
    platformWindowHandle->setEnableSystemResize(true);
    setFixedWidth(350);
    initHeadUI();
    initInfoUI();
    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

FilePropertyDialog::~FilePropertyDialog()
{
}

void FilePropertyDialog::initHeadUI()
{
    fileIcon = new DLabel(this);
    fileIcon->setFixedHeight(128);

    editStackWidget = new EditStackedWidget(this);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 10, 10, 10);
    vlayout->addWidget(fileIcon, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout->addWidget(editStackWidget, 1, Qt::AlignHCenter | Qt::AlignTop);

    QFrame *frame = new QFrame(this);
    frame->setLayout(vlayout);
    addContent(frame);
}

void FilePropertyDialog::initInfoUI()
{
    scrollArea = new QScrollArea();
    scrollArea->setObjectName("PropertyDialog-QScrollArea");
    QPalette palette = scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    scrollArea->viewport()->setPalette(palette);
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    QFrame *mainWidget = new QFrame(this);
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 20);
    scrollWidgetLayout->setSpacing(kArrowExpandSpacing);
    scrollWidgetLayout->addStretch();
    mainWidget->setLayout(scrollWidgetLayout);

    scrollArea->setWidget(mainWidget);

    basicWidget = new BasicWidget(this);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(scrollArea);
    QVBoxLayout *widgetlayout = qobject_cast<QVBoxLayout *>(this->layout());
    widgetlayout->addLayout(vlayout1, 1);
}

int FilePropertyDialog::contentHeight()
{
    int expandsHeight = kArrowExpandSpacing;
    for (const QWidget *expand : extendedControl) {
        expandsHeight += expand->height();
    }

#define DIALOG_TITLEBAR_HEIGHT 50
    return (DIALOG_TITLEBAR_HEIGHT
            + fileIcon->height()
            + editStackWidget->height()
            + expandsHeight
            + contentsMargins().top()
            + contentsMargins().bottom()
            + 40);
}

void FilePropertyDialog::setSelectFileUrl(const QUrl &url)
{
    currentFileUrl = url;
    editStackWidget->selectFile(url);
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (info.isNull())
        return;
    fileIcon->setPixmap(info->fileIcon().pixmap(128, 128));
    addExtendedControl(basicWidget, true);
}

void FilePropertyDialog::processHeight(int height)
{
    QRect rect = geometry();
    rect.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rect);
}

void FilePropertyDialog::insertExtendedControl(int index, ExtendedControlView *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    widget->setSelectFileUrl(currentFileUrl);
    QMargins cm = vlayout->contentsMargins();
    QRect rc = contentsRect();
    widget->setFixedWidth(rc.width() - cm.left() - cm.right());
    extendedControl.append(widget);
}

void FilePropertyDialog::addExtendedControl(ExtendedControlView *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    insertExtendedControl(vlayout->count() - 1, widget);
}

void FilePropertyDialog::insertExtendedControl(int index, ExtendedControlDrawerView *widget, bool expansion)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    widget->setSelectFileUrl(currentFileUrl);
    widget->setFixedHeight(kArrowExpandHight);
    widget->setExpand(expansion);
    QMargins cm = vlayout->contentsMargins();
    QRect rc = contentsRect();
    widget->setFixedWidth(rc.width() - cm.left() - cm.right());
    extendedControl.append(widget);
    connect(widget, &ExtendedControlDrawerView::heightChanged, this, &FilePropertyDialog::processHeight);
}

void FilePropertyDialog::addExtendedControl(ExtendedControlDrawerView *widget, bool expansion)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    insertExtendedControl(vlayout->count() - 1, widget, expansion);
}

void FilePropertyDialog::closeDialog()
{
    emit closed(currentFileUrl);
}

void FilePropertyDialog::mousePressEvent(QMouseEvent *event)
{
    editStackWidget->mouseProcess(event);
    DDialog::mousePressEvent(event);
}

void FilePropertyDialog::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);
}

void FilePropertyDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    //展示时须设置弹窗尺寸，防止最小化后再次展示时窗口大小异常
    QRect rc = geometry();
    rc.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rc);
}

void FilePropertyDialog::closeEvent(QCloseEvent *event)
{
    closeDialog();
    DDialog::closeEvent(event);
}

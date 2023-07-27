// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultpropertydialog.h"

#include <dfm-base/base/schemefactory.h>

#include <denhancedwidget.h>

static const int kArrowExpandSpacing = 10;

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultPropertyDialog::VaultPropertyDialog(QWidget *parent)
    : DDialog(parent),
      platformWindowHandle(new DPlatformWindowHandle(this, this))
{
    platformWindowHandle->setEnableSystemResize(true);
    setFixedWidth(350);
    initInfoUI();
    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

VaultPropertyDialog::~VaultPropertyDialog()
{
}

void VaultPropertyDialog::initInfoUI()
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

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(scrollArea);
    QVBoxLayout *widgetlayout = qobject_cast<QVBoxLayout *>(this->layout());
    widgetlayout->addLayout(vlayout1, 1);
}

void VaultPropertyDialog::createHeadUI(const QUrl &url)
{
    fileIconLabel = new QLabel(this);
    fileIconLabel->setFixedHeight(128);
    FileInfoPointer info = InfoFactory::create<FileInfo>(url);
    if (!info.isNull())
        fileIconLabel->setPixmap(info->fileIcon().pixmap(128, 128));

    fileNameLabel = new QLabel(tr("My Vault"), this);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 10, 10, 10);
    vlayout->addWidget(fileIconLabel, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout->addWidget(fileNameLabel, 1, Qt::AlignHCenter | Qt::AlignTop);

    QFrame *frame = new QFrame(this);
    frame->setLayout(vlayout);

    addContent(frame);
}

void VaultPropertyDialog::createBasicWidget(const QUrl &url)
{
    basicWidget = new BasicWidget(this);
    basicWidget->selectFileUrl(url);
    addExtendedControl(basicWidget);
}

int VaultPropertyDialog::contentHeight()
{
    int expandsHeight = kArrowExpandSpacing;
    for (const QWidget *expand : extendedControl) {
        expandsHeight += expand->height();
    }

    QWidget *w = this->getContent(0);
    int h = w == nullptr ? 0 : w->height();

#define DIALOG_TITLEBAR_HEIGHT 50
    return (DIALOG_TITLEBAR_HEIGHT
            + h
            + expandsHeight
            + contentsMargins().top()
            + contentsMargins().bottom()
            + 40);
}

void VaultPropertyDialog::insertExtendedControl(int index, QWidget *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    QMargins cm = vlayout->contentsMargins();
    QRect rc = contentsRect();
    widget->setFixedWidth(rc.width() - cm.left() - cm.right());
    extendedControl.append(widget);
    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(widget, widget);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, this, &VaultPropertyDialog::processHeight);
}

void VaultPropertyDialog::addExtendedControl(QWidget *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    insertExtendedControl(vlayout->count() - 1, widget);
}

void VaultPropertyDialog::processHeight(int height)
{
    QRect rect = geometry();
    rect.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rect);
}

void VaultPropertyDialog::selectFileUrl(const QUrl &url)
{
    createHeadUI(url);
    createBasicWidget(url);
}

void VaultPropertyDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    //展示时须设置弹窗尺寸，防止最小化后再次展示时窗口大小异常
    QRect rc = geometry();
    rc.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rc);
}

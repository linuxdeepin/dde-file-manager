// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepropertydialog.h"
#include "basicwidget.h"
#include "permissionmanagerwidget.h"
#include "utils/propertydialogmanager.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileinfohelper.h>

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
using namespace dfmplugin_propertydialog;

static const int kArrowExpandSpacing = 10;

FilePropertyDialog::FilePropertyDialog(QWidget *parent)
    : DDialog(parent),
      platformWindowHandle(new DPlatformWindowHandle(this, this))
{
    platformWindowHandle->setEnableSystemResize(true);
    setFixedWidth(350);
    initInfoUI();
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FilePropertyDialog::onFileInfoUpdated, Qt::QueuedConnection);
}

FilePropertyDialog::~FilePropertyDialog()
{
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
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 30);
    scrollWidgetLayout->setSpacing(kArrowExpandSpacing);
    scrollWidgetLayout->addStretch();
    mainWidget->setLayout(scrollWidgetLayout);

    scrollArea->setWidget(mainWidget);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(scrollArea);
    QVBoxLayout *widgetlayout = qobject_cast<QVBoxLayout *>(this->layout());
    widgetlayout->addLayout(vlayout1, 1);
}

void FilePropertyDialog::createHeadUI(const QUrl &url)
{
    fileIcon = new QLabel(this);
    fileIcon->setFixedHeight(128);
    currentInfo = InfoFactory::create<FileInfo>(url);
    if (!currentInfo.isNull())
        fileIcon->setPixmap(currentInfo->fileIcon().pixmap(128, 128));

    editStackWidget = new EditStackedWidget(this);
    editStackWidget->selectFile(url);
    connect(editStackWidget, &EditStackedWidget::selectUrlRenamed, this, &FilePropertyDialog::onSelectUrlRenamed);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(10, 10, 10, 10);
    vlayout->addWidget(fileIcon, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout->addWidget(editStackWidget, 1, Qt::AlignHCenter | Qt::AlignTop);

    QFrame *frame = new QFrame(this);
    frame->setLayout(vlayout);

    addContent(frame);
}

void FilePropertyDialog::createBasicWidget(const QUrl &url)
{
    basicWidget = new BasicWidget(this);
    basicWidget->selectFileUrl(url);
    addExtendedControl(basicWidget);
}

void FilePropertyDialog::createPermissionManagerWidget(const QUrl &url)
{
    permissionManagerWidget = new PermissionManagerWidget(this);
    permissionManagerWidget->selectFileUrl(url);

    insertExtendedControl(INT_MAX, permissionManagerWidget);
}

void FilePropertyDialog::filterControlView()
{
    PropertyFilterType controlFilter = PropertyDialogManager::instance().basicFiledFiltes(currentFileUrl);
    if ((controlFilter & PropertyFilterType::kIconTitle) < 1) {
        createHeadUI(currentFileUrl);
    }

    if ((controlFilter & PropertyFilterType::kBasisInfo) < 1) {
        createBasicWidget(currentFileUrl);
    }

    if ((controlFilter & PropertyFilterType::kPermission) < 1) {
        createPermissionManagerWidget(currentFileUrl);
    }
}

int FilePropertyDialog::contentHeight()
{
    int expandsHeight = std::accumulate(extendedControl.begin(), extendedControl.end(),
                                        kArrowExpandSpacing, [](int sum, QWidget *w) {
                                            return sum += w->height();
                                        });

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

void FilePropertyDialog::selectFileUrl(const QUrl &url)
{
    currentFileUrl = url;
}

qint64 FilePropertyDialog::getFileSize()
{
    if (basicWidget)
        return basicWidget->getFileSize();
    return 0;
}

int FilePropertyDialog::getFileCount()
{
    if (basicWidget)
        return basicWidget->getFileCount();
    return 1;
}

void FilePropertyDialog::setBasicInfoExpand(bool expand)
{
    if (basicWidget)
        basicWidget->setExpand(expand);
}

void FilePropertyDialog::processHeight(int height)
{
    QRect rect = geometry();
    rect.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rect);
}

void FilePropertyDialog::insertExtendedControl(int index, QWidget *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    QMargins cm = vlayout->contentsMargins();
    QRect rc = contentsRect();
    widget->setFixedWidth(rc.width() - cm.left() - cm.right());
    extendedControl.append(widget);
    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(widget, widget);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, this, &FilePropertyDialog::processHeight);
}

void FilePropertyDialog::addExtendedControl(QWidget *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    insertExtendedControl(vlayout->count() - 1, widget);
}

void FilePropertyDialog::closeDialog()
{
    emit closed(currentFileUrl);
}

void FilePropertyDialog::onSelectUrlRenamed(const QUrl &url)
{
    if (permissionManagerWidget)
        permissionManagerWidget->updateFileUrl(url);

    if (basicWidget)
        basicWidget->updateFileUrl(url);
}

void FilePropertyDialog::onFileInfoUpdated(const QUrl &url, const bool isLinkOrg)
{
    if (url != currentFileUrl || currentInfo.isNull())
        return;

    if (isLinkOrg)
        currentInfo->customData(Global::ItemRoles::kItemFileRefreshIcon);

    if (!fileIcon)
        return;

    fileIcon->setPixmap(currentInfo->fileIcon().pixmap(128, 128));
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

void FilePropertyDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
    DDialog::keyPressEvent(event);
}

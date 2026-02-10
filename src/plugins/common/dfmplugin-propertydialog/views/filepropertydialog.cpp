// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepropertydialog.h"
#include "basicwidget.h"
#include "permissionmanagerwidget.h"
#include "utils/propertydialogmanager.h"
#include "utils/propertydialogutil.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/thumbnail/thumbnailhelper.h>

#include <DFontSizeManager>
#include <denhancedwidget.h>
#include <dtkgui_config.h>

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
#include <QScreen>
#include <QTimer>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;
DFMGLOBAL_USE_NAMESPACE

static constexpr int kArrowExpandSpacing { 10 };
static constexpr int kArrowExpandHeader { 30 };
static constexpr int kDialogHeader { 80 };
static constexpr int kDialogWidth { 380 };
static constexpr int kExtendedWidgetWidth { 360 };

FilePropertyDialog::FilePropertyDialog(QWidget *parent)
    : DDialog(parent),
      platformWindowHandle(new DPlatformWindowHandle(this, this))
{
    platformWindowHandle->setEnableSystemResize(true);
    setFixedWidth(kDialogWidth);
    initInfoUI();
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FilePropertyDialog::onFileInfoUpdated, Qt::QueuedConnection);
#ifdef DTKGUI_VERSION_STR
    platformWindowHandle->setWindowEffect(DPlatformWindowHandle::EffectScene::EffectNoStart);
#endif
    qApp->installEventFilter(this);
}

FilePropertyDialog::~FilePropertyDialog()
{
}

void FilePropertyDialog::initInfoUI()
{
    scrollArea = new QScrollArea();
    scrollArea->setObjectName("PropertyDialog-QScrollArea");
    QPalette palette = scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Window, Qt::NoBrush);
    scrollArea->viewport()->setPalette(palette);
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFrame *mainWidget = new QFrame(this);
    mainWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mainWidget->setMaximumWidth(kDialogWidth);

    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 10);
    scrollWidgetLayout->setSpacing(kArrowExpandSpacing);
    mainWidget->setLayout(scrollWidgetLayout);

    scrollArea->setWidget(mainWidget);

    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setMaximumWidth(kDialogWidth);

    QVBoxLayout *vlayout1 = new QVBoxLayout;
    vlayout1->addWidget(scrollArea);
    vlayout1->setContentsMargins(0, 0, 0, 0);
    vlayout1->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *widgetlayout = qobject_cast<QVBoxLayout *>(this->layout());
    widgetlayout->addLayout(vlayout1, 1);
}

void FilePropertyDialog::createHeadUI(const QUrl &url)
{
    fileIcon = new QLabel(this);
    fileIcon->setFixedHeight(128);
    currentInfo = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoSync);
    setFileIcon(fileIcon, currentInfo);

    editStackWidget = new EditStackedWidget(this);
    editStackWidget->selectFile(url);
    connect(editStackWidget, &EditStackedWidget::selectUrlRenamed, this, &FilePropertyDialog::onSelectUrlRenamed);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setContentsMargins(0, 0, 0, 0);
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

    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    if (vlayout) {
        insertExtendedControl(vlayout->count(), permissionManagerWidget);
    }
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
            + kArrowExpandSpacing * extendedControl.size());
}

void FilePropertyDialog::setFileIcon(QLabel *fileIcon, FileInfoPointer fileInfo)
{
    if (!fileInfo.isNull()) {
        ThumbnailHelper helper;
        QUrl localUrl = fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl);
        if (fileInfo->canAttributes(CanableInfoType::kCanRedirectionFileUrl))
            localUrl = fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
        if (helper.checkThumbEnable(localUrl)) {
            QIcon icon = fileInfo->extendAttributes(ExtInfoType::kFileThumbnail).value<QIcon>();
            if (icon.isNull()) {
                const auto &img = helper.thumbnailImage(localUrl, Global::kLarge);
                icon = QPixmap::fromImage(img);
            }
            if (!icon.isNull()) {
                fileIcon->setPixmap(icon.pixmap(128, 128).scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                return;
            }
        }
        fileIcon->setPixmap(fileInfo->fileIcon().pixmap(128, 128));
    }
}

bool FilePropertyDialog::eventFilter(QObject *object, QEvent *event)
{
    // 检查是否是ComboBox上的滚轮事件
    if (object->inherits("QComboBox") && event->type() == QEvent::Wheel) {
        // 确保ComboBox是我们对话框的后代
        QWidget *widget = qobject_cast<QWidget *>(object);
        if (widget && isAncestorOf(widget)) {
            // 将事件重定向到滚动区域
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            QCoreApplication::sendEvent(this, wheelEvent);
            return true;
        }
    }

    return DDialog::eventFilter(object, event);
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

int FilePropertyDialog::initalHeightOfView()
{
    int expandsHeight = kDialogHeader + fileIcon->height() + editStackWidget->height();
    for (int i = 0; i < extendedControl.size(); ++i) {
        DArrowLineDrawer *lineWidget = qobject_cast<DArrowLineDrawer *>(extendedControl.at(i));
        if (lineWidget) {
            BasicWidget *baseWidget = qobject_cast<BasicWidget *>(lineWidget);
            if (baseWidget && baseWidget->expand())
                expandsHeight += kArrowExpandHeader + baseWidget->expansionPreditHeight() + kArrowExpandSpacing;
            else
                expandsHeight += kArrowExpandHeader + kArrowExpandSpacing;
        } else {
            QWidget *widget = extendedControl.at(i);
            if (widget)
                expandsHeight += widget->sizeHint().height() + kArrowExpandSpacing;
        }
    }
    return expandsHeight;
}

void FilePropertyDialog::processHeight(int height)
{
    Q_UNUSED(height)
    if (!isShown)
        return;

    QRect rect = geometry();
    int screenHeight = WindowUtils::cursorScreen()->availableSize().height();
    screenHeight -= 100;

    int contentH = contentHeight() + kArrowExpandSpacing;
    int maxHeight = qMin(contentH, screenHeight);
    rect.setHeight(maxHeight);
    setGeometry(rect);

    if (scrollArea) {
        QWidget *content = scrollArea->widget();
        if (content) {
            content->setMinimumHeight(0);
            content->adjustSize();
            content->updateGeometry();
        }
        scrollArea->updateGeometry();
    }
}

void FilePropertyDialog::insertExtendedControl(int index, QWidget *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    index = index > vlayout->count() ? vlayout->count() : index;
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    widget->setFixedWidth(kExtendedWidgetWidth);
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
    close();
    PropertyDialogUtil::instance()->showPropertyDialog({ url }, QVariantHash());
}

void FilePropertyDialog::onFileInfoUpdated(const QUrl &url, const QString &infoPtr, const bool isLinkOrg)
{
    if (url != currentFileUrl || currentInfo.isNull() || QString::number(quintptr(currentInfo.data()), 16) != infoPtr)
        return;

    if (isLinkOrg)
        currentInfo->customData(Global::ItemRoles::kItemFileRefreshIcon);

    if (!fileIcon)
        return;

    setFileIcon(fileIcon, currentInfo);
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

    isShown = true;
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    if (vlayout) {
        if (vlayout->count() > 0) {
            QLayoutItem *item = vlayout->itemAt(vlayout->count() - 1);
            if (item && item->spacerItem()) {
                vlayout->removeItem(item);
                delete item;
            }
        }
        vlayout->addStretch();
    }

    QTimer::singleShot(0, this, [this]() {
        processHeight(contentHeight());
    });
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

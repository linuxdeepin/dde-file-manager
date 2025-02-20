// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreviewdialog.h"
#include "pluginInterface/filepreviewfactory.h"
#include "unknowfilepreview.h"
#include "utils/previewfileoperation.h"

#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-framework/dpf.h>

#include <DGuiApplicationHelper>
#include <DAnchors>

#include <QAction>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QTimer>
#include <QWindow>
#include <QMimeType>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_filepreview;

FilePreviewDialog::FilePreviewDialog(const QList<QUrl> &previewUrllist, QWidget *parent)
    : DAbstractDialog(parent),
      fileList(previewUrllist),
      dialogManager(DialogManagerInstance)
{
    initUI();

    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FilePreviewDialog::handleFileInfoRefreshFinished, Qt::QueuedConnection);

    if (previewUrllist.count() < 2) {
        statusBar->preButton()->hide();
        statusBar->nextButton()->hide();
    }
    firstEnterSwitchToPage = true;
    switchToPage(0);
    if (windowHandle())
        windowHandle()->installEventFilter(this);
}

FilePreviewDialog::~FilePreviewDialog()
{
    if (preview) {
        preview->deleteLater();
        preview = nullptr;
        QGuiApplication::changeOverrideCursor(QCursor(Qt::ArrowCursor));
    }
}

void FilePreviewDialog::updatePreviewList(const QList<QUrl> &previewUrllist)
{
    // 视频预览的前一秒禁止再次播放
    if (playingVideo) {
        return;
    }

    fileList = previewUrllist;
    currentPageIndex = -1;

    if (previewUrllist.count() < 2) {
        statusBar->preButton()->hide();
        statusBar->nextButton()->hide();
    } else {
        statusBar->preButton()->show();
        statusBar->nextButton()->show();
    }
    firstEnterSwitchToPage = true;
    switchToPage(0);
}

void FilePreviewDialog::setEntryUrlList(const QList<QUrl> &urlList)
{
    if (urlList.isEmpty())
        return;
    QUrl currentUrl = fileList.at(currentPageIndex);
    if (urlList.contains(currentUrl)) {
        previewDir = true;
        fileList = urlList;
        currentPageIndex = fileList.indexOf(currentUrl);
    }
}

void FilePreviewDialog::done(int r)
{
    DAbstractDialog::done(r);

    if (preview) {
        preview->stop();
        preview->deleteLater();
        preview = nullptr;
    }
}

void FilePreviewDialog::setCurrentWinID(quint64 winID)
{
    currentWinID = winID;
}

void FilePreviewDialog::playCurrentPreviewFile()
{
    if (preview) {
        if (preview->metaObject()->className() == QStringLiteral("VideoPreview")) {
            playingVideo = true;
            // 1s 后才能重新预览视频，原因是快速切换预览视频会因为视频插件内部的崩溃引起文管崩溃
            QTimer::singleShot(1000, [this]() {
                playingVideo = false;
            });
        }
        preview->play();
    }
}

void FilePreviewDialog::openFile()
{
    bool succ = PreviewFileOperation::openFileHandle(currentWinID, fileList.at(currentPageIndex));
    if (succ)
        close();
}

void FilePreviewDialog::handleFileInfoRefreshFinished(const QUrl url, const QString &infoPtr, const bool isLinkOrg)
{
    if (UniversalUtils::urlEquals(url, fileList.at(currentPageIndex))) {
        switchToPage(currentPageIndex);
    }
}

void FilePreviewDialog::childEvent(QChildEvent *event)
{
    if (closeBtn && event->added()) {
        closeBtn->raise();
    }

    return DAbstractDialog::childEvent(event);
}

void FilePreviewDialog::showEvent(QShowEvent *event)
{
    return DAbstractDialog::showEvent(event);
}

void FilePreviewDialog::closeEvent(QCloseEvent *event)
{
    emit signalCloseEvent();
    if (preview) {
        preview->stop();
        preview->deleteLater();
        preview = nullptr;
    }

    return DAbstractDialog::closeEvent(event);
}

void FilePreviewDialog::resizeEvent(QResizeEvent *event)
{
    DAbstractDialog::resizeEvent(event);
    QTimer::singleShot(50, this, [=]() {   //! 50ms这个时间视机器性能而定
        repaint();   // 通过重绘来解决调整大小前的窗口残留的问题
    });
}

bool FilePreviewDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        const QKeyEvent *e = static_cast<QKeyEvent *>(event);
        switch (e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Up:
            if (!e->isAutoRepeat())
                previousPage();
            break;
        case Qt::Key_Right:
        case Qt::Key_Down:
            if (!e->isAutoRepeat())
                nextPage();
            break;
        case Qt::Key_Escape:
        case Qt::Key_Space: {
            if (!e->isAutoRepeat()) {
                // 视频预览的前一秒禁止再次播放
                if (playingVideo) {
                    break;
                }
                if (preview) {
                    preview->stop();
                }
                close();
            }
            return true;
        }
        default:
            break;
        }
    }

    return DAbstractDialog::eventFilter(obj, event);
}

void FilePreviewDialog::initUI()
{
    closeBtn = new DFloatingButton(DStyle::SP_CloseButton, this);
    closeBtn->setStyleSheet("background-color: transparent;");
    closeBtn->setFixedSize(46, 46);
    closeBtn->setIconSize(QSize(40, 40));
    closeBtn->setFlat(true);
    DAnchorsBase::setAnchor(closeBtn, Qt::AnchorRight, this, Qt::AnchorRight);
    connect(closeBtn, &QPushButton::clicked, this, &FilePreviewDialog::close);

    separator = new DHorizontalLine(this);
    separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    statusBar = new FilePreviewDialogStatusBar(this);
    statusBar->setObjectName("StatusBar");
    statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // statusBar->openButton()->setFocus();

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QHBoxLayout *separatorLayout = new QHBoxLayout(this);

    separatorLayout->addSpacing(10);
    separatorLayout->addWidget(separator);
    separatorLayout->addSpacing(10);

    layout->addLayout(separatorLayout, 1);
    layout->addWidget(statusBar, 0, Qt::AlignBottom);

    QAction *shortcutAction = new QAction(this);

    shortcutAction->setShortcut(QKeySequence::Copy);
    addAction(shortcutAction);

    connect(statusBar->preButton(), &QPushButton::clicked, this, &FilePreviewDialog::previousPage);
    connect(statusBar->nextButton(), &QPushButton::clicked, this, &FilePreviewDialog::nextPage);
    connect(statusBar->openButton(), &QPushButton::clicked, this, &FilePreviewDialog::openFile);
}

void FilePreviewDialog::switchToPage(int index)
{
    currentPageIndex = index;
    statusBar->preButton()->setEnabled(index > 0);
    statusBar->nextButton()->setEnabled(index < fileList.count() - 1);

    FileInfoPointer info = InfoFactory::create<FileInfo>(fileList.at(index));
    if (info.isNull())
        return;

    if (!info) {
        fileList.removeAt(index);

        if (fileList.isEmpty())
            return;

        return switchToPage(index);
    }

    info->updateAttributes();

    AbstractBasePreview *view = nullptr;
    const QMimeType &mimeType = DMimeDatabase().mimeTypeForUrl(fileList.at(index));

    QStringList keyList(mimeType.name());

    keyList.append(mimeType.aliases());
    keyList.append(mimeType.allAncestors());

    // TODO: change view select item
    // if (previewDir) {
    //     QList<QUrl> selectUrl { fileList.at(index) };
    //     auto eventID { DPF_NAMESPACE::Event::instance()->eventType("dfmplugin_workspace", "slot_View_SelectFiles") };
    //     if (eventID != DPF_NAMESPACE::EventTypeScope::kInValid)
    //         dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SelectFiles", currentWinID, selectUrl);
    //     else
    //         dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasView_Select", selectUrl);
    // }

    for (const QString &key : keyList) {
        const QString &gKey = generalKey(key);

        if (preview && (FilePreviewFactory::isSuitedWithKey(preview, key) || FilePreviewFactory::isSuitedWithKey(preview, gKey)) && !FileUtils::isDesktopFile(fileList.at(index))) {
            if (preview->setFileUrl(fileList.at(index))) {
                preview->contentWidget()->updateGeometry();
                updateTitle();
                // statusBar->openButton()->setFocus();
                preview->contentWidget()->adjustSize();
                int newPerviewWidth = preview->contentWidget()->size().width();
                int newPerviewHeight = preview->contentWidget()->size().height();
                setFixedSize(newPerviewWidth, newPerviewHeight + statusBar->height());
                playCurrentPreviewFile();
                moveToCenter();
                return;
            }
        }
        view = FilePreviewFactory::create(key);
        if (!view && gKey != key) {
            if (!FileUtils::isDesktopFile(fileList.at(index))) {
                view = FilePreviewFactory::create(gKey);
            }
        }

        if (view) {
            view->initialize(this, statusBar);

            if (info->canAttributes(CanableInfoType::kCanRedirectionFileUrl) && view->setFileUrl(info->urlOf(UrlInfoType::kRedirectedFileUrl)))
                break;
            else if (view->setFileUrl(fileList.at(index)))
                break;
            else
                view->deleteLater();
        }
    }

    if (!view) {
        if (preview && qobject_cast<UnknowFilePreview *>(preview)) {
            preview->setFileUrl(fileList.at(index));
            // statusBar->openButton()->setFocus();
            return;
        } else {
            view = new UnknowFilePreview(this);
            view->initialize(this, statusBar);
            view->setFileUrl(fileList.at(index));
        }
    }

    if (preview)
        disconnect(preview, &AbstractBasePreview::titleChanged, this, &FilePreviewDialog::updateTitle);

    connect(view, &AbstractBasePreview::titleChanged, this, &FilePreviewDialog::updateTitle);

    if (preview) {
        preview->contentWidget()->setVisible(false);
        static_cast<QHBoxLayout *>(statusBar->layout())->removeWidget(preview->statusBarWidget());
        static_cast<QVBoxLayout *>(layout())->removeWidget(preview->contentWidget());
        preview->handleBeforDestroy();
        preview->deleteLater();
    }

    static_cast<QVBoxLayout *>(layout())->insertWidget(0, view->contentWidget());

    if (QWidget *w = view->statusBarWidget())
        static_cast<QHBoxLayout *>(statusBar->layout())->insertWidget(3, w, 0, view->statusBarWidgetAlignment());

    separator->setVisible(view->showStatusBarSeparator());
    preview = view;

    playCurrentPreviewFile();
    // statusBar->openButton()->setFocus();
    this->adjustSize();
    preview->contentWidget()->adjustSize();
    int newPerviewWidth = preview->contentWidget()->size().width();
    int newPerviewHeight = preview->contentWidget()->size().height();
    setFixedSize(newPerviewWidth, newPerviewHeight + statusBar->height());
    updateTitle();
    moveToCenter();
}

void FilePreviewDialog::previousPage()
{
    if (currentPageIndex < 1)
        return;
    if (playingVideo)
        return;
    firstEnterSwitchToPage = false;
    switchToPage(currentPageIndex - 1);
}

void FilePreviewDialog::nextPage()
{
    if (currentPageIndex > fileList.count() - 2)
        return;
    if (playingVideo)
        return;
    firstEnterSwitchToPage = false;
    switchToPage(currentPageIndex + 1);
}

void FilePreviewDialog::updateTitle()
{
    // 在频繁启动关闭的场景下，preview可能会意外释放，引起空指针造成的崩溃
    if (!preview)
        return;

    QFont font = statusBar->title()->font();
    QFontMetrics fm(font);
    QString elidedText;

    if (!statusBar->preButton()->isVisible()) {
        /*smb 中一直按着空格预览，preview 已经析构了，但是定时器的timeout事件已经执行，这里使用智能指针进行判断*/
        if (!preview) {
            return;
        }
        elidedText = fm.elidedText(preview->title(), Qt::ElideMiddle, width() / 2 - statusBar->contentsMargins().left() - statusBar->layout()->spacing() - 30);
    } else {
        elidedText = fm.elidedText(preview->title(), Qt::ElideMiddle, width() / 2 - statusBar->preButton()->width() - statusBar->nextButton()->width() - statusBar->contentsMargins().left() - statusBar->layout()->spacing() * 3 - 30);
    }
    statusBar->title()->setText(elidedText);
    statusBar->title()->setHidden(statusBar->title()->text().isEmpty());
}

QString FilePreviewDialog::generalKey(const QString &key)
{
    const QStringList &tmp = key.split('/');

    if (tmp.size() > 1)
        return tmp.first() + "/*";

    return key;
}

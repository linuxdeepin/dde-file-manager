// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QScreen>
#include <QTimer>
#include <QWindow>
#include <QMimeType>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_filepreview;

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)

FilePreviewDialog::FilePreviewDialog(const QList<QUrl> &previewUrllist, QWidget *parent)
    : DAbstractDialog(parent),
      fileList(previewUrllist),
      dialogManager(DialogManagerInstance)
{
    qCInfo(logLibFilePreview) << "FilePreviewDialog: initializing with" << previewUrllist.size() << "files";
    initUI();

    connect(&FileInfoHelper::instance(), &FileInfoHelper::fileRefreshFinished, this,
            &FilePreviewDialog::handleFileInfoRefreshFinished, Qt::QueuedConnection);

    if (previewUrllist.count() < 2) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: single file mode - hiding navigation buttons";
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
    qCDebug(logLibFilePreview) << "FilePreviewDialog: destructor called";
    if (preview) {
        preview->deleteLater();
        preview = nullptr;
        QGuiApplication::changeOverrideCursor(QCursor(Qt::ArrowCursor));
    }
}

void FilePreviewDialog::updatePreviewList(const QList<QUrl> &previewUrllist)
{
    qCInfo(logLibFilePreview) << "FilePreviewDialog: updating preview list with" << previewUrllist.size() << "files";

    // 视频预览的前一秒禁止再次播放
    if (playingVideo) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: video is playing, ignoring preview list update";
        return;
    }

    fileList = previewUrllist;
    currentPageIndex = -1;

    if (previewUrllist.count() < 2) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: single file mode - hiding navigation buttons";
        statusBar->preButton()->hide();
        statusBar->nextButton()->hide();
    } else {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: multiple files mode - showing navigation buttons";
        statusBar->preButton()->show();
        statusBar->nextButton()->show();
    }
    firstEnterSwitchToPage = true;
    switchToPage(0);
}

void FilePreviewDialog::setEntryUrlList(const QList<QUrl> &urlList)
{
    if (urlList.isEmpty()) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: empty URL list provided for entry list";
        return;
    }

    QUrl currentUrl = fileList.at(currentPageIndex);
    if (urlList.contains(currentUrl)) {
        qCInfo(logLibFilePreview) << "FilePreviewDialog: setting directory preview mode with" << urlList.size() << "files";
        previewDir = true;
        fileList = urlList;
        currentPageIndex = fileList.indexOf(currentUrl);
        qCDebug(logLibFilePreview) << "FilePreviewDialog: current file index in directory:" << currentPageIndex;
    } else {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: current file not found in provided URL list";
    }
}

void FilePreviewDialog::done(int r)
{
    qCDebug(logLibFilePreview) << "FilePreviewDialog: dialog done with result:" << r;
    DAbstractDialog::done(r);

    if (preview) {
        preview->stop();
        preview->deleteLater();
        preview = nullptr;
    }
}

void FilePreviewDialog::setCurrentWinID(quint64 winID)
{
    qCDebug(logLibFilePreview) << "FilePreviewDialog: setting current window ID:" << winID;
    currentWinID = winID;
}

void FilePreviewDialog::playCurrentPreviewFile()
{
    if (preview) {
        if (preview->metaObject()->className() == QStringLiteral("VideoPreview")) {
            qCDebug(logLibFilePreview) << "FilePreviewDialog: starting video preview playback";
            playingVideo = true;
            // 1s 后才能重新预览视频，原因是快速切换预览视频会因为视频插件内部的崩溃引起文管崩溃
            QTimer::singleShot(1000, [this]() {
                playingVideo = false;
                qCDebug(logLibFilePreview) << "FilePreviewDialog: video preview cooldown period ended";
            });
        }
        preview->play();
    }
}

void FilePreviewDialog::openFile()
{
    qCInfo(logLibFilePreview) << "FilePreviewDialog: opening current file:" << fileList.at(currentPageIndex).toString();
    bool succ = PreviewFileOperation::openFileHandle(currentWinID, fileList.at(currentPageIndex));
    if (succ) {
        qCInfo(logLibFilePreview) << "FilePreviewDialog: file opened successfully, closing preview dialog";
        close();
    } else {
        qCWarning(logLibFilePreview) << "FilePreviewDialog: failed to open file:" << fileList.at(currentPageIndex).toString();
    }
}

void FilePreviewDialog::handleFileInfoRefreshFinished(const QUrl url, const QString &infoPtr, const bool isLinkOrg)
{
    Q_UNUSED(infoPtr)
    Q_UNUSED(isLinkOrg)

    if (UniversalUtils::urlEquals(url, fileList.at(currentPageIndex))) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: file info refreshed for current file, updating preview:" << url.toString();
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
    qCInfo(logLibFilePreview) << "FilePreviewDialog: close event triggered";
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
    qCDebug(logLibFilePreview) << "FilePreviewDialog: resize event - old size:" << event->oldSize() << "new size:" << event->size();
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
            if (!e->isAutoRepeat()) {
                qCDebug(logLibFilePreview) << "FilePreviewDialog: previous page key pressed";
                previousPage();
            }
            return true;
        case Qt::Key_Right:
        case Qt::Key_Down:
            if (!e->isAutoRepeat()) {
                qCDebug(logLibFilePreview) << "FilePreviewDialog: next page key pressed";
                nextPage();
            }
            return true;
        case Qt::Key_Escape:
        case Qt::Key_Space: {
            if (!e->isAutoRepeat()) {
                qCInfo(logLibFilePreview) << "FilePreviewDialog: close key pressed (Escape/Space)";
                // 视频预览的前一秒禁止再次播放
                if (playingVideo) {
                    qCDebug(logLibFilePreview) << "FilePreviewDialog: video is playing, ignoring close request";
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
    qCDebug(logLibFilePreview) << "FilePreviewDialog: initializing UI components";

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

    connect(statusBar->preButton(), &QPushButton::clicked, this, &FilePreviewDialog::previousPage);
    connect(statusBar->nextButton(), &QPushButton::clicked, this, &FilePreviewDialog::nextPage);
    connect(statusBar->openButton(), &QPushButton::clicked, this, &FilePreviewDialog::openFile);

    qCDebug(logLibFilePreview) << "FilePreviewDialog: UI initialization completed";
}

void FilePreviewDialog::switchToPage(int index)
{
    qCInfo(logLibFilePreview) << "FilePreviewDialog: switching to page" << index << "of" << fileList.count() << "files";

    if (index < 0 || index >= fileList.count()) {
        qCWarning(logLibFilePreview) << "FilePreviewDialog: invalid page index:" << index << "valid range: 0 -" << (fileList.count() - 1);
        return;
    }

    // 在切换视图前记录当前窗口中心位置（如果窗口可见）
    saveCenterPos();

    currentPageIndex = index;
    statusBar->preButton()->setEnabled(index > 0);
    statusBar->nextButton()->setEnabled(index < fileList.count() - 1);

    FileInfoPointer info = InfoFactory::create<FileInfo>(fileList.at(index));
    if (info.isNull()) {
        qCWarning(logLibFilePreview) << "FilePreviewDialog: failed to create file info for:" << fileList.at(index).toString();
        return;
    }

    if (!info) {
        qCWarning(logLibFilePreview) << "FilePreviewDialog: file info is null, removing from list:" << fileList.at(index).toString();
        fileList.removeAt(index);

        if (fileList.isEmpty()) {
            qCWarning(logLibFilePreview) << "FilePreviewDialog: no more files to preview";
            return;
        }

        return switchToPage(index);
    }

    info->updateAttributes();

    AbstractBasePreview *view = nullptr;
    const QMimeType &mimeType = DMimeDatabase().mimeTypeForUrl(fileList.at(index));

    QStringList keyList(mimeType.name());
    keyList.append(mimeType.aliases());
    keyList.append(mimeType.allAncestors());

    qCDebug(logLibFilePreview) << "FilePreviewDialog: searching preview for MIME type:" << mimeType.name() << "with keys:" << keyList;

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

        // Check if we can reuse existing preview, prioritize exact key match over general key
        bool canReusePreview = false;
        if (preview && !FileUtils::isDesktopFile(fileList.at(index))) {
            // First check exact key match
            if (FilePreviewFactory::isSuitedWithKey(preview, key)) {
                canReusePreview = true;
            } else if (gKey != key) {
                // Only use general key if no exact key plugin exists
                // This prevents text/* from intercepting text/markdown when markdown-preview exists
                if (!FilePreviewFactory::hasPluginForKey(key)) {
                    canReusePreview = FilePreviewFactory::isSuitedWithKey(preview, gKey);
                }
            }
        }

        if (canReusePreview) {
            qCDebug(logLibFilePreview) << "FilePreviewDialog: reusing existing preview for key:" << key;
            if (preview->setFileUrl(fileList.at(index))) {
                preview->contentWidget()->updateGeometry();
                updateTitle();
                // statusBar->openButton()->setFocus();
                preview->contentWidget()->adjustSize();
                int newPerviewWidth = preview->contentWidget()->size().width();
                int newPerviewHeight = preview->contentWidget()->size().height();
                setFixedSize(newPerviewWidth, newPerviewHeight + statusBar->height());
                playCurrentPreviewFile();
                restoreCenterPos();
                qCInfo(logLibFilePreview) << "FilePreviewDialog: successfully reused preview for file:" << fileList.at(index).toString();
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
            qCDebug(logLibFilePreview) << "FilePreviewDialog: created new preview for key:" << key;
            view->initialize(this, statusBar);

            if (info->canAttributes(CanableInfoType::kCanRedirectionFileUrl) && view->setFileUrl(info->urlOf(UrlInfoType::kRedirectedFileUrl))) {
                qCDebug(logLibFilePreview) << "FilePreviewDialog: using redirected URL for preview";
                break;
            } else if (view->setFileUrl(fileList.at(index))) {
                qCDebug(logLibFilePreview) << "FilePreviewDialog: successfully set file URL for preview";
                break;
            } else {
                qCDebug(logLibFilePreview) << "FilePreviewDialog: preview failed to handle file, trying next option";
                view->deleteLater();
            }
        }
    }

    if (!view) {
        if (preview && qobject_cast<UnknowFilePreview *>(preview)) {
            qCDebug(logLibFilePreview) << "FilePreviewDialog: reusing unknown file preview";
            preview->setFileUrl(fileList.at(index));
            // statusBar->openButton()->setFocus();
            return;
        } else {
            qCInfo(logLibFilePreview) << "FilePreviewDialog: no suitable preview found, using unknown file preview";
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

    // 切换并调整大小后，尝试将窗口移动回之前的中心位置
    restoreCenterPos();

    qCInfo(logLibFilePreview) << "FilePreviewDialog: successfully switched to page" << index << "for file:" << fileList.at(index).toString();
}

void FilePreviewDialog::previousPage()
{
    if (currentPageIndex < 1) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: already at first page, cannot go to previous";
        return;
    }
    if (playingVideo) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: video is playing, ignoring previous page request";
        return;
    }
    qCDebug(logLibFilePreview) << "FilePreviewDialog: navigating to previous page:" << (currentPageIndex - 1);
    firstEnterSwitchToPage = false;
    switchToPage(currentPageIndex - 1);
}

void FilePreviewDialog::nextPage()
{
    if (currentPageIndex > fileList.count() - 2) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: already at last page, cannot go to next";
        return;
    }
    if (playingVideo) {
        qCDebug(logLibFilePreview) << "FilePreviewDialog: video is playing, ignoring next page request";
        return;
    }
    qCDebug(logLibFilePreview) << "FilePreviewDialog: navigating to next page:" << (currentPageIndex + 1);
    firstEnterSwitchToPage = false;
    switchToPage(currentPageIndex + 1);
}

void FilePreviewDialog::updateTitle()
{
    // 在频繁启动关闭的场景下，preview可能会意外释放，引起空指针造成的崩溃
    if (!preview) {
        qCWarning(logLibFilePreview) << "FilePreviewDialog: preview is null during title update";
        return;
    }

    QFont font = statusBar->title()->font();
    QFontMetrics fm(font);
    QString elidedText;

    if (!statusBar->preButton()->isVisible()) {
        /*smb 中一直按着空格预览，preview 已经析构了，但是定时器的timeout事件已经执行，这里使用智能指针进行判断*/
        if (!preview) {
            qCWarning(logLibFilePreview) << "FilePreviewDialog: preview became null during title update";
            return;
        }
        elidedText = fm.elidedText(preview->title(), Qt::ElideMiddle, width() / 2 - statusBar->contentsMargins().left() - statusBar->layout()->spacing() - 30);
    } else {
        elidedText = fm.elidedText(preview->title(), Qt::ElideMiddle, width() / 2 - statusBar->preButton()->width() - statusBar->nextButton()->width() - statusBar->contentsMargins().left() - statusBar->layout()->spacing() * 3 - 30);
    }
    statusBar->title()->setText(elidedText);
    statusBar->title()->setHidden(statusBar->title()->text().isEmpty());

    qCDebug(logLibFilePreview) << "FilePreviewDialog: updated title to:" << elidedText;
}

QString FilePreviewDialog::generalKey(const QString &key)
{
    const QStringList &tmp = key.split('/');

    if (tmp.size() > 1)
        return tmp.first() + "/*";

    return key;
}

void FilePreviewDialog::saveCenterPos()
{
    if (!isVisible()) {
        // 窗口不可见时不记录
        return;
    }

    QRectF rectF = geometry();
    previousCenter = rectF.center();
    qCDebug(logLibFilePreview) << "FilePreviewDialog: saved center position:" << previousCenter;
}

void FilePreviewDialog::restoreCenterPos()
{
    if (previousCenter.isNull()) {
        // 没有记录过中心点
        return;
    }

    // 计算新的左上角位置以保持中心点不变，使用浮点数计算保持精度
    QSizeF sizeF = size();
    QPointF newTopLeftF(previousCenter.x() - sizeF.width() / 2.0, previousCenter.y() - sizeF.height() / 2.0);
    QPoint newTopLeft = newTopLeftF.toPoint();

    // 限制窗口位置到记录中心所在屏幕的可用区域，避免跨屏显示
    QScreen *scr = QGuiApplication::screenAt(previousCenter.toPoint());
    QRect avail;
    if (scr) {
        avail = scr->availableGeometry();
    } else if (QGuiApplication::primaryScreen()) {
        avail = QGuiApplication::primaryScreen()->availableGeometry();
    }

    if (!avail.isNull()) {
        QSize s = size();
        // 如果窗口大小大于可用区域，则将窗口居中显示在该屏幕内
        if (s.width() > avail.width() || s.height() > avail.height()) {
            QPoint centered(avail.center().x() - s.width() / 2, avail.center().y() - s.height() / 2);
            newTopLeft = centered;
        } else {
            // 将 newTopLeft 限定在 avail 内
            int x = qBound(avail.left(), newTopLeft.x(), avail.right() - s.width());
            int y = qBound(avail.top(), newTopLeft.y(), avail.bottom() - s.height());
            newTopLeft = QPoint(x, y);
        }
    }

    // 将窗口移动到新位置，但确保在主线程和可见时操作
    if (isVisible()) {
        move(newTopLeft);
        qCDebug(logLibFilePreview) << "FilePreviewDialog: restored window to keep center at" << previousCenter << "moved to" << newTopLeft;
    }
}

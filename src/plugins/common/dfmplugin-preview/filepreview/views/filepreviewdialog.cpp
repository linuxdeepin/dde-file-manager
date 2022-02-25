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
#include "filepreviewdialog.h"
#include "dfm-base/file/local/localfilehandler.h"
#include "dfm-base/base/schemefactory.h"
#include "pluginInterface/filepreviewfactory.h"
#include "unknowfilepreview.h"
#include "dfm-base/utils/fileutils.h"

#include <DWindowCloseButton>
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
DPFILEPREVIEW_USE_NAMESPACE

FilePreviewDialog::FilePreviewDialog(const QList<QUrl> &previewUrllist, QWidget *parent)
    : DAbstractDialog(parent),
      fileList(previewUrllist),
      dialogManager(DialogManagerInstance)
{
    initUI();
    if (previewUrllist.count() < 2) {
        statusBar->preButton()->hide();
        statusBar->nextButton()->hide();
    }
    firstEnterSwitchToPage = true;
    switchToPage(0);
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

void FilePreviewDialog::setEntryUrlList(const QList<QUrl> &UrlList)
{
    if (UrlList.isEmpty())
        return;
    QUrl currentUrl = fileList.at(currentPageIndex);
    if (UrlList.contains(currentUrl)) {
        entryUrlList = UrlList;
        fileList = entryUrlList;
        currentPageIndex = entryUrlList.indexOf(currentUrl);
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

void FilePreviewDialog::playCurrentPreviewFile()
{
    if (preview) {
        // 修复bug-63504 bug-63352
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

void FilePreviewDialog::childEvent(QChildEvent *event)
{
    if (closeButton && event->added()) {
        closeButton->raise();
    }

    return DAbstractDialog::childEvent(event);
}

void FilePreviewDialog::showEvent(QShowEvent *event)
{
    windowHandle()->installEventFilter(this);

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
    QTimer::singleShot(50, this, [=]() {   //fix 32985 【文件管理器】【5.1.1.86-1】【sp2】空格预览界面展示异常。50ms这个时间视机器性能而定
        repaint();   //通过重绘来解决调整大小前的窗口残留的问题
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
        case Qt::Key_Space: {
            // 视频预览的前一秒禁止再次播放
            if (playingVideo) {
                break;
            }
            if (preview) {
                preview->stop();
            }
            close();
            return true;
        }
        case Qt::Key_Escape:
            close();
            break;
        default:
            break;
        }
    }

    return DAbstractDialog::eventFilter(obj, event);
}

void FilePreviewDialog::initUI()
{
    closeButton = new DWindowCloseButton(this);
    closeButton->setObjectName("CloseButton");
    closeButton->setFocusPolicy(Qt::NoFocus);
    closeButton->setIconSize({ 50, 50 });
    closeButton->setFixedSize({ 50, 50 });
    QColor base_color = palette().base().color();
    DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);
    if (ct == DGuiApplicationHelper::LightType) {
        closeButton->setStyleSheet("background-color:rgba(255, 255, 255, 25);");
    } else {
        closeButton->setStyleSheet("background-color:rgba(0, 0, 0, 25);");
    }

    DAnchorsBase::setAnchor(closeButton, Qt::AnchorRight, this, Qt::AnchorRight);
    connect(closeButton, &QPushButton::clicked, this, &FilePreviewDialog::close);

    separator = new DHorizontalLine(this);
    separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    statusBar = new FilePreviewDialogStatusBar(this);
    statusBar->setObjectName("StatusBar");
    statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    statusBar->openButton()->setFocus();

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QHBoxLayout *separator_layout = new QHBoxLayout(this);

    separator_layout->addSpacing(10);
    separator_layout->addWidget(separator);
    separator_layout->addSpacing(10);

    layout->addLayout(separator_layout, 1);
    layout->addWidget(statusBar, 0, Qt::AlignBottom);

    QAction *shortcut_action = new QAction(this);

    shortcut_action->setShortcut(QKeySequence::Copy);
    addAction(shortcut_action);

    connect(statusBar->preButton(), &QPushButton::clicked, this, &FilePreviewDialog::previousPage);
    connect(statusBar->nextButton(), &QPushButton::clicked, this, &FilePreviewDialog::nextPage);
    connect(statusBar->openButton(), &QPushButton::clicked, this, [this] {
        /*fix bug 47136 在回收站预览打开不了，url传入错误，因为在回收站里面实现了openfile，所以这里倒回到以前代码*/
        DFMBASE_NAMESPACE::LocalFileHandler fileHandler;
        bool ok = fileHandler.openFile(fileList.at(currentPageIndex));
        if (!ok) {
            QString error;
            error = fileHandler.errorString();
            dialogManager->showErrorDialog("open file error", error);
        }
        close();
    });

    //    connect(shortcut_action, &QAction::triggered, this, [this] {
    //        if (preview) {
    //            preview->copyFile();
    //        }
    //    });
}

void FilePreviewDialog::switchToPage(int index)
{
    currentPageIndex = index;
    statusBar->preButton()->setEnabled(index > 0);
    statusBar->nextButton()->setEnabled(index < fileList.count() - 1);

    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(fileList.at(index));
    if (info.isNull())
        return;

    if (!info) {
        fileList.removeAt(index);

        if (fileList.isEmpty())
            return;

        return switchToPage(index);
    }

    AbstractBasePreview *view = nullptr;
    const QMimeType &mime_type = MimeDatabase::mimeTypeForUrl(fileList.at(index));

    QStringList key_list(mime_type.name());

    key_list.append(mime_type.aliases());
    key_list.append(mime_type.allAncestors());

    for (const QString &key : key_list) {
        const QString &general_key = generalKey(key);

        if (preview && (FilePreviewFactory::isSuitedWithKey(preview, key) || FilePreviewFactory::isSuitedWithKey(preview, general_key)) && !FileUtils::isDesktopFile(fileList.at(index))) {
            if (preview->setFileUrl(fileList.at(index))) {
                preview->contentWidget()->updateGeometry();
                adjustSize();
                updateTitle();
                statusBar->openButton()->setFocus();
                playCurrentPreviewFile();
                moveToCenter();
                return;
            }
        }

        view = FilePreviewFactory::create(key);

        if (!view && general_key != key) {
            if (!FileUtils::isDesktopFile(fileList.at(index))) {
                view = FilePreviewFactory::create(general_key);
            }
        }

        if (view) {
            view->initialize(this, statusBar);

            if (info->canRedirectionFileUrl() && view->setFileUrl(info->redirectedFileUrl()))
                break;
            else if (view->setFileUrl(fileList.at(index)))
                break;
            else
                view->deleteLater();
        }
    }

    if (!view) {
        if (qobject_cast<UnknowFilePreview *>(preview)) {
            preview->setFileUrl(fileList.at(index));
            statusBar->openButton()->setFocus();
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
        preview->deleteLater();
        static_cast<QVBoxLayout *>(layout())->removeWidget(preview->contentWidget());
    }

    static_cast<QVBoxLayout *>(layout())->insertWidget(0, view->contentWidget());

    if (preview)
        static_cast<QHBoxLayout *>(statusBar->layout())->removeWidget(preview->statusBarWidget());

    if (QWidget *w = view->statusBarWidget())
        static_cast<QHBoxLayout *>(statusBar->layout())->insertWidget(3, w, 0, view->statusBarWidgetAlignment());

    separator->setVisible(view->showStatusBarSeparator());
    preview = view;

    QTimer::singleShot(0, this, [this] {
        updateTitle();
        statusBar->openButton()->setFocus();
        int perviewwidth = preview->contentWidget()->size().width();
        int perviewheight = preview->contentWidget()->size().height();
        this->resize(perviewwidth, perviewheight);
        adjustSize();
        preview->contentWidget()->adjustSize();
        int newPerviewWidth = preview->contentWidget()->size().width();
        int newPerviewHeight = preview->contentWidget()->size().height();

        if (perviewwidth != newPerviewWidth || perviewheight != newPerviewHeight) {
            resize(newPerviewWidth, newPerviewHeight);
        }

        playCurrentPreviewFile();
        moveToCenter();
    });
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
        /*fix bug 46804 smb 中一直按着空格预览，preview 已经析构了，但是定时器的timeout事件已经执行，这里使用智能指针进行判断*/
        if (!preview) {
            qDebug() << "preview is null,so exit";
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
    const QStringList &_tmp = key.split('/');

    if (_tmp.size() > 1)
        return _tmp.first() + "/*";

    return key;
}

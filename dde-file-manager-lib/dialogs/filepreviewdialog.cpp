/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "filepreviewdialog.h"
#include "dfileservices.h"
#include "dabstractfileinfo.h"
#include "dfmfilepreviewfactory.h"
#include "dfilestatisticsjob.h"

#include "shutil/fileutils.h"

#include <danchors.h>
#include <dthememanager.h>
#include <denhancedwidget.h>

#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QAction>
#include <QWindow>
#include <QTimer>
#include <QMediaPlayer>
#include <QMultimedia>
#include <QGuiApplication>
#include <QCursor>
#include <QDesktopWidget>

DFM_BEGIN_NAMESPACE

class FilePreviewDialogStatusBar : public QFrame
{
    Q_OBJECT
public:
    explicit FilePreviewDialogStatusBar(QWidget *parent = 0);

    QLabel *title() const;
    QPushButton *preButton() const;
    QPushButton *nextButton() const;
    QPushButton *openButton() const;

private:
    QLabel *m_title;

    QPushButton *m_preButton;
    QPushButton *m_nextButton;
    QPushButton *m_openButton;

};

FilePreviewDialogStatusBar::FilePreviewDialogStatusBar(QWidget *parent)
    : QFrame(parent)
{
    m_preButton = new QPushButton(this);
    m_preButton->setObjectName("PreButton");
    m_preButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_preButton->setShortcut(QKeySequence::Back);
    m_preButton->setFocusPolicy(Qt::NoFocus);

    m_nextButton = new QPushButton(this);
    m_nextButton->setObjectName("NextButton");
    m_nextButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nextButton->setShortcut(QKeySequence::Forward);
    m_nextButton->setFocusPolicy(Qt::NoFocus);

    m_title = new QLabel(this);
    m_title->setObjectName("TitleLabel");
    m_title->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_title->hide();

    m_openButton = new QPushButton(QObject::tr("Open"), this);
    m_openButton->setObjectName("OpenButton");
    m_openButton->setFixedHeight(24);
    m_openButton->setShortcut(QKeySequence::Open);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(10, 0, 10, 0);
    layout->addWidget(m_preButton);
    layout->addWidget(m_nextButton);
    layout->addWidget(m_title);
    layout->addWidget(m_openButton, 0, Qt::AlignRight);

    setLayout(layout);
}

QPushButton *FilePreviewDialogStatusBar::openButton() const
{
    return m_openButton;
}

QPushButton *FilePreviewDialogStatusBar::nextButton() const
{
return m_nextButton;
}

QPushButton *FilePreviewDialogStatusBar::preButton() const
{
return m_preButton;
}

QLabel *FilePreviewDialogStatusBar::title() const
{
return m_title;
}

UnknowFilePreview::UnknowFilePreview(QObject *parent)
    : DFMFilePreview(parent)
{
    m_contentWidget = new QWidget();
    m_contentWidget->setFixedSize(550, 200);
    m_iconLabel = new QLabel(m_contentWidget);
    m_iconLabel->setObjectName("IconLabel");
    m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nameLabel = new QLabel(m_contentWidget);
    m_nameLabel->setObjectName("NameLabel");
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setWordWrap(true);
    m_sizeLabel = new QLabel(m_contentWidget);
    m_sizeLabel->setObjectName("SizeLabel");
    m_sizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_typeLabel = new QLabel(m_contentWidget);
    m_typeLabel->setObjectName("TypeLabel");
    m_typeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QVBoxLayout *vlayout = new QVBoxLayout();

    vlayout->addWidget(m_nameLabel);
    vlayout->addWidget(m_sizeLabel);
    vlayout->addWidget(m_typeLabel);
    vlayout->addStretch();

    QHBoxLayout *hlayout = new QHBoxLayout(m_contentWidget);

    hlayout->setContentsMargins(30, 20, 20, 20);
    hlayout->addWidget(m_iconLabel);
    hlayout->addSpacing(30);
    hlayout->addLayout(vlayout);
    hlayout->addStretch();
}

UnknowFilePreview::~UnknowFilePreview()
{
    if (m_contentWidget){
        m_contentWidget->deleteLater();
    }

    if (m_sizeWorker) {
        m_sizeWorker->stop();
        m_sizeWorker->deleteLater();
    }
}

bool UnknowFilePreview::setFileUrl(const DUrl &url)
{
    m_url = url;

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

    if (info)
        setFileInfo(info);

    return true;
}

DUrl UnknowFilePreview::fileUrl() const
{
    return m_url;
}

void UnknowFilePreview::setFileInfo(const DAbstractFileInfoPointer &info)
{
    const QIcon &icon = info->fileIcon();

    m_iconLabel->setPixmap(icon.pixmap(150));

    QFont font = m_nameLabel->font();
    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(info->fileName(), Qt::ElideMiddle, 300);

    m_nameLabel->setText(elidedText);

    if (info->isFile() || info->isSymLink()) {
        m_sizeLabel->setText(QObject::tr("Size: %1").arg(info->sizeDisplayName()));
        m_typeLabel->setText(QObject::tr("Type: %1").arg(info->mimeTypeDisplayName()));
    } else if (info->isDir()) {
        if (!m_sizeWorker) {
            m_sizeWorker = new DFileStatisticsJob(this);

            connect(m_sizeWorker, &DFileStatisticsJob::dataNotify, this, &UnknowFilePreview::updateFolderSize);
        } else if (m_sizeWorker->isRunning()) {
            m_sizeWorker->stop();
            m_sizeWorker->wait();
        }

        m_sizeWorker->start({info->fileUrl()});
        m_sizeLabel->setText(QObject::tr("Size: 0"));
    }
}

void UnknowFilePreview::updateFolderSize(qint64 size)
{
    m_typeLabel->setText(QObject::tr("Items: %1").arg(m_sizeWorker->filesCount() + m_sizeWorker->directorysCount()));
    m_sizeLabel->setText(QObject::tr("Size: %1").arg(FileUtils::formatSize(size)));
}


QWidget *UnknowFilePreview::contentWidget() const
{
    return m_contentWidget;
}

FilePreviewDialog::FilePreviewDialog(const DUrlList &previewUrllist, QWidget *parent)
    : DAbstractDialog(parent)
    , m_fileList(previewUrllist)
{
    D_THEME_INIT_WIDGET(FilePreviewDialog);

#ifdef Q_OS_LINUX
#ifndef ARCH_SW     // 申威和龙芯架构已禁用视频预览功能，不会触发此问题
#ifndef ARCH_MIPSEL //
    // 先触发Qt多媒体库加载gstreamter插件
    // 因为预览视频时会先加载mpv库，如果之后再加载gst库，会导致崩溃在init_plugin
    // 崩溃的原因是libcdio这个demuxer的long_name字段为NULL，gst-libav代码segfault了。
    // 猜测原因是：
    // 影院用的mpv初始化时，初始化了全部的demuxers，包括libavdevice里带的，特别是其中的libcdio。
    // Qt用了gst-libav，这个库加载时也会掉ffmpeg的初始化函数，但是只调用了部分，它编译时显示去掉了libavdevice依赖，所以初始化时不会加载libcdio的demuxer。
    //问题来了：如果先跑gst-libav去初始化ffmpeg，那么没有崩溃问题。因为libcdio demuxer没加载。如果先跑影院库预览视频后，libcdio demuxer被加载了。于是就崩了。
//    QMediaPlayer::hasSupport("audio/mpeg");
    // NOTE(zccrs): 不再需要此特殊处理，已给 gst-libav 添加 fix-crash-on-load-libcdio-plugin.patch
#endif
#endif
#endif

    initUI();

    if (previewUrllist.count() < 2) {
        m_statusBar->preButton()->hide();
        m_statusBar->nextButton()->hide();
    }

    switchToPage(0);
}

FilePreviewDialog::~FilePreviewDialog()
{
    if (m_preview){
        m_preview->deleteLater();
        QGuiApplication::changeOverrideCursor(QCursor(Qt::ArrowCursor));
    }
}

void FilePreviewDialog::updatePreviewList(const DUrlList &previewUrllist)
{
    m_fileList = previewUrllist;
    m_currentPageIndex = -1;

    if (previewUrllist.count() < 2) {
        m_statusBar->preButton()->hide();
        m_statusBar->nextButton()->hide();
    }else{
        m_statusBar->preButton()->show();
        m_statusBar->nextButton()->show();
    }

    switchToPage(0);
}

void FilePreviewDialog::childEvent(QChildEvent *event)
{
    if (m_closeButton && event->added()) {
        m_closeButton->raise();
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
    if (m_preview) {
        m_preview->stop();
        m_preview->deleteLater();
        m_preview = nullptr;
    }

    return DAbstractDialog::closeEvent(event);
}

bool FilePreviewDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        const QKeyEvent *e = static_cast<QKeyEvent*>(event);
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
        case Qt::Key_Space:{
            m_preview->stop();
            close();
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
    m_closeButton = new QPushButton(this);
    m_closeButton->setObjectName("CloseButton");
    m_closeButton->setFocusPolicy(Qt::NoFocus);

    m_separator = new DSeparatorHorizontal(this);
    m_separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_statusBar = new FilePreviewDialogStatusBar(this);
    m_statusBar->setObjectName("StatusBar");
    m_statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_statusBar->openButton()->setFocus();

    DAnchorsBase::setAnchor(m_closeButton, Qt::AnchorRight, this, Qt::AnchorRight);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QHBoxLayout *separator_layout = new QHBoxLayout();

    separator_layout->addSpacing(10);
    separator_layout->addWidget(m_separator);
    separator_layout->addSpacing(10);

    layout->addLayout(separator_layout, 1);
    layout->addWidget(m_statusBar, 0, Qt::AlignBottom);

    QAction *shortcut_action = new QAction(this);

    shortcut_action->setShortcut(QKeySequence::Copy);
    addAction(shortcut_action);

    connect(m_closeButton, &QPushButton::clicked, this, &FilePreviewDialog::close);
    connect(m_statusBar->preButton(), &QPushButton::clicked, this, &FilePreviewDialog::previousPage);
    connect(m_statusBar->nextButton(), &QPushButton::clicked, this, &FilePreviewDialog::nextPage);
    connect(m_statusBar->openButton(), &QPushButton::clicked, this, [this] {
        DFileService::instance()->openFile(this, m_fileList.at(m_currentPageIndex));
        close();
    });
    connect(shortcut_action, &QAction::triggered, this, [this] {
        if (m_preview) {
            m_preview->copyFile();
        }
    });
}

static QString generalKey(const QString &key)
{
    const QStringList &_tmp = key.split('/');

    if (_tmp.size() > 1)
        return _tmp.first() + "/*";

    return key;
}

void FilePreviewDialog::switchToPage(int index)
{
    if (m_preview) {
        m_preview->stop();
    }

    m_currentPageIndex = index;
    m_statusBar->preButton()->setEnabled(index > 0);
    m_statusBar->nextButton()->setEnabled(index < m_fileList.count() - 1);

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, m_fileList.at(index));

    if (!info) {
        m_fileList.removeAt(index);

        if (m_fileList.isEmpty())
            return;

        return switchToPage(index);
    }

    DFMFilePreview *preview = 0;
    const QMimeType &mime_type = info->mimeType();

    QStringList key_list(mime_type.name());

    key_list.append(mime_type.aliases());
    key_list.append(mime_type.allAncestors());

    for (const QString &key : key_list) {
        const QString &general_key = generalKey(key);

        if (m_preview && (DFMFilePreviewFactory::isSuitedWithKey(m_preview, key)
                          || DFMFilePreviewFactory::isSuitedWithKey(m_preview, general_key))) {
            if (m_preview->setFileUrl(m_fileList.at(index))) {
                m_preview->contentWidget()->updateGeometry();
                adjustSize();
                updateTitle();
                m_statusBar->openButton()->setFocus();
                playCurrentPreviewFile();
                return;
            }
        }

        preview = DFMFilePreviewFactory::create(key);

        if (!preview && general_key != key) {
            preview = DFMFilePreviewFactory::create(general_key);
        }

        if (preview) {
            preview->initialize(this, m_statusBar);

            if (preview->setFileUrl(m_fileList.at(index)))
                break;
            else if (info->canRedirectionFileUrl() && preview->setFileUrl(info->redirectedFileUrl()))
                break;
            else
                preview->deleteLater();
        }
    }
    if (!preview) {
        if (qobject_cast<UnknowFilePreview*>(m_preview)) {
            m_preview->setFileUrl(m_fileList.at(index));
            m_statusBar->openButton()->setFocus();
            return;
        } else {

            preview = new UnknowFilePreview(this);
            preview->initialize(this, m_statusBar);
            preview->setFileUrl(m_fileList.at(index));
        }
    }

    if (m_preview)
        disconnect(m_preview, &DFMFilePreview::titleChanged, this, &FilePreviewDialog::updateTitle);

    connect(preview, &DFMFilePreview::titleChanged, this, &FilePreviewDialog::updateTitle);

    if (m_preview)
        m_preview->deleteLater();

    if (m_preview) {
        static_cast<QVBoxLayout*>(layout())->removeWidget(m_preview->contentWidget());
    }

    static_cast<QVBoxLayout*>(layout())->insertWidget(0, preview->contentWidget());

    if (m_preview)
        static_cast<QHBoxLayout*>(m_statusBar->layout())->removeWidget(m_preview->statusBarWidget());

    if (QWidget * w = preview->statusBarWidget())
        static_cast<QHBoxLayout*>(m_statusBar->layout())->insertWidget(3, w, 0, preview->statusBarWidgetAlignment());

    m_separator->setVisible(preview->showStatusBarSeparator());
    m_preview = preview;

    QTimer::singleShot(0, this, [preview, this] {
        resize(sizeHint());
        updateTitle();
        m_statusBar->openButton()->setFocus();
        playCurrentPreviewFile();
    });
}

void FilePreviewDialog::setEntryUrlList(const DUrlList &entryUrlList)
{
    if (entryUrlList.isEmpty())
        return;
    DUrl currentUrl = m_fileList.at(m_currentPageIndex);
    if (entryUrlList.contains(currentUrl)){
        m_entryUrlList = entryUrlList;
        m_fileList = m_entryUrlList;
        m_currentPageIndex = m_entryUrlList.indexOf(currentUrl);
    }
}

void FilePreviewDialog::done(int r)
{
    DAbstractDialog::done(r);

    if (m_preview) {
        m_preview->stop();
        m_preview->deleteLater();
        m_preview = nullptr;
    }
}

void FilePreviewDialog::playCurrentPreviewFile()
{
    m_preview->play();
}

void FilePreviewDialog::previousPage()
{
    if (m_currentPageIndex < 1)
        return;

    switchToPage(m_currentPageIndex - 1);
}

void FilePreviewDialog::nextPage()
{
    if (m_currentPageIndex > m_fileList.count() - 2)
        return;

    switchToPage(m_currentPageIndex + 1);
}

void FilePreviewDialog::updateTitle()
{
    QFont font = m_statusBar->title()->font();
    QFontMetrics fm(font);
    QString elidedText;
    if (!m_statusBar->preButton()->isVisible()){
        elidedText = fm.elidedText(m_preview->title(), Qt::ElideMiddle, width() / 2 -
                                   m_statusBar->contentsMargins().left() - m_statusBar->layout()->spacing() -30);
    }else{
        elidedText = fm.elidedText(m_preview->title(), Qt::ElideMiddle, width() / 2 - m_statusBar->preButton()->width() -
                                   m_statusBar->nextButton()->width() - m_statusBar->contentsMargins().left() - m_statusBar->layout()->spacing() * 3 - 30);
    }
    m_statusBar->title()->setText(elidedText);
    m_statusBar->title()->setHidden(m_statusBar->title()->text().isEmpty());
}

DFM_END_NAMESPACE

#include "filepreviewdialog.moc"

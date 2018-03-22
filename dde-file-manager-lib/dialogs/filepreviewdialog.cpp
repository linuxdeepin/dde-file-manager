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
#include "shutil/filessizeworker.h"
#include "shutil/fileutils.h"

#include <anchors.h>
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

    m_sizeWorker = new FilesSizeWorker;
    m_sizeThread = new QThread;
    m_sizeWorker->moveToThread(m_sizeThread);
    connect(this, &UnknowFilePreview::requestStartFolderSize, m_sizeWorker, &FilesSizeWorker::coumpueteSize);
    connect(m_sizeWorker, &FilesSizeWorker::sizeUpdated, this, &UnknowFilePreview::updateFolderSize);
    m_sizeThread->start();
}

UnknowFilePreview::~UnknowFilePreview()
{
    if (m_contentWidget){
        m_sizeWorker->stop();
        m_sizeWorker->deleteLater();
        m_sizeThread->quit();
        m_sizeThread->deleteLater();
        m_contentWidget->deleteLater();
    }
}

bool UnknowFilePreview::setFileUrl(const DUrl &url)
{
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

    if (info)
        setFileInfo(info);

    return true;
}

void UnknowFilePreview::setFileInfo(const DAbstractFileInfoPointer &info)
{
    const QIcon &icon = info->fileIcon();

    m_iconLabel->setPixmap(icon.pixmap(150));

    QFont font = m_nameLabel->font();
    QFontMetrics fm(font);
    QString elidedText = fm.elidedText(info->fileName(), Qt::ElideMiddle, 300);

    m_nameLabel->setText(elidedText);

    if (info->isFile() || info->isSymLink()){
        m_sizeLabel->setText(QObject::tr("Size: %1").arg(info->sizeDisplayName()));
        m_typeLabel->setText(QObject::tr("Type: %1").arg(info->mimeTypeDisplayName()));
    }else if (info->isDir()){
        m_sizeWorker->stop();
        startFolderSize(info->fileUrl());
        m_sizeLabel->setText(QObject::tr("Size: 0"));
        m_typeLabel->setText(QObject::tr("Items: %1").arg(info->sizeDisplayName()));
    }
}


void UnknowFilePreview::startFolderSize(const DUrl &url)
{
    DUrl validUrl = url;
    if (url.isUserShareFile()){
        validUrl.setScheme(FILE_SCHEME);
    }
    DUrlList urls;
    urls << validUrl;
    m_sizeWorker->setUrls(urls);
    m_sizeWorker->setStopped(false);
    emit requestStartFolderSize();
}

void UnknowFilePreview::updateFolderSize(qint64 size)
{
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

void FilePreviewDialog::hideEvent(QHideEvent *event)
{
    m_preview->stop();
    return DAbstractDialog::hideEvent(event);
}

void FilePreviewDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_lastPosX = geometry().bottomLeft().x();
    m_lastPosY = geometry().bottomLeft().y();
    return DAbstractDialog::mouseReleaseEvent(event);
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
            hide();
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

    AnchorsBase::setAnchor(m_closeButton, Qt::AnchorRight, this, Qt::AnchorRight);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QHBoxLayout *separator_layout = new QHBoxLayout();

    separator_layout->addSpacing(10);
    separator_layout->addWidget(m_separator);
    separator_layout->addSpacing(10);

    layout->addLayout(separator_layout, 1);
    layout->addWidget(m_statusBar, 0, Qt::AlignBottom);

    connect(m_closeButton, &QPushButton::clicked, this, &FilePreviewDialog::close);
    connect(m_statusBar->preButton(), &QPushButton::clicked, this, &FilePreviewDialog::previousPage);
    connect(m_statusBar->nextButton(), &QPushButton::clicked, this, &FilePreviewDialog::nextPage);
    connect(m_statusBar->openButton(), &QPushButton::clicked, this, [this] {
        DFileService::instance()->openFile(this, m_fileList.at(m_currentPageIndex));
        close();
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
                adjsutPostion();
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
        adjsutPostion();
        playCurrentPreviewFile();
    });
}

void FilePreviewDialog::adjsutPostion()
{
    if (m_lastPosX != -1 && m_lastPosY != -1){
        move(m_lastPosX, m_lastPosY - height());
    }
    QDesktopWidget w;
    int screenWidth = w.screenGeometry().width();
    if (x() + width() >= screenWidth){
        m_lastPosX = screenWidth - width();
    }else{
        m_lastPosX = x();
    }

    if (y() <= 0){
        m_lastPosY = height();
    }else{
        m_lastPosY = y() + height();
    }
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

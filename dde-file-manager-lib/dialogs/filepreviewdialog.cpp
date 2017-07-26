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
#include "dfmfilepreview.h"
#include "dfmfilepreviewfactory.h"

#include <anchors.h>
#include <dthememanager.h>
#include <denhancedwidget.h>

#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QAction>
#include <QWindow>

DFM_BEGIN_NAMESPACE

class FilePreviewDialogStatusBar : public QFrame
{
    Q_OBJECT
public:
    explicit FilePreviewDialogStatusBar(QWidget *parent = 0);

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

    m_nextButton = new QPushButton(this);
    m_nextButton->setObjectName("NextButton");
    m_nextButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_nextButton->setShortcut(QKeySequence::Forward);

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
}

class UnknowFilePreview : public DFMFilePreview
{
    Q_OBJECT

public:
    explicit UnknowFilePreview(QObject *parent = 0);
    ~UnknowFilePreview();

    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;
    void setFileInfo(const DAbstractFileInfoPointer &info);

    QWidget *contentWidget() const Q_DECL_OVERRIDE;

private:
    QPointer<QWidget> m_contentWidget;
    QLabel *m_iconLabel;
    QLabel *m_nameLabel;
    QLabel *m_sizeLabel;
    QLabel *m_typeLabel;
};

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
    if (m_contentWidget)
        m_contentWidget->deleteLater();
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
    m_nameLabel->setText(info->fileName());
    m_sizeLabel->setText(QString(QObject::tr("Size: %1")).arg(info->sizeDisplayName()));
    m_typeLabel->setText(QString(QObject::tr("Type: %1").arg(info->mimeTypeDisplayName())));
}

QWidget *UnknowFilePreview::contentWidget() const
{
    return m_contentWidget;
}

FilePreviewDialog::FilePreviewDialog(const DUrlList &list, QWidget *parent)
    : DAbstractDialog(parent)
    , m_fileList(list)
{
    D_THEME_INIT_WIDGET(FilePreviewDialog);

    initUI();

    if (list.count() < 2) {
        m_statusBar->m_preButton->hide();
        m_statusBar->m_nextButton->hide();
    }

    switchToPage(0);
}

FilePreviewDialog::~FilePreviewDialog()
{
    if (m_preview)
        m_preview->deleteLater();
}

void FilePreviewDialog::resizeEvent(QResizeEvent *event)
{
    m_separator->setFixedWidth(event->size().width() - 20);

    return DAbstractDialog::resizeEvent(event);
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

bool FilePreviewDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        const QKeyEvent *e = static_cast<QKeyEvent*>(event);

        switch (e->key()) {
        case Qt::Key_Left:
        case Qt::Key_Up:
            previousPage();
            break;
        case Qt::Key_Right:
        case Qt::Key_Down:
            nextPage();
            break;
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
    m_closeButton->setShortcut(Qt::Key_Space);

    m_separator = new DSeparatorHorizontal(this);
    m_separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_statusBar = new FilePreviewDialogStatusBar(this);
    m_statusBar->setObjectName("StatusBar");
    m_statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    AnchorsBase::setAnchor(m_closeButton, Qt::AnchorRight, this, Qt::AnchorRight);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_separator, 1, Qt::AlignBottom | Qt::AlignHCenter);
    layout->addWidget(m_statusBar, 0, Qt::AlignBottom);

    connect(m_closeButton, &QPushButton::clicked, this, &FilePreviewDialog::close);
    connect(m_statusBar->m_preButton, &QPushButton::clicked, this, &FilePreviewDialog::previousPage);
    connect(m_statusBar->m_nextButton, &QPushButton::clicked, this, &FilePreviewDialog::nextPage);
    connect(m_statusBar->m_openButton, &QPushButton::clicked, this, [this] {
        DFileService::instance()->openFile(this, m_fileList.at(m_currentPageIndex));
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
    m_statusBar->m_preButton->setEnabled(index > 0);
    m_statusBar->m_nextButton->setEnabled(index < m_fileList.count() - 1);

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, m_fileList.at(index));

    if (!info || !info->isFile()) {
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
                adjustSize();
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

    if (m_preview)
        static_cast<QVBoxLayout*>(layout())->removeWidget(m_preview->contentWidget());

    static_cast<QVBoxLayout*>(layout())->insertWidget(0, preview->contentWidget());

    if (m_preview)
        static_cast<QHBoxLayout*>(m_statusBar->layout())->removeWidget(m_preview->statusBarWidget());

    if (QWidget * w = preview->statusBarWidget())
        static_cast<QHBoxLayout*>(m_statusBar->layout())->insertWidget(3, w, 0, preview->statusBarWidgetAlignment());

    m_separator->setVisible(preview->showStatusBarSeparator());
    m_preview = preview;

    updateTitle();
    adjustSize();
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
    m_statusBar->m_title->setText(m_preview->title());
    m_statusBar->m_title->setHidden(m_statusBar->m_title->text().isEmpty());
}

DFM_END_NAMESPACE

#include "filepreviewdialog.moc"

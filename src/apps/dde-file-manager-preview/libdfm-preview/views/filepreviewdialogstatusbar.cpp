// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreviewdialogstatusbar.h"

#include <DGuiApplicationHelper>

#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>

DGUI_USE_NAMESPACE
using namespace dfmplugin_filepreview;

namespace {
QIcon createColoredIcon(const QString &iconName, const QColor &color, const QSize &size, const QWidget *widget)
{
    const qreal dpr = widget ? widget->devicePixelRatioF() : qApp->devicePixelRatio();
    QPixmap pixmap = QIcon::fromTheme(iconName).pixmap(size, dpr);
    if (pixmap.isNull())
        return QIcon();

    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);

    return QIcon(pixmap);
}

QColor arrowIconColor(const QPushButton *button, bool hovered)
{
    const bool darkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    QColor color = darkTheme ? QColor(Qt::white) : QColor(Qt::black);
    if (!button || !button->isEnabled())
        color.setAlphaF(0.4);
    else if (!hovered)
        color.setAlphaF(0.7);
    return color;
}
}   // namespace

Q_DECLARE_LOGGING_CATEGORY(logLibFilePreview)

FilePreviewDialogStatusBar::FilePreviewDialogStatusBar(QWidget *parent)
    : QFrame(parent)
{
    qCDebug(logLibFilePreview) << "FilePreviewDialogStatusBar: initializing status bar";

    QSize iconSize(12, 12);
    preBtn = new QPushButton(this);
    preBtn->setAccessibleName("PreBtn");
    preBtn->setObjectName("PreButton");
    preBtn->setIconSize(iconSize);
    preBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    preBtn->setShortcut(QKeySequence::Back);
    preBtn->setFocusPolicy(Qt::NoFocus);

    nextBtn = new QPushButton(this);
    nextBtn->setAccessibleName("NextBtn");
    nextBtn->setObjectName("NextButton");
    nextBtn->setIconSize(iconSize);
    nextBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    nextBtn->setShortcut(QKeySequence::Forward);
    nextBtn->setFocusPolicy(Qt::NoFocus);

    previewTitle = new QLabel(this);
    previewTitle->setObjectName("TitleLabel");
    previewTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    previewTitle->hide();

    openBtn = new QPushButton(QObject::tr("Open", "button"), this);
    openBtn->setAccessibleName("OpenBtn");
    openBtn->setObjectName("OpenButton");
    QFont font = openBtn->font();
    font.setPixelSize(12);
    openBtn->setFont(font);
    openBtn->setMinimumWidth(60);
    openBtn->setShortcut(QKeySequence::Open);

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(preBtn);
    layout->addWidget(nextBtn);
    layout->addWidget(previewTitle);
    layout->addWidget(openBtn, 0, Qt::AlignRight);

    setLayout(layout);

    preBtn->installEventFilter(this);
    nextBtn->installEventFilter(this);
    updateNavButtonIcons();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [this] { updateNavButtonIcons(); });

    qCDebug(logLibFilePreview) << "FilePreviewDialogStatusBar: status bar initialization completed";
}

QLabel *FilePreviewDialogStatusBar::title() const
{
    return previewTitle;
}

QPushButton *FilePreviewDialogStatusBar::preButton() const
{
    return preBtn;
}

QPushButton *FilePreviewDialogStatusBar::nextButton() const
{
    return nextBtn;
}

QPushButton *FilePreviewDialogStatusBar::openButton() const
{
    return openBtn;
}

bool FilePreviewDialogStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == preBtn || watched == nextBtn) {
        switch (event->type()) {
        case QEvent::Enter:
            if (watched == preBtn)
                preBtnHovered = true;
            else
                nextBtnHovered = true;
            updateNavButtonIcons();
            break;
        case QEvent::Leave:
            if (watched == preBtn)
                preBtnHovered = false;
            else
                nextBtnHovered = false;
            updateNavButtonIcons();
            break;
        case QEvent::EnabledChange:
        case QEvent::StyleChange:
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
        case QEvent::ScreenChangeInternal:
        case QEvent::DevicePixelRatioChange:
            updateNavButtonIcons();
            break;
        default:
            break;
        }
    }

    return QFrame::eventFilter(watched, event);
}

void FilePreviewDialogStatusBar::updateNavButtonIcons()
{
    const QSize iconSize(12, 12);
    if (preBtn)
        preBtn->setIcon(createColoredIcon("go-previous", arrowIconColor(preBtn, preBtnHovered), iconSize, preBtn));
    if (nextBtn)
        nextBtn->setIcon(createColoredIcon("go-next", arrowIconColor(nextBtn, nextBtnHovered), iconSize, nextBtn));
}

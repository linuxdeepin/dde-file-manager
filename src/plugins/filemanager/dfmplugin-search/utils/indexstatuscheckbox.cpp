// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexstatuscheckbox.h"

#include <DDciIcon>

#include <QApplication>
#include <QSizePolicy>

DGUI_USE_NAMESPACE
namespace dfmplugin_search {
DWIDGET_USE_NAMESPACE

IndexStatusCheckBox::IndexStatusCheckBox(QWidget *parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);

    QSizePolicy policy = sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    setSizePolicy(policy);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    m_checkBox = new QCheckBox(this);
    layout->addWidget(m_checkBox);

    auto *statusWidget = new QWidget(this);
    statusWidget->setContentsMargins(4, 0, 0, 0);
    QSizePolicy statusPolicy = statusWidget->sizePolicy();
    statusPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    statusWidget->setSizePolicy(statusPolicy);

    m_statusLayout = new QHBoxLayout(statusWidget);
    m_statusLayout->setContentsMargins(0, 0, 0, 0);
    m_statusLayout->setSpacing(0);
    layout->addWidget(statusWidget);

    m_spinner = new DSpinner(statusWidget);
    m_spinner->setFixedSize(16, 16);

    m_iconLabel = new DTipLabel("", statusWidget);
    m_iconLabel->setFixedSize(16, 16);
    m_iconLabel->setPixmap(iconPixmap("dialog-ok", 16));

    m_msgLabel = new DTipLabel("", statusWidget);
    m_msgLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_msgLabel->setWordWrap(true);
    m_msgLabel->setContentsMargins(4, 0, 0, 0);
    m_msgLabel->setOpenExternalLinks(false);
    m_msgLabel->setTextFormat(Qt::RichText);

    QSizePolicy msgPolicy = m_msgLabel->sizePolicy();
    msgPolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    msgPolicy.setHorizontalStretch(1);
    m_msgLabel->setSizePolicy(msgPolicy);

    connect(m_msgLabel, &QLabel::linkActivated, this, [this](const QString &) {
        emit resetRequested();
    });

    connect(m_checkBox, &QCheckBox::checkStateChanged, this, &IndexStatusCheckBox::checkStateChanged);

    m_statusLayout->addWidget(m_spinner);
    m_statusLayout->addWidget(m_iconLabel);
    m_statusLayout->addWidget(m_msgLabel, 1);

    m_spinner->hide();
    m_iconLabel->show();
}

void IndexStatusCheckBox::setDisplayText(const QString &text)
{
    m_checkBox->setText(text);
}

void IndexStatusCheckBox::setChecked(bool checked)
{
    m_checkBox->setChecked(checked);
}

bool IndexStatusCheckBox::isChecked() const
{
    return m_checkBox->isChecked();
}

void IndexStatusCheckBox::setInactiveText(const QString &text)
{
    m_inactiveText = text;
    if (m_status == Status::Inactive)
        setStatus(Status::Inactive);
}

void IndexStatusCheckBox::setIndexingTexts(const QString &initialText,
                                           const QString &filesText,
                                           const QString &itemsText)
{
    m_indexingInitialText = initialText;
    m_indexingFilesText = filesText;
    m_indexingItemsText = itemsText;

    if (m_status == Status::Indexing)
        updateIndexingProgress(0, 0);
}

void IndexStatusCheckBox::setCompletedText(const QString &mainText, const QString &linkText, const QString &href)
{
    m_iconLabel->setPixmap(iconPixmap("dialog-ok", 16));
    setRunning(false);
    setFormattedTextWithLink(mainText, linkText, href);
}

void IndexStatusCheckBox::setFailedText(const QString &mainText, const QString &linkText, const QString &href)
{
    m_iconLabel->setPixmap(iconPixmap("dialog-error", 16));
    setRunning(false);
    setFormattedTextWithLink(mainText, linkText, href);
}

void IndexStatusCheckBox::setStatus(Status status)
{
    m_status = status;
    updateUI(status);

    switch (status) {
    case Status::Indexing:
        setRunning(true);
        updateIndexingProgress(0, 0);
        break;
    case Status::Completed:
        setRunning(false);
        m_iconLabel->show();
        m_iconLabel->setPixmap(iconPixmap("dialog-ok", 16));
        break;
    case Status::Failed:
        setRunning(false);
        m_iconLabel->show();
        m_iconLabel->setPixmap(iconPixmap("dialog-error", 16));
        break;
    case Status::Inactive:
        m_spinner->hide();
        m_spinner->stop();
        m_iconLabel->hide();
        m_msgLabel->setTextFormat(Qt::PlainText);
        m_msgLabel->setText(m_inactiveText);
        break;
    }
}

IndexStatusCheckBox::Status IndexStatusCheckBox::status() const
{
    return m_status;
}

void IndexStatusCheckBox::updateIndexingProgress(qlonglong count, qlonglong total)
{
    if (m_status != Status::Indexing)
        return;

    m_msgLabel->setTextFormat(Qt::PlainText);
    if (count == 0 && total == 0) {
        m_msgLabel->setText(m_indexingInitialText);
        return;
    }

    if (count != 0 && total == 0) {
        m_msgLabel->setText(m_indexingFilesText.arg(count));
        return;
    }

    m_msgLabel->setText(m_indexingItemsText.arg(count).arg(total));
}

void IndexStatusCheckBox::setRunning(bool running)
{
    if (running) {
        m_spinner->show();
        m_spinner->start();
        m_iconLabel->hide();
        return;
    }

    m_spinner->hide();
    m_spinner->stop();
    m_iconLabel->show();
}

void IndexStatusCheckBox::updateUI(Status status)
{
    if (!m_statusLayout)
        return;

    const int index = m_statusLayout->indexOf(m_msgLabel);
    if (status == Status::Inactive || status == Status::Indexing)
        m_statusLayout->setStretch(index, 1);
    else
        m_statusLayout->setStretch(index, 0);

    m_statusLayout->update();
}

QPixmap IndexStatusCheckBox::iconPixmap(const QString &iconName, int size)
{
    const auto ratio = qApp->devicePixelRatio();
    const auto &dciIcon = DDciIcon::fromTheme(iconName);
    QPixmap px;
    if (!dciIcon.isNull()) {
        const auto theme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType
                ? DDciIcon::Light
                : DDciIcon::Dark;
        px = dciIcon.pixmap(ratio, size, theme);
    } else {
        const auto &qicon = QIcon::fromTheme(iconName);
        px = qicon.pixmap(size);
        px.setDevicePixelRatio(ratio);
    }

    return px;
}

void IndexStatusCheckBox::setFormattedTextWithLink(const QString &mainText, const QString &linkText, const QString &href)
{
    m_msgLabel->setTextFormat(Qt::RichText);
    m_msgLabel->setText(QString("%1 %2 <a href=\"%3\">%4</a>").arg(linkStyle(), mainText, href, linkText));
}

QString IndexStatusCheckBox::linkStyle() const
{
    return QStringLiteral("<style> a {text-decoration: none; color: #0081FF;} a:hover {color: #40A9FF;} </style>");
}

}   // namespace dfmplugin_search

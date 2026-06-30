// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewhintwidget.h"

#include <DIconButton>
#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QIcon>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
constexpr int kLeftMargin = 12;
constexpr int kRightMargin = 12;
constexpr int kTopBottomMargin = 6;
constexpr int kItemSpacing = 10;
constexpr int kCustomWidgetGap = 40;
constexpr int kFramRadius = 12;
}   // namespace

DFMBASE_BEGIN_NAMESPACE
class MessageLabel : public QLabel
{
public:
    QSize sizeHint() const override
    {
        return fontMetrics().size({}, text());
    }
};
DFMBASE_END_NAMESPACE

ViewHintWidget::ViewHintWidget(QWidget *parent)
    : DFloatingMessage(DFloatingMessage::ResidentType, parent)
{
    setFramRadius(kFramRadius);
    initLayout();
}

void ViewHintWidget::initLayout()
{
    // 替换 DFloatingMessage 默认 content widget，使用自定义控件以获得精确布局控制。
    // 原因: 基类的 changeEvent() 会访问内部 d-pointer，复用其控件会导致悬空指针崩溃。

    m_iconButton = new DIconButton(this);
    m_iconButton->setFlat(true);
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_iconButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_iconButton->setIconSize({ 16, 16 });

    m_messageLabel = new MessageLabel();
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_closeButton = new DIconButton(this);
    m_closeButton->setFlat(true);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIcon(DDciIcon::fromTheme("dfm-clear"));
    m_closeButton->setIconSize({ 20, 20 });
    m_closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_contentWidget = new QWidget();
    m_layout = new QHBoxLayout(m_contentWidget);
    m_layout->setContentsMargins(kLeftMargin, kTopBottomMargin, kRightMargin, kTopBottomMargin);
    m_layout->setSpacing(kItemSpacing);
    m_layout->setAlignment(Qt::AlignVCenter);

    m_layout->addWidget(m_iconButton);
    m_layout->addWidget(m_messageLabel, 1);
    m_layout->addWidget(m_closeButton);

    // setWidget() 销毁旧 content 及其子控件，基类 d-pointer 失效
    DFloatingWidget::setWidget(m_contentWidget);

    connect(m_closeButton, &DIconButton::clicked, this, [this]() {
        close();
        Q_EMIT closeButtonClicked();
    });
}

void ViewHintWidget::setCustomWidget(QWidget *widget)
{
    if (m_customWidget) {
        m_layout->removeWidget(m_customWidget);
        m_customWidget->deleteLater();
    }

    m_customWidget = widget;

    if (widget) {
        // 在 message 和 close 之间插入，需额外 30px spacer 补足 40px 间距（基础 spacing 为 10px）
        int insertIdx = 2;
        m_layout->insertSpacing(insertIdx, kCustomWidgetGap - kItemSpacing);
        m_layout->insertWidget(insertIdx + 1, widget);
    }
}

QWidget *ViewHintWidget::customWidget() const
{
    return m_customWidget;
}

void ViewHintWidget::setIcon(const QString &icon)
{
    if (!m_iconButton)
        return;

    auto dciIcon = DDciIcon::fromTheme(icon);
    if (!dciIcon.isNull())
        m_iconButton->setIcon(dciIcon);
    else {
        auto qicon = QIcon::fromTheme(icon);
        m_iconButton->setIcon(qicon);
    }
}

void ViewHintWidget::setMessage(const QString &msg)
{
    if (m_messageLabel)
        m_messageLabel->setText(msg);
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewhintmessage.h"
#include "viewhintwidget.h"

#include <DMessageManager>
#include <DPushButton>
#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QPointer>
#include <QWidget>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

DFMBASE_BEGIN_NAMESPACE

class ViewHintMessagePrivate
{
public:
    QString icon;
    QString text;
    QList<QPair<QString, QString>> actions;   // {id, label}
    bool autoDismissOnAction = true;
    ViewHintCustomWidgetFactory leftCustomWidgetFactory;
    ViewHintCustomWidgetFactory rightCustomWidgetFactory;
    QPointer<DFMBASE_NAMESPACE::ViewHintWidget> message;
};

DFMBASE_END_NAMESPACE

ViewHintMessage::ViewHintMessage(QObject *parent)
    : QObject(parent), d(new ViewHintMessagePrivate)
{
}

void ViewHintMessage::rebuildRightWidget(ViewHintWidget *widget)
{
    if (!widget)
        return;

    QWidget *rightCustom = nullptr;
    if (d->rightCustomWidgetFactory) {
        rightCustom = d->rightCustomWidgetFactory(widget);
    }

    bool hasActions = !d->actions.isEmpty();
    if (!rightCustom && !hasActions) {
        widget->setCustomWidget(nullptr, ViewHintWidget::Side::Right);
        return;
    }

    auto *container = new QWidget(widget);
    auto *box = new QHBoxLayout(container);
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(10);

    if (rightCustom)
        box->addWidget(rightCustom);

    for (const auto &action : std::as_const(d->actions)) {
        const QString id = action.first;
        auto *button = new DPushButton(action.second, container);
        button->setFocusPolicy(Qt::NoFocus);
        button->setFixedHeight(28);
        DFontSizeManager::instance()->bind(button, DFontSizeManager::T6);
        connect(button, &DPushButton::clicked, this, [this, id]() {
            Q_EMIT actionTriggered(id);
            if (d->autoDismissOnAction && d->message)
                d->message->close();
        });
        box->addWidget(button);
    }

    widget->setCustomWidget(container, ViewHintWidget::Side::Right);
}

ViewHintMessage::~ViewHintMessage()
{
    if (d && d->message) {
        disconnect(d->message, &QObject::destroyed, this, nullptr);
        d->message->close();
    }
    delete d;
}

void ViewHintMessage::setIcon(const QString &icon)
{
    d->icon = icon;
    if (d->message)
        d->message->setIcon(icon);
}

void ViewHintMessage::setText(const QString &text)
{
    d->text = text;
    if (d->message)
        d->message->setMessage(text);
}

void ViewHintMessage::setActions(const QList<QPair<QString, QString>> &actions)
{
    d->actions = actions;
    if (d->message)
        rebuildRightWidget(d->message.data());
}

void ViewHintMessage::setAutoDismissOnAction(bool autoDismiss)
{
    d->autoDismissOnAction = autoDismiss;
}

void ViewHintMessage::setCustomWidgetFactory(const ViewHintCustomWidgetFactory &factory, Side side)
{
    auto &stored = (side == Side::Left) ? d->leftCustomWidgetFactory : d->rightCustomWidgetFactory;
    stored = factory;

    if (!d->message)
        return;

    if (side == Side::Left) {
        auto *w = factory ? factory(d->message.data()) : nullptr;
        d->message->setCustomWidget(w, ViewHintWidget::Side::Left);
    } else {
        rebuildRightWidget(d->message.data());
    }
}

void ViewHintMessage::setCustomWidget(QWidget *widget, Side side)
{
    if (d->message)
        d->message->setCustomWidget(widget, static_cast<ViewHintWidget::Side>(side));
}

bool ViewHintMessage::isVisible() const
{
    return d->message && d->message->isVisible();
}

void ViewHintMessage::show(QWidget *hostWidget)
{
    if (!hostWidget || d->message)
        return;

    // WA_DeleteOnClose: 确保关闭时真正删除消息，避免留下隐藏的消息
    auto *msg = new ViewHintWidget(hostWidget);
    DFontSizeManager::instance()->bind(msg, DFontSizeManager::T6);
    msg->setAttribute(Qt::WA_DeleteOnClose);
    msg->setIcon(d->icon);
    msg->setMessage(d->text);

    if (d->leftCustomWidgetFactory) {
        auto *leftWidget = d->leftCustomWidgetFactory(msg);
        if (leftWidget)
            msg->setCustomWidget(leftWidget, ViewHintWidget::Side::Left);
    }

    rebuildRightWidget(msg);

    // 内置关闭按钮点击 → actionTriggered("close")
    static const QString kCloseAction { "close" };
    connect(msg, &DFloatingMessage::closeButtonClicked, this, [this]() {
        Q_EMIT actionTriggered(kCloseAction);
    });

    // 消息销毁后自动清理 controller，调用方可持有 QPointer<ViewHintMessage>
    connect(msg, &QObject::destroyed, this, [this]() {
        Q_EMIT closed();
        deleteLater();
    });

    d->message = msg;
    DMessageManager::instance()->setContentMargens(hostWidget, { 0, 0, 0, 60 });
    DMessageManager::instance()->sendMessage(hostWidget, msg);
    Q_EMIT shown();
}

void ViewHintMessage::close()
{
    if (d->message)
        d->message->close();
}

void ViewHintMessage::refresh()
{
    if (!d->message)
        return;

    auto *w = d->leftCustomWidgetFactory ? d->leftCustomWidgetFactory(d->message.data()) : nullptr;
    d->message->setCustomWidget(w, ViewHintWidget::Side::Left);

    d->message->setIcon(d->icon);
    d->message->setMessage(d->text);
    rebuildRightWidget(d->message.data());
}

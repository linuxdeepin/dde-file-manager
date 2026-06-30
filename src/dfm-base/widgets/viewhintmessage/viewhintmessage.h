// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_BASE_VIEWHINTMESSAGE_H
#define DFM_BASE_VIEWHINTMESSAGE_H

#include <dfm-base/dfm_base_global.h>

#include <QIcon>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>

class QWidget;

DFMBASE_BEGIN_NAMESPACE

class ViewHintMessagePrivate;

/*!
 * \brief 可复用的浮动提示消息，挂载到宿主控件顶部。
 *
 * 布局: [icon] [message] [action buttons (可选)] [close button]
 *
 * 通过 actionTriggered(id) 信号转发用户点击，内置关闭按钮的 id 为 "close"。
 * 调用方负责处理业务逻辑（如切换配置），本组件不触及任何持久化状态。
 */
class ViewHintMessage : public QObject
{
    Q_OBJECT
    friend class ViewHintMessagePrivate;

public:
    explicit ViewHintMessage(QObject *parent = nullptr);
    ~ViewHintMessage() override;

    void setIcon(const QString &icon);
    void setText(const QString &text);
    void setActions(const QList<QPair<QString, QString>> &actions);
    void setAutoDismissOnAction(bool autoDismiss);
    bool isVisible() const;

public Q_SLOTS:
    void show(QWidget *hostWidget);
    void close();

Q_SIGNALS:
    void shown();
    void actionTriggered(const QString &id);

private:
    ViewHintMessagePrivate *d { nullptr };
};

DFMBASE_END_NAMESPACE

#endif   // DFM_BASE_VIEWHINTMESSAGE_H

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_BASE_VIEWHINTMESSAGE_H
#define DFM_BASE_VIEWHINTMESSAGE_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/widgets/viewhintmessage/viewhintwidget.h>

#include <QIcon>
#include <QList>
#include <QObject>
#include <QPair>
#include <QString>
#include <functional>

class QWidget;

DFMBASE_BEGIN_NAMESPACE

class ViewHintMessagePrivate;

using ViewHintCustomWidgetFactory = std::function<QWidget *(QWidget *parent)>;

/*!
 * \brief 可复用的浮动提示消息，挂载到宿主控件顶部。
 *
 * 布局: [leftCustom] [icon] [message] [action buttons / rightCustom] [close button]
 *
 * 左侧自定义控件位于最左边（图标之前），右侧自定义控件位于关闭按钮之前。
 * 调用方可通过 setCustomWidgetFactory(factory, side) 统一控制两侧内容，
 * 例如搜索插件设置左侧为 DSpinner 并清除图标。
 *
 * 通过 actionTriggered(id) 信号转发用户点击，内置关闭按钮的 id 为 "close"。
 * 调用方负责处理业务逻辑（如切换配置），本组件不触及任何持久化状态。
 */
class ViewHintMessage : public QObject
{
    Q_OBJECT
    friend class ViewHintMessagePrivate;

public:
    using Side = ViewHintWidget::Side;

    explicit ViewHintMessage(QObject *parent = nullptr);
    ~ViewHintMessage() override;

    void setIcon(const QString &icon);
    void setText(const QString &text);
    void setActions(const QList<QPair<QString, QString>> &actions);
    void setAutoDismissOnAction(bool autoDismiss);
    void setCustomWidgetFactory(const ViewHintCustomWidgetFactory &factory, Side side = Side::Right);
    bool isVisible() const;

public Q_SLOTS:
    void show(QWidget *hostWidget);
    void close();
    void refresh();
    void setCustomWidget(QWidget *widget, Side side = Side::Right);

Q_SIGNALS:
    void shown();
    void closed();
    void actionTriggered(const QString &id);

private:
    ViewHintMessagePrivate *d { nullptr };
    void rebuildRightWidget(ViewHintWidget *widget);
};

DFMBASE_END_NAMESPACE

#endif   // DFM_BASE_VIEWHINTMESSAGE_H

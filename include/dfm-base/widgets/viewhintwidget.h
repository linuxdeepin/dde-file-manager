// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_BASE_VIEWHINTWIDGET_H
#define DFM_BASE_VIEWHINTWIDGET_H

#include <dfm-base/dfm_base_global.h>

#include <DFloatingMessage>
#include <DDciIcon>
#include <DIconButton>

#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>

DFMBASE_BEGIN_NAMESPACE

class MessageLabel;

/*!
 * \brief 带精确布局控制的浮动消息控件。
 *
 * 继承 DFloatingMessage 并替换内部 content widget 以控制间距：
 *   [icon] [10px] [message] [40px] [custom widget] [10px] [close]
 */
class ViewHintWidget : public DTK_WIDGET_NAMESPACE::DFloatingMessage
{
    Q_OBJECT

public:
    explicit ViewHintWidget(QWidget *parent = nullptr);

    void setCustomWidget(QWidget *widget);
    QWidget *customWidget() const;

    void setIcon(const QString &icon);
    void setMessage(const QString &msg);

private:
    void initLayout();

    QWidget *m_contentWidget { nullptr };
    QHBoxLayout *m_layout { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *m_iconButton { nullptr };
    MessageLabel *m_messageLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DIconButton *m_closeButton { nullptr };
    QPointer<QWidget> m_customWidget;
};

DFMBASE_END_NAMESPACE

#endif   // DFM_BASE_VIEWHINTWIDGET_H

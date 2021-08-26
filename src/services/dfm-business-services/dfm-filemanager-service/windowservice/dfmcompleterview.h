/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DFMCOMPLETERVIEW_H
#define DFMCOMPLETERVIEW_H

#include <QCompleter>
#include <QListView>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFileSystemModel>

class DFMCompleterViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DFMCompleterViewDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {

    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        // prepare
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                                  ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = QPalette::Inactive;
        }

        // draw background
        if (option.showDecorationSelected && (option.state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
            painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
        }

        // draw text
        if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
            painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
        } else {
            painter->setPen(option.palette.color(cg, QPalette::Text));
        }

        painter->setFont(option.font);
        painter->drawText(option.rect.adjusted(31, 0, 0, 0), Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize s = QStyledItemDelegate::sizeHint(option, index);
        s.setHeight(24);

        return s;
    }
};

class DFMCompleterView :public QListView
{
    Q_OBJECT
public:
    explicit DFMCompleterView()
    {
        this->setModel(&m_model);

        m_completer.setModel(&m_model);
        m_completer.setModel(&m_model);
        m_completer.setPopup(this);
        m_completer.setCompletionMode(QCompleter::PopupCompletion);
        m_completer.setCaseSensitivity(Qt::CaseSensitive);
        m_completer.setMaxVisibleItems(10);

        QObject::connect(&m_completer, QOverload<const QString &>::of(&QCompleter::activated),
                         this, QOverload<const QString &>::of(&DFMCompleterView::completerActivated));

        QObject::connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
                         this, QOverload<const QModelIndex &>::of(&DFMCompleterView::completerActivated));

        QObject::connect(&m_completer, QOverload<const QString &>::of(&QCompleter::highlighted),
                         this, QOverload<const QString &>::of(&DFMCompleterView::completerHighlighted));

        QObject::connect(&m_completer, QOverload<const QModelIndex &>::of(&QCompleter::highlighted),
                         this, QOverload<const QModelIndex &>::of(&DFMCompleterView::completerHighlighted));

        setItemDelegate(&m_delegate);
    }

    QCompleter *completer()
    {
        return &m_completer;
    }

    QStringListModel* model()
    {
        return &m_model;
    }

    DFMCompleterViewDelegate* itemDelegate()
    {
        return &m_delegate;
    }


Q_SIGNALS:
    void completerActivated(const QString &text);
    void completerActivated(const QModelIndex &index);
    void completerHighlighted(const QString &text);
    void completerHighlighted(const QModelIndex &index);

private:
    QCompleter m_completer;
    QStringListModel m_model;
    DFMCompleterViewDelegate m_delegate;
};

#endif //DFMCOMPLETERVIEW_H

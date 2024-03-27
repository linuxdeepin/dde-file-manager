// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONITEMDELEGATE_H
#define ICONITEMDELEGATE_H

#include "baseitemdelegate.h"

#include <QStyledItemDelegate>
#include <QTextLayout>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class IconItemEditor;
class IconItemDelegatePrivate;
class IconItemDelegate : public BaseItemDelegate
{
    Q_OBJECT
    friend class ExpandedItem;

public:
    explicit IconItemDelegate(FileViewHelper *parent);
    ~IconItemDelegate() override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    bool eventFilter(QObject *object, QEvent *event) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const override;
    void updateItemSizeHint() override;

    int iconSizeLevel() const override;
    int minimumIconSizeLevel() const override;
    int maximumIconSizeLevel() const override;
    int increaseIcon() override;
    int decreaseIcon() override;
    int setIconSizeByIconSizeLevel(int level) override;

    void hideNotEditingIndexWidget() override;

    QRectF itemIconRect(const QRectF &itemRect) const override;
    QList<QRect> itemGeomertys(const QStyleOptionViewItem &opt, const QModelIndex &index) const override;

    bool itemExpanded() override;
    QRect expandItemRect() override;
    QModelIndex expandedIndex() override;

    QString displayFileName(const QModelIndex &index) const;
    QList<QRectF> calFileNameRect(const QString &name, const QRectF &rect, Qt::TextElideMode elideMode) const;

private slots:
    void editorFinished();

private:
    void onTriggerEdit(const QModelIndex &index);

    QPainterPath paintItemBackgroundAndGeomerty(QPainter *painter, const QStyleOptionViewItem &option,
                                                const QModelIndex &index, int backgroundMargin) const;
    QRectF paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const;

    void paintItemFileName(QPainter *painter, QRectF iconRect, QPainterPath path, int backgroundMargin, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;

    QSize iconSizeByIconSizeLevel() const;

    Q_DECLARE_PRIVATE_D(d, IconItemDelegate)
};

}

#endif   // DFMICONITEMDELEGATE_H

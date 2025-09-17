// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEITEMDELEGATE_H
#define BASEITEMDELEGATE_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QStyledItemDelegate>
#include <QTextOption>

QT_BEGIN_NAMESPACE
class QTextLayout;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class AbstractItemPaintProxy;
class FileViewHelper;
class FileView;
class BaseItemDelegatePrivate;
class BaseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BaseItemDelegate(FileViewHelper *parent);

    virtual ~BaseItemDelegate() override;

    /*!
     * \brief paintGeomertys paint geomertys for all items
     * \param option
     * \param index
     * \param sizeHintMode
     * \return
     */
    virtual QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode = false) const = 0;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;

    /**
     * @brief iconSizeLevel return icon size level
     * @return Return current icon level if level is vaild, otherwise return -1
     */
    virtual int iconSizeLevel() const;

    /**
     * @brief minimumIconSizeLevel return minimum icon size level
     * @return Return minimum icon level if level is vaild, otherwise return -1
     */
    virtual int minimumIconSizeLevel() const;

    /**
     * @brief maximumIconSizeLevel return maximum icon size level
     * @return Return maximum icon level if level is vaild, otherwise return -1
     */
    virtual int maximumIconSizeLevel() const;

    /**
     * @brief increaseIcon increase icon size level
     * @return  Return current icon level if icon can increase; otherwise return -1.
     */
    virtual int increaseIcon();

    /**
     * @brief increaseIcon decrease icon size level
     * @return  Return current icon level if icon can decrease; otherwise return -1.
     */
    virtual int decreaseIcon();

    /*!
     * \brief setIconSizeByIconSizeLevel set icon size level
     * \param level
     * \return  Return current icon level if level is vaild, otherwise return -1
     */
    virtual int setIconSizeByIconSizeLevel(int level);

    /**
     * @brief hasWidgetIndexs
     * @return
     */
    virtual QModelIndexList hasWidgetIndexs() const;

    /**
     * @brief hideAllIIndexWidget
     */
    virtual void hideAllIIndexWidget();
    /**
     * @brief hideNotEditingIndexWidget
     */
    virtual void hideNotEditingIndexWidget();

    /**
     * @brief commitDataAndCloseActiveEditor
     */
    virtual void commitDataAndCloseActiveEditor();

    /**
     * @brief updateItemSizeHint when icon size change or font change on pain event call
     */
    virtual void updateItemSizeHint() = 0;

    virtual QRectF itemIconRect(const QRectF &itemRect) const;

    virtual QList<QRect> itemGeomertys(const QStyleOptionViewItem &opt, const QModelIndex &index) const;

    virtual QRect getRectOfItem(RectOfItemType type, const QModelIndex &index) const;

    virtual bool itemExpanded();
    virtual QRect expandItemRect();
    virtual QModelIndex expandedIndex();
    virtual QWidget *expandedItem();

    QModelIndex editingIndex() const;

    QWidget *editingIndexWidget() const;

    FileViewHelper *parent() const;

    void paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const;
    QSize getIndexIconSize(const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const;

    void setPaintProxy(AbstractItemPaintProxy *proxy);

    // Group rendering virtual interfaces
    virtual bool isGroupHeaderItem(const QModelIndex &index) const;
    virtual QSize getGroupHeaderSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void paintGroupHeader(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QRect getExpandButtonRect(const QStyleOptionViewItem &option) const;

signals:
    void groupExpansionToggled(const QString &groupKey);
    void groupHeaderClicked(const QString &groupKey);

protected:
    explicit BaseItemDelegate(BaseItemDelegatePrivate &dd, FileViewHelper *parent);

    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    // Pure virtual method for group header height - must be implemented by subclasses
    virtual int getGroupHeaderHeight(const QStyleOptionViewItem &option) const = 0;

    // Group rendering helper methods
    void paintGroupBackground(QPainter *painter, const QStyleOptionViewItem &option) const;
    void paintExpandButton(QPainter *painter, const QRect &buttonRect, bool isExpanded) const;
    void paintGroupText(QPainter *painter, const QRect &textRect, const QString &text, const QStyleOptionViewItem &option) const;

    // Group layout calculation methods

    QRect getGroupTextRect(const QStyleOptionViewItem &option) const;

    QList<QRectF> getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const;
    void paintEmblems(QPainter *painter, const QRectF &iconRect, const QModelIndex &index) const;
    bool isThumnailIconIndex(const QModelIndex &index) const;

    QScopedPointer<BaseItemDelegatePrivate> d;
    Q_DECLARE_PRIVATE_D(d, BaseItemDelegate)

private:
    // Group rendering constants
    QSize m_expandButtonSize = QSize(16, 16);
    int m_leftMargin = 12;
    int m_rightMargin = 12;
};

}

#endif   // BASEITEMDELEGATE_H

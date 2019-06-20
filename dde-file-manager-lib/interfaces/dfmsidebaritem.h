/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DFMSIDEBARITEM_H
#define DFMSIDEBARITEM_H

#include <QWidget>
#include <QIcon>

#include <dfmglobal.h>

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMSideBarItemPrivate;
class DFMSideBarItem : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool canDeleteViaDrag READ canDeleteViaDrag WRITE setCanDeleteViaDrag)
    Q_PROPERTY(bool reorderable READ reorderable WRITE setReorderable)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    Q_PROPERTY(bool autoOpenUrlOnClick
               READ autoOpenUrlOnClick WRITE setAutoOpenUrlOnClick)
    Q_PROPERTY(DUrl url READ url CONSTANT)
    Q_PROPERTY(QString groupName READ groupName CONSTANT)
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit DFMSideBarItem(const DUrl &url = DUrl(), QWidget *parent = nullptr);
    ~DFMSideBarItem() override;

    const DUrl url() const;

    bool reorderable() const;
    bool canDeleteViaDrag() const;
    bool readOnly() const;
    bool checked() const;
    bool autoOpenUrlOnClick() const;
    bool itemVisible() const;

    QString groupName() const;
    QString text() const;

    void showRenameEditor();

    virtual int sortingPriority() const;

    void setContentWidget(QWidget *widget);
    QWidget *contentWidget() const;

    void setIconFromThemeConfig(const QString &group, const QString &key = "icon");

    static const int minimumWidth = 100;
    static const int maximumWidth = 200;

public Q_SLOTS:
    void setReorderable(bool reorderable);
    void setCanDeleteViaDrag(bool canDeleteViaDrag);
    void setReadOnly(bool readOnly);
    void setChecked(bool checked);
    void setText(QString text);
    void setAutoOpenUrlOnClick(bool autoCd);
    void setVisible(bool visible) override;

    void onEditingFinished();
    void playAnimation();

Q_SIGNALS:
    void clicked();
    void urlChanged(DUrl url);
    void renameFinished(QString name);
    void itemDragReleased(QPoint dropPos, Qt::DropAction action);
    void reorder(DFMSideBarItem *ori, DFMSideBarItem *dst, bool insertBefore);

protected:
    void setUrl(DUrl url);
    void setGroupName(QString groupName);

    virtual QMenu *createStandardContextMenu() const;
    virtual Qt::DropAction canDropMimeData(const QMimeData *data, Qt::DropActions actions) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action) const;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool event(QEvent *event) override;

    //friend DFMSideBarItem *DFMSideBarItemGroup::takeItem(int index);
    friend class DFMSideBarItemGroup;
    friend class DFMSideBarPrivate;

private:
    QScopedPointer<DFMSideBarItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMSideBarItem)
};

DFM_END_NAMESPACE

#endif // DFMSIDEBARITEM_H

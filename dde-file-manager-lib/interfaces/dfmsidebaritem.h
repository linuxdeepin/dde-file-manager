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
    ~DFMSideBarItem();

    const DUrl url() const;

    bool reorderable() const;
    bool readOnly() const;
    bool checked() const;
    bool autoOpenUrlOnClick() const;

    QString groupName() const;
    QString text() const;

    void showRenameEditor();

    void setContentWidget(QWidget *widget);
    QWidget *contentWidget() const;

    void setIconFromThemeConfig(const QString &group, const QString &key = "icon");

public Q_SLOTS:
    void setReorderable(bool reorderable);
    void setReadOnly(bool readOnly);
    void setChecked(bool checked);
    void setText(QString text);
    void setAutoOpenUrlOnClick(bool autoCd);

    void hideRenameEditor();
    void playAnimation();

Q_SIGNALS:
    void clicked();
    void renameFinished(QString name);
    void reorder(DFMSideBarItem *ori, DFMSideBarItem *dst, bool insertBefore);

protected:
    void setUrl(DUrl url);
    void setGroupName(QString groupName);

    virtual QMenu *createStandardContextMenu() const;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action) const;

    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    //friend DFMSideBarItem *DFMSideBarItemGroup::takeItem(int index);
    friend class DFMSideBarItemGroup;

private:
    QScopedPointer<DFMSideBarItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMSideBarItem)
};

DFM_END_NAMESPACE

#endif // DFMSIDEBARITEM_H

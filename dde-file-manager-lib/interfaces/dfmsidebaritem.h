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

    Q_PROPERTY(bool hasDrag READ hasDrag WRITE setHasDrag)
    Q_PROPERTY(bool readOnly READ readOnly WRITE setReadOnly)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit DFMSideBarItem(const DUrl &url = DUrl(), QWidget *parent = nullptr);
    ~DFMSideBarItem();

    const DUrl url() const;

    bool hasDrag() const;
    bool readOnly() const;
    bool checked() const;

    QString text() const;

    void setContentWidget(QWidget *widget);
    QWidget *contentWidget() const;

    void setIconFromThemeConfig(const QString &group, const QString &key = "icon");

public Q_SLOTS:
    void setHasDrag(bool hasDrag);
    void setReadOnly(bool readOnly);
    void setChecked(bool checked);

    void setText(QString text);

    void playAnimation();

Q_SIGNALS:
    void clicked();

protected:
    virtual QMenu *createStandardContextMenu() const;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action) const;

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QScopedPointer<DFMSideBarItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMSideBarItem)
};

DFM_END_NAMESPACE

#endif // DFMSIDEBARITEM_H

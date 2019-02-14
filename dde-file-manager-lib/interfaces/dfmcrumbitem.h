/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#ifndef DFMCRUMBITEM_H
#define DFMCRUMBITEM_H

#include <QPushButton>

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class CrumbData;
class DFMCrumbItemPrivate;
class DFMCrumbItem : public QPushButton
{
    Q_OBJECT
public:
    explicit DFMCrumbItem(const CrumbData &data, QWidget *parent = nullptr);
    ~DFMCrumbItem();

    DUrl url() const;
    void setText(const QString &text);
    void setUrl(const DUrl &url);
    void setIconFromThemeConfig(const QString &group, const QString &key = "icon");

protected:
    virtual Qt::DropAction canDropMimeData(const QMimeData *data, Qt::DropActions actions) const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action) const;

    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QScopedPointer<DFMCrumbItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMCrumbItem)
};

DFM_END_NAMESPACE

#endif // DFMCRUMBITEM_H

/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef DCRUMBBUTTON_H
#define DCRUMBBUTTON_H

#include <QPushButton>
#include <QListWidgetItem>
#include "durl.h"

class DCrumbButton : public QPushButton
{
    Q_OBJECT
public:
    DCrumbButton(int index, const QString &text, QWidget *parent = 0);
    DCrumbButton(int index, const QIcon& icon, const QString &text, QWidget *parent = 0);
    int getIndex();
    QString getName();
    void setItem(QListWidgetItem* item);
    void setListWidget(QListWidget* widget);
    QListWidgetItem* getItem();

    DUrl url() const;
    void setUrl(const DUrl& url);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    int m_index;
    QString m_name;
    QString m_path;
    DUrl m_url;
    QListWidgetItem* m_item;
    QListWidget* m_listWidget;
    QPoint oldGlobalPos;
};

class DCrumbIconButton : public DCrumbButton
{
    Q_OBJECT
public:
    DCrumbIconButton(int index, const QIcon& normalIcon, const QIcon& hoverIcon, const QIcon& checkedIcon, const QString &text, QWidget *parent = 0);
private:
    QIcon m_normalIcon;
    QIcon m_hoverIcon;
    QIcon m_checkedIcon;
protected:
    void checkStateSet();
    void nextCheckState();
};

#endif // DCRUMBBUTTON_H

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

#include "dcrumbbutton.h"
#include "dthememanager.h"
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

DCrumbButton::DCrumbButton(int index, const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    m_index = index;
    m_name = text;
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("DCrumbButton");
}

DCrumbButton::DCrumbButton(int index, const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton(icon, "", parent)
{
    m_index = index;
    m_name = text;
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setObjectName("DCrumbButton");
}

int DCrumbButton::getIndex()
{
    return m_index;
}

QString DCrumbButton::getName()
{
    return m_name;
}

void DCrumbButton::setItem(QListWidgetItem *item)
{
    m_item = item;
}

void DCrumbButton::setListWidget(QListWidget *widget)
{
    m_listWidget = widget;
}

QListWidgetItem *DCrumbButton::getItem()
{
    return m_item;
}

DUrl DCrumbButton::url() const
{
    return m_url;
}

void DCrumbButton::setUrl(const DUrl &url)
{
    m_url = url;
}

void DCrumbButton::paintEvent(QPaintEvent *e)
{

    QPushButton *button = qobject_cast<QPushButton*>(m_listWidget->itemWidget(m_listWidget->item(m_index + 1)));

    if (!isChecked() && (!button || (button &&  !button->isChecked()))) {
        QPainter painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);

        double w = width();
        double h = height();
        QPolygon polygon;
        polygon << QPoint(w, 2);
        polygon << QPoint(w, h - 2);
        QPen pen(QColor(0, 0, 0, 24));
        pen.setWidthF(1);
        painter.setPen(pen);
        painter.drawPolygon(polygon);
        painter.end();
    }


    QPushButton::paintEvent(e);
}

void DCrumbButton::mouseMoveEvent(QMouseEvent *e)
{
    return QWidget::mouseMoveEvent(e);
}

void DCrumbButton::mousePressEvent(QMouseEvent *e)
{
    oldGlobalPos = e->globalPos();
    if(e->button() == Qt::LeftButton)
        return QPushButton::mousePressEvent(e);

}

void DCrumbButton::mouseReleaseEvent(QMouseEvent *e)
{
    const QPoint &difference = e->globalPos() - oldGlobalPos;

    if (qAbs(difference.x()) < 10 && qAbs(difference.y()) < 10 && e->button() == Qt::LeftButton) {
        QPushButton::mouseReleaseEvent(e);
    }
}

DCrumbIconButton::DCrumbIconButton(int index, const QIcon &normalIcon, const QIcon &hoverIcon, const QIcon &checkedIcon, const QString &text, QWidget *parent)
    : DCrumbButton(index, normalIcon, text, parent)
{
    m_normalIcon = normalIcon;
    m_hoverIcon = hoverIcon;
    m_checkedIcon = checkedIcon;
    setObjectName("DCrumbIconButton");
}

DCrumbIconButton::DCrumbIconButton(int index, const QString &id, const QString &text, QWidget *parent)
    : DCrumbButton(index, QIcon(), text, parent),
      m_id(id)
{
    m_normalIcon = getIcon("icon", ThemeConfig::Normal);
    m_hoverIcon = getIcon("icon", ThemeConfig::Hover);
    m_checkedIcon = getIcon("icon", ThemeConfig::Checked);
    setIcon(m_normalIcon);
    setObjectName("DCrumbIconButton");
    connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, this, &DCrumbIconButton::updateIcons);
}

void DCrumbIconButton::updateIcons(QWidget *widget, QString theme)
{
    Q_UNUSED(theme)
    if (widget == this){
        m_normalIcon = getIcon("icon", ThemeConfig::Normal);
        m_hoverIcon = getIcon("icon", ThemeConfig::Hover);
        m_checkedIcon = getIcon("icon", ThemeConfig::Checked);
        checkStateSet();
        update();
    }
}

QIcon DCrumbIconButton::getIcon(const QString &key, ThemeConfig::State state) const
{
    QPixmap p;
    if (!m_id.isEmpty()){
//        qDebug() << m_id << ":" << ThemeConfig::instace()->string("CrumbIconButton." + m_id, key, state);
        p = ThemeConfig::instace()->pixmap("CrumbIconButton." + m_id, key, state);
    }

    return QIcon(p);
}

void DCrumbIconButton::checkStateSet()
{
    if(isChecked()){
        setIcon(m_checkedIcon);
    }
    else{
        setIcon(m_normalIcon);
    }
    qDebug() << "Check state changed: " << isChecked();
}

void DCrumbIconButton::nextCheckState()
{
    setChecked(!isChecked());
    if(isChecked()){
        setIcon(m_checkedIcon);
    }
    else{
        setIcon(m_normalIcon);
    }
    qDebug() << "Next check state changed: " << isChecked();
}

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

#ifndef DADVANCEDINFOWIDGET_H
#define DADVANCEDINFOWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QMap>
#include <QUrl>
#include <QPair>

typedef QPair<QString, QString> QStringPair;
class DAdvancedInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DAdvancedInfoWidget(QWidget *parent = nullptr, const QString &file = "");
    void initData();
    void initUI();

signals:

public slots:

private:
    QListWidget* m_listWidget;
    QList<QStringPair> m_InfoList;
    QUrl m_url;
};

#endif // DADVANCEDINFOWIDGET_H

/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef TAGWIDGET_H
#define TAGWIDGET_H

#include "dfmplugin_tag_global.h"

#include <dtkwidget_global.h>
#include <dcrumbedit.h>

#include <QFrame>
#include <QSharedPointer>

DWIDGET_BEGIN_NAMESPACE
class DCrumbEdit;
DWIDGET_END_NAMESPACE

namespace dfmplugin_tag {

class TagColorListWidget;
class TagWidgetPrivate;
class TagWidget : public QFrame
{
    Q_OBJECT
public:
    TagWidget(QUrl url, QWidget *parent = nullptr);
    ~TagWidget();

    void loadTags(const QUrl &url);
    QWidget *tagTitle();
    QWidget *tagLeftTitle();
    TagColorListWidget *tagActionWidget();
    DTK_WIDGET_NAMESPACE::DCrumbEdit *tagCrumbEdit();

    static bool shouldShow(const QUrl &url);

public slots:
    void onCrumbListChanged();
    void onCheckedColorChanged(const QColor &color);
    void onTagChanged(const QMap<QString, QStringList> &fileAndTags);
    void filterInput();

protected:
    void initConnection();
    void updateCrumbsColor(const QMap<QString, QColor> &tagsColor);

    QSharedPointer<TagWidgetPrivate> d;
};

}

#endif   // TAGWIDGET_H

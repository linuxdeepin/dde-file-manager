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
#ifndef TAGWIDGET_P_H
#define TAGWIDGET_P_H

#include "dfmplugin_tag_global.h"

#include <QUrl>
#include <QMap>

class QLabel;
class QVBoxLayout;

namespace dfmplugin_tag {

class TagWidget;
class TagCrumbEdit;
class TagColorListWidget;
class TagWidgetPrivate : public QObject
{
    Q_OBJECT
    friend class TagWidget;

public:
    explicit TagWidgetPrivate(TagWidget *qq, const QUrl &url);
    virtual ~TagWidgetPrivate();

private:
    void initializeUI();

    QUrl url;
    QLabel *tagLable { nullptr };
    QLabel *tagLeftLable { nullptr };
    QVBoxLayout *mainLayout { nullptr };
    TagCrumbEdit *crumbEdit { nullptr };
    TagColorListWidget *colorListWidget { nullptr };

    QMap<QString, QString> currentTagWithColorMap;

    TagWidget *q { nullptr };
};

}

#endif   // TAGWIDGET_P_H

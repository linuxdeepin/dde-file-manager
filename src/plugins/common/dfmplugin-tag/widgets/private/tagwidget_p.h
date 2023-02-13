// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

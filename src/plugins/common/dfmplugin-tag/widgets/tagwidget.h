// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void onTagChanged(const QVariantMap &fileAndTags);
    void filterInput();

protected:
    void initConnection();
    void updateCrumbsColor(const QMap<QString, QColor> &tagsColor);

    QSharedPointer<TagWidgetPrivate> d;
};

}

#endif   // TAGWIDGET_H

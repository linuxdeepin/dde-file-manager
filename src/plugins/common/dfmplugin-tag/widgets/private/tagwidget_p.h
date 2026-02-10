// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGWIDGET_P_H
#define TAGWIDGET_P_H

#include "dfmplugin_tag_global.h"
#include <dtkwidget_global.h>

#include <QUrl>
#include <QMap>
#include <QBoxLayout>
#include <DLabel>

class QVBoxLayout;
class QHBoxLayout;

DWIDGET_USE_NAMESPACE

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

private slots:
    void initUiForSizeMode();

private:
    void initializeUI();

    QUrl url;
    DLabel *tagLable { nullptr };
    DLabel *tagLeftLable { nullptr };
    QVBoxLayout *mainLayout { nullptr };
    TagCrumbEdit *crumbEdit { nullptr };
    QBoxLayout *tagColorListLayout { nullptr };
    TagColorListWidget *colorListWidget { nullptr };
    bool horizontalLayout { false };

    QMap<QString, QString> currentTagWithColorMap;

    TagWidget *q { nullptr };
};

}

#endif   // TAGWIDGET_P_H

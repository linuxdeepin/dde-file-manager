// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMTAGWIDGET_H
#define DFMTAGWIDGET_H
#include "dfmglobal.h"
#include "dtagactionwidget.h"

#include <QFrame>
#include <QObject>
#include <QSharedDataPointer>
#include <dcrumbedit.h>
DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMTagWidgetPrivate;
class DFMTagWidget : public QFrame
{
    Q_OBJECT
public:
    DFMTagWidget(DUrl url, QWidget *parent = nullptr);
    ~DFMTagWidget();

    void loadTags(const DUrl &durl);
    QWidget *tagTitle();
    QWidget *tagLeftTitle();
    DTagActionWidget *tagActionWidget();
    DCrumbEdit *tagCrumbEdit();

    static bool shouldShow(const DUrl &url);

protected:
    void initUi();
    void initConnection();
    void updateCrumbsColor(const QMap<QString, QColor> &tagsColor);
    QMap<QString, QColor> tagsColor(const QStringList &tagList);
private:
    QScopedPointer<DFMTagWidgetPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMTagWidget)
};

DFM_END_NAMESPACE

#endif // DFMTAGWIDGET_H

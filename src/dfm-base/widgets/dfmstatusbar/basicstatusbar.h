// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICSTATUSBAR_H
#define BASICSTATUSBAR_H

#include <dfm-base/dfm_base_global.h>

#include <QFrame>

namespace dfmbase {

class FileInfo;
class BasicStatusBarPrivate;
class BasicStatusBar : public QFrame
{
    Q_OBJECT
public:
    explicit BasicStatusBar(QWidget *parent = nullptr);

    virtual QSize sizeHint() const override;
    virtual void clearLayoutAndAnchors();

    void itemSelected(const QList<QUrl> &urls);
    void itemCounted(const int count);

    void updateStatusMessage();

protected:
    void insertWidget(const int index, QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

    void setTipText(const QString &tip);

private:
    void initUI();
    QSharedPointer<BasicStatusBarPrivate> d;
};

}

#endif   // BASICSTATUSBAR_H

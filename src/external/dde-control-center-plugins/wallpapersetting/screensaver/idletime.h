// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IDLETIME_H
#define IDLETIME_H

#include <widgets/comboxwidget.h>

#include <DBackgroundGroup>

class QCheckBox;
namespace dfm_wallpapersetting {

class IdleTime : public QWidget
{
    Q_OBJECT
public:
    explicit IdleTime(QWidget *parent = nullptr);
    ~IdleTime();
    void reset();
    static QVector<int> availableTime();
    static QString timeFormat(int second);
    static QStringList translateText(const QVector<int> &list);
signals:
    int getCurrentIdle();
    void setCurrentIdle(int sec);
    bool getIsLock();
    void setIsLock(bool l);
protected slots:
    void setIdle(int idx);
private:
    DTK_WIDGET_NAMESPACE::DBackgroundGroup *bkg = nullptr;
    QCheckBox *pwdCheck = nullptr;
    dcc::widgets::ComboxWidget *combox = nullptr;
    QVector<int> valueList;
};

}

#endif // IDLETIME_H

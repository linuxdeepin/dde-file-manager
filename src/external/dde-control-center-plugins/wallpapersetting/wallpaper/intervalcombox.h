// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTERVALCOMBOX_H
#define INTERVALCOMBOX_H

#include <widgets/comboxwidget.h>

namespace dfm_wallpapersetting {

class IntervalCombox : public dcc::widgets::ComboxWidget
{
    Q_OBJECT
public:
    explicit IntervalCombox(QFrame *parent = nullptr);
    ~IntervalCombox();
    void reset(QString sc);
    static QStringList availableInterval();
    static QString timeFormat(int second);
    static QStringList translateText(const QStringList &list);
public slots:
    void setInterval(int index);
signals:
    QString getCurrentSlideShow(const QString &sc) const;
    void setCurrentSlideshow(const QString &sc, const QString &time);
private:
    QStringList valueList;
    QString screen;
};

}
#endif // INTERVALCOMBOX_H

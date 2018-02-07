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

#ifndef MUSICMESSAGEVIEW_H
#define MUSICMESSAGEVIEW_H

#include <QFrame>

class QLabel;
class MusicMessageView : public QFrame
{
    Q_OBJECT
public:
    explicit MusicMessageView(const QString& uri = "", QWidget *parent = 0);
    void initUI();
    void updateElidedText();

signals:

public slots:

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QString m_uri;
    QLabel* m_titleLabel;
    QLabel* m_artistLabel;
    QLabel* m_albumLabel;
    QLabel* m_imgLabel;

    QString m_title;
    QString m_artist;
    QString m_album;
    int m_margins;
};

#endif // MUSICMESSAGEVIEW_H

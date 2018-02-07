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

#ifndef DCHECKABLEBUTTON_H
#define DCHECKABLEBUTTON_H

#include <QPushButton>

class QLabel;

class DCheckableButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DCheckableButton(const QString& normal,
                              const QString& hover,
                              const QString& text,
                              QWidget *parent = 0);
    ~DCheckableButton();
    void initUI();
    void setUrl(const QString &url);
    QString getUrl();
protected:
    void nextCheckState();
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

signals:

public slots:

private:
    QString m_icon;
    QString m_text;
    QLabel* m_iconLabel;
    QLabel* m_textLabel;
    QString m_normal;
    QString m_hover;
    QString m_url;
};

#endif // DCHECKABLEBUTTON_H

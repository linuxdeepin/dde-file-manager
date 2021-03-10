/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QLabel>

class ImageView : public QLabel
{
    Q_OBJECT
public:
    explicit ImageView(const QString &fileName, const QByteArray &format, QWidget *parent = nullptr);

    void setFile(const QString &fileName, const QByteArray &format);
    QSize sourceSize() const;

private:
    QSize m_sourceSize;
    QMovie *movie = nullptr;
};

#endif // IMAGEVIEW_H

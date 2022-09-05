// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

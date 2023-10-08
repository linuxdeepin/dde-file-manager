// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

namespace dfm_wallpapersetting {

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    void updateSize();
    void setImage(const QString &img);
    void setBackground(const QColor &color);
    void setBoder(const QColor &color);
protected:
    QPixmap scaledPixmap(const QPixmap &pixmap) const;
protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
public slots:
protected:
    QString imgPath;
    QPixmap pixmap;
    QColor bkgColor;
    QColor bdColor;
};

}
#endif // PREVIEWWIDGET_H

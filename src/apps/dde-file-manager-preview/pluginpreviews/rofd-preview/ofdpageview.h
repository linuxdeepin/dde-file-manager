// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OFDPAGEVIEW_H
#define OFDPAGEVIEW_H

#include <QPixmap>
#include <QWidget>

namespace plugin_filepreview {

/**
 * @brief Displays one rendered OFD page, scaled to fit while keeping the
 * page aspect ratio, centred on a white canvas.
 */
class OfdPageView : public QWidget
{
    Q_OBJECT
public:
    explicit OfdPageView(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void setPage(const QImage &image);
    void setBusy(bool busy);

Q_SIGNALS:
    void viewportResized();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap pagePixmap;
    bool busy = false;
};
}

#endif   // OFDPAGEVIEW_H

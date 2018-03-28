/*
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

#include "imageview.h"

#include <QUrl>
#include <QImageReader>
#include <QApplication>
#include <QDesktopWidget>
#include <QtMath>
#include <QPainter>
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QDebug>

#define MIN_SIZE QSize(400, 300)

ImageView::ImageView(const QString &fileName, QWidget *parent)
    : QLabel(parent)
{
    setFile(fileName);
    setMinimumSize(MIN_SIZE);
    setAlignment(Qt::AlignCenter);
}

void ImageView::setFile(const QString &fileName)
{
    QImageReader reader(fileName);

    m_sourceSize = reader.size();

    const QSize &dsize = qApp->desktop()->size();

    setPixmap(QPixmap::fromImageReader(&reader).scaled(QSize(qMin((int)(dsize.width() * 0.7), m_sourceSize.width()),
                                                             qMin((int)(dsize.height() * 0.8), m_sourceSize.height())),
                                                       Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QSize ImageView::sourceSize() const
{
    return m_sourceSize;
}

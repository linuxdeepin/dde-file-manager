#ifndef DUTILITY_H
#define DUTILITY_H

#include "dutil_global.h"

#include <QImage>
#include <QPixmap>

DUTIL_BEGIN_NAMESPACE
QImage dropShadow(const QPixmap &px, qreal radius, const QColor &color = Qt::black, QSize size = QSize());
DUTIL_END_NAMESPACE

#endif // DUTILITY_H

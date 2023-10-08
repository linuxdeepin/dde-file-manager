// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/commondefine.h"
#include "colordialog.h"

#include <DStyle>
#include <DFontSizeManager>
#include <DHorizontalLine>
#include <DSuggestButton>

#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QUuid>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

#define OUTSIDE_MARGIN 5
#define INSIDE_MARGIN (10 - OUTSIDE_MARGIN)
#define PICK_ID "PICKTOOL_ID"
#define PICKER_SERVICE "com.deepin.Picker"
#define PICKER_PATH "/com/deepin/Picker"

ColorLabel::ColorLabel(QWidget *parent) : QLabel(parent)
{

}

void ColorLabel::setHue(qreal h)
{
    hue = h;
    updateImage();
    update();
}

void ColorLabel::updateImage()
{
    QSize orgSize = insideGeometry().size() * devicePixelRatioF();

    QImage img(orgSize, QImage::Format_ARGB32);
    img.fill(Qt::transparent);

    // range
    const qreal w = orgSize.width() - 1;
    const qreal h = orgSize.height() - 1;
    for (qreal s = 0; s <= w; s++) {
        for (qreal v = 0; v <= h; v++) {
            QColor penColor = QColor::fromHsvF(hue, s / w, v / h);
            if (penColor.isValid())
                img.setPixelColor(int(s), h - int(v), penColor);
        }
    }

    img.setDevicePixelRatio(devicePixelRatioF());
    image = img;
}

void ColorLabel::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);
    auto ava = insideGeometry();

    QPainter pa(this);
    pa.setRenderHint(QPainter::Antialiasing);
    pa.setPen(Qt::NoPen);
    pa.setBrush(Qt::NoBrush);
    {
        pa.save();
        QPainterPath roundPath;
        roundPath.addRoundedRect(ava, 4, 4);
        pa.setClipPath(roundPath);
        pa.drawImage(ava, image);
        pa.restore();
    }

    // draw border
    pa.setBrush(QColor(0, 0, 0, 0.1 * 255));
    {
        QPainterPath out;
        out.addRoundedRect(ava, 4, 4);
        QPainterPath in;
        in.addRoundedRect(ava.marginsRemoved(QMargins(1, 1, 1, 1)), 4, 4);
        pa.drawPath(out - in);
    }

    if (!ava.contains(currentPos))
        return;

    // draw cursor
    pa.setBrush(Qt::NoBrush);
    {
        QPen pen(QColor(0, 0, 0, 0.2 * 255), 1);
        pa.setPen(pen);
        pa.drawEllipse(currentPos, 5, 5);
    }

    {
        QPen pen(Qt::white, 2);
        pa.setPen(pen);
        pa.drawEllipse(currentPos, 4, 4);
    }
}

void ColorLabel::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        pressed = true;

    QLabel::mousePressEvent(e);
}

void ColorLabel::mouseMoveEvent(QMouseEvent *e)
{
    QLabel::mousePressEvent(e);
    e->accept();

    auto tipPoint = e->pos();
    auto ava = insideGeometry();

    // normalize
    if (tipPoint.x() < ava.left())
        tipPoint.setX(ava.left());
    if (tipPoint.x() > ava.right())
        tipPoint.setX(ava.right());
    if (tipPoint.y() < ava.top())
        tipPoint.setY(ava.top());
    if (tipPoint.y() > ava.bottom())
        tipPoint.setY(ava.bottom());

    if (pressed && (currentPos != tipPoint)) {
        QColor c;
        if (pickColor(tipPoint, c)) {
            currentPos = tipPoint;
            emit colorPicked(c);
            update();
        }
    }
}

void ColorLabel::mouseReleaseEvent(QMouseEvent *e)
{
    QLabel::mouseReleaseEvent(e);
    if (e->button() != Qt::LeftButton)
        return;

    auto tipPoint = e->pos();
    if (pressed && insideGeometry().contains(tipPoint)) {
        pressed = false;
        QColor c;
        if (pickColor(tipPoint, c)) {
            currentPos = tipPoint;
            emit colorPicked(c);
            update();
        }
    }
}

bool ColorLabel::pickColor(QPoint pos, QColor &c) const
{
    auto ava = insideGeometry();
    if (!ava.contains(pos))
        return false;

    pos = (pos - ava.topLeft());
    c = QColor::fromHsvF(hue, pos.x() / qreal(ava.width() - 1), 1 - pos.y() / qreal(ava.height() - 1));
    return true;
}

void ColorLabel::setCurrent(const QColor &c)
{
    qreal h, s, v;
    c.getHsvF(&h, &s, &v);

    setHue(h);

    auto ava = insideGeometry();
    currentPos = (QPoint(s * (ava.width() - 1),  (1 - v) * (ava.height() - 1))) + ava.topLeft();
}

QColor ColorLabel::current() const
{
    QColor c;
    pickColor(currentPos, c);
    return c;
}

QRect ColorLabel::insideGeometry() const
{
    return QRect(QPoint(INSIDE_MARGIN, INSIDE_MARGIN), size() - QSize(INSIDE_MARGIN * 2, INSIDE_MARGIN * 2));
}

ColorDialog::ColorDialog(QWidget *parent) : DAbstractDialog(true, parent)
{
    setFixedSize(274, 330);
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(OUTSIDE_MARGIN, OUTSIDE_MARGIN, OUTSIDE_MARGIN, OUTSIDE_MARGIN);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    label = new ColorLabel(this);
    label->setFixedSize(254 + 2 * INSIDE_MARGIN , 170 + 2 * INSIDE_MARGIN);
    label->setHue(0);
    mainLayout->addWidget(label);

    mainLayout->addSpacing(6);

    slider = new ColorSlider(this);
    slider->setFixedSize(226, 20);
    slider->updateImage();
    {
        QHBoxLayout *lay = new QHBoxLayout;
        lay->setSpacing(0);
        lay->setContentsMargins(INSIDE_MARGIN, 0, INSIDE_MARGIN, 0);
        pickTool = new DIconButton(this);
        pickTool->setFixedSize(20, 20);
        pickTool->setIconSize(QSize(20, 20));
        pickTool->setIcon(QIcon::fromTheme("dfm-color-picker"));
        pickTool->setProperty(PICK_ID, QUuid::createUuid().toString());
        pickTool->setFlat(true);

        lay->addWidget(pickTool);
        lay->addSpacing(8);
        lay->addWidget(slider);

        mainLayout->addLayout(lay);
    }

    mainLayout->addSpacing(7);

    editor = new ColorEditor(this);
    {
        QHBoxLayout *lay = new QHBoxLayout;
        editor->setFixedSize(254, 43);
        lay->setSpacing(0);
        lay->setContentsMargins(INSIDE_MARGIN, 0, INSIDE_MARGIN, 0);
        lay->addWidget(editor);

        mainLayout->addLayout(lay);
    }

    mainLayout->addSpacing(9);
    {
        auto line = new DHorizontalLine(this);
        line->setFixedHeight(2);
        mainLayout->addWidget(line);
    }

    mainLayout->addSpacing(10);
    {
        auto cancel = new QPushButton(tr("Cancel", "button"), this);
        cancel->setFixedSize(119, 36);

        auto confirm = new DSuggestButton(tr("Confirm", "button"), this);
        confirm->setFixedSize(119, 36);
        confirm->setDefault(true);

        QHBoxLayout *lay = new QHBoxLayout;
        lay->setSpacing(0);
        lay->setContentsMargins(INSIDE_MARGIN, 0, INSIDE_MARGIN, 0);
        lay->addWidget(cancel);
        lay->addSpacing(5);
        auto line = new DVerticalLine(this);
        line->setFixedSize(3, 28);
        lay->addWidget(line);
        lay->addSpacing(5);
        lay->addWidget(confirm);

        mainLayout->addLayout(lay);

        connect(cancel, &DIconButton::clicked, this, &ColorDialog::reject);
        connect(confirm, &DIconButton::clicked, this, &ColorDialog::accept);
    }

    mainLayout->addSpacerItem(new QSpacerItem(1, 11, QSizePolicy::Expanding, QSizePolicy::Expanding));

    // sync color
    connect(label, &ColorLabel::colorPicked, this, &ColorDialog::onColorPicked);
    connect(slider, &ColorSlider::valueChanged, this, &ColorDialog::onSliderChanged);
    connect(editor, &ColorEditor::colorEdited, this, &ColorDialog::onColorEdited);

    QDBusConnection::sessionBus().connect(PICKER_SERVICE, PICKER_PATH, PICKER_SERVICE,
                                          "colorPicked", this, SLOT(onPicked(QString,QString)));
    connect(pickTool, &DIconButton::clicked, this, &ColorDialog::startPick);
}

void ColorDialog::setColor(const QColor &c)
{
    label->setCurrent(c);
    slider->setValue(c.hsvHueF());
    editor->setCurrent(c);
}

QColor ColorDialog::color() const
{
    return editor->getCurrent();
}

void ColorDialog::onSliderChanged(qreal h)
{
    label->setHue(h);
    editor->setCurrent(label->current());
}

void ColorDialog::onColorPicked(const QColor &c)
{
    editor->setCurrent(c);
}

void ColorDialog::onColorEdited(const QColor &c)
{
    label->setCurrent(c);
    slider->setValue(c.hsvHueF());
}

void ColorDialog::onPicked(const QString &uuid, const QString &colorName)
{
    if (uuid != pickTool->property(PICK_ID).toString())
        return;

    qDebug() << "picked color:" << colorName;
    QColor color(colorName);
    setColor(color);
}

void ColorDialog::startPick()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(PICKER_SERVICE, PICKER_PATH,
                                   PICKER_SERVICE, "StartPick");

    msg.setArguments({QVariant::fromValue(pickTool->property(PICK_ID).toString())});
    QDBusConnection::sessionBus().asyncCall(msg, 5);
    qDebug() << "picker serivce called." << msg.service() << msg.arguments();
}

void ColorDialog::mousePressEvent(QMouseEvent *event)
{
    DAbstractDialog::mousePressEvent(event);

    // pressed on empty area to take focus from editor
    setFocus();
}

void ColorDialog::keyPressEvent(QKeyEvent *event)
{
    // line editor will ingore enter event. then this dialog receive it and close.
    if (!hasFocus()) {
        event->ignore();
        return;
    }

    DAbstractDialog::keyPressEvent(event);
}

ColorSlider::ColorSlider(QWidget *parent) : QWidget(parent)
{
}

bool ColorSlider::pickColor(int x, qreal &h) const
{
    if (x < 0 || x >= width())
        return false;

    h = x / qreal(width() - 1);
    return true;
}

void ColorSlider::updateImage()
{
    QSize imageSize = QSize(width(), 10) * devicePixelRatioF();
    QImage img(imageSize, QImage::Format_ARGB32);
    QPainter pa(&img);
    pa.setPen(Qt::NoPen);
    pa.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    const qreal w = img.width() - 1;
    const int hgt = img.height();
    for (qreal h = 0; h <= w; h++) {
        pa.fillRect(QRect(h, 0, 1, hgt), QColor::fromHsvF(h / w, 1, 1));
    }

    img.setDevicePixelRatio(devicePixelRatioF());

    image = img;
}

void ColorSlider::setValue(qreal h)
{
    if (h < 0 || h > 1)
        return;

    currentX = h * (width() - 1);
    update();
}

QRect ColorSlider::handleRect() const
{
    // offset to center
    return QRect(currentX - 2, 2, 6, 16);
}

QRect ColorSlider::imageRect() const
{
    return QRect(0, 5, width(), 10);
}

void ColorSlider::paintEvent(QPaintEvent *e)
{
    QPainter pa(this);
    pa.setRenderHint(QPainter::Antialiasing);
    pa.setPen(Qt::NoPen);

    auto imgRect = imageRect();
    {
        pa.save();
        QPainterPath roundPath;
        roundPath.addRoundedRect(imgRect, 1, 1);
        pa.setClipPath(roundPath);
        pa.drawImage(imgRect, image);
        pa.restore();
    }

    // draw border
    pa.setBrush(QColor(0, 0, 0, 0.1 * 255));
    {
        QPainterPath out;
        out.addRoundedRect(imgRect, 1, 1);
        QPainterPath in;
        in.addRect(imgRect.marginsRemoved(QMargins(1, 1, 1, 1)));
        pa.drawPath(out - in);
    }

    // draw slider
    pa.setBrush(QColor(0, 0, 0, 0.2 * 255));
    pa.drawRect(handleRect());

    pa.setBrush(Qt::white);
    pa.drawRect(handleRect().marginsRemoved(QMargins(1, 1, 1, 1)));
}

void ColorSlider::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton) {
        pressed = true;
        auto pos = e->pos();
        if (handleRect().marginsAdded(QMargins(1, 1, 1, 1)).contains(pos)) {
            onHandle = true;
            clickOffset = pos.x() - currentX;
        } else {
            onHandle = false;
        }
    }
}

void ColorSlider::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
    e->accept();

    if (pressed && onHandle) {
        auto mx = e->pos().x() - clickOffset;

        if (mx < 0)
            mx = 0;

        if (mx >= width())
            mx = width() - 1;

        qreal h;
        if ((mx != currentX) && pickColor(mx, h)) {
            currentX = mx;
            emit valueChanged(h);
            update();
        }
    }
}

void ColorSlider::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mouseReleaseEvent(e);
    if (e->button() != Qt::LeftButton)
        return;

    auto pos = e->pos();
    // clicked on color
    if (pressed && !onHandle && imageRect().marginsAdded(QMargins(0, 2, 0, 2)).contains(pos)) {
        qreal h;
        if (pickColor(pos.x(), h)) {
            currentX = pos.x();
            emit valueChanged(h);
            update();
        }
    }

    pressed = false;
    onHandle = false;
}

ColorEditor::ColorEditor(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    setLayout(lay);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

    lay->addSpacing(prevewRect().width());

    // line editor width is PM_FrameMargins(1) + PM_ContentsMargins(10) + textmargin(0)
    // + SideWidgetList(0) + horizontalMargin(2) + curor(1) + contents("DDDDDD", 49)
    // #
    hexEditor = create("Hex", QSize(76, 24), lay); // 13 + 49 + 13 + 1
    hexEditor->setMaxLength(6);

    QValidator *hex = new QRegExpValidator(QRegExp(QString(R"(^([0-9a-fA-F]{6}|[0-9a-fA-F]{3})$)")), this);
    hexEditor->setValidator(hex);

    // r
    rEditor = create("R", QSize(46, 24), lay);
    rEditor->setMaxLength(3);

    // g
    gEditor = create("G", QSize(46, 24), lay);
    gEditor->setMaxLength(3);

    // b
    bEditor = create("B", QSize(46, 24), lay);
    bEditor->setMaxLength(3);

    QIntValidator *intValidator = new QIntValidator(0, 255, this);
    rEditor->setValidator(intValidator);
    gEditor->setValidator(intValidator);
    bEditor->setValidator(intValidator);

    connect(hexEditor, &QLineEdit::editingFinished, this, &ColorEditor::onNameChanged);
    connect(rEditor, &QLineEdit::editingFinished, this, &ColorEditor::onNodeChanged);
    connect(gEditor, &QLineEdit::editingFinished, this, &ColorEditor::onNodeChanged);
    connect(bEditor, &QLineEdit::editingFinished, this, &ColorEditor::onNodeChanged);
}

void ColorEditor::setCurrent(const QColor &c)
{
    current = c;
    hexEditor->setText(c.name().replace('#',"").toUpper());
    rEditor->setText(QString::number(c.red()));
    gEditor->setText(QString::number(c.green()));
    bEditor->setText(QString::number(c.blue()));
    update();
}

void ColorEditor::onNameChanged()
{
    QColor color(QString("#%0").arg(hexEditor->text()));
    setCurrent(color);
    emit colorEdited(color);
}

void ColorEditor::onNodeChanged()
{
    QColor color(
                rEditor->text().toInt(),
                gEditor->text().toInt(),
                bEditor->text().toInt()
                );
    setCurrent(color);
    emit colorEdited(color);
}

QLineEdit *ColorEditor::create(const QString &text, const QSize &sz, QBoxLayout *lay)
{
    QVBoxLayout *vlay = new QVBoxLayout;
    vlay->setContentsMargins(0, 0, 0, 0);
    vlay->setSpacing(0);

    QLineEdit *edit = new QLineEdit(this);
    DStyle::setFrameRadius(edit, 4);
    DFontSizeManager::instance()->bind(edit, DFontSizeManager::T8);
    edit->setFixedSize(sz);
    edit->setAlignment(Qt::AlignCenter);

    QLabel *label = new QLabel(this);
    label->setText(text);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedHeight(15);
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T10);

    vlay->addWidget(edit);
    vlay->addSpacing(4);
    vlay->addWidget(label, 0, Qt::AlignCenter);

    lay->addSpacing(4);
    lay->addLayout(vlay);

    return edit;
}

void ColorEditor::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter pa(this);
    pa.setRenderHint(QPainter::Antialiasing);

    // draw preview
    const QRect r = prevewRect();
    pa.setPen(Qt::NoPen);
    pa.setBrush(current);
    pa.drawRoundedRect(r, 4, 4);

    // draw border
    pa.setBrush(QColor(0, 0, 0, 0.1 * 255));
    {
        QPainterPath out;
        out.addRoundedRect(r, 4, 4);
        QPainterPath in;
        in.addRoundedRect(r.marginsRemoved(QMargins(1, 1, 1, 1)), 4, 4);
        pa.drawPath(out - in);
    }
}

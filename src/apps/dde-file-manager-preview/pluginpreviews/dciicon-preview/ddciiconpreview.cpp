// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddciiconpreview.h"

#include <dfm-base/interfaces/abstractbasepreview.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/fileutils.h>

#include <dtkgui_config.h>

#ifdef DTKGUI_CLASS_DDciIcon
#    include <DDciIcon>
#endif

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QApplication>
#include <QSplitter>
#include <QGraphicsItem>
#include <QBasicTimer>
#include <QtMath>
#include <QTimer>
#include <QLabel>
#include <QResizeEvent>

namespace plugin_filepreview {
DFM_LOG_REGISTER_CATEGORY(PREVIEW_NAMESPACE)

DFMBASE_USE_NAMESPACE
#ifdef DTKGUI_CLASS_DDciIcon
DGUI_USE_NAMESPACE;
#endif

#ifdef DTKGUI_CLASS_DDciIcon
static QPixmap generateTransparentTexture()
{
    QPixmap tilePixmap(64, 64);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, 0x20, 0x20, color);
    tilePainter.fillRect(0x20, 0x20, 0x20, 0x20, color);
    tilePainter.end();
    return tilePixmap;
}

class IconOptionWidget : public QWidget
{
public:
    explicit IconOptionWidget(QWidget *parent = nullptr)
        : QWidget(parent), titleLabel(nullptr), mainLayout(nullptr), titleLayout(nullptr)
    {
        this->setFocusPolicy(Qt::ClickFocus);
        this->mainLayout = new QVBoxLayout(this);
        this->mainLayout->setContentsMargins(10, 10, 10, 10);
        this->mainLayout->setSpacing(5);
        this->titleLayout = new QHBoxLayout;
        this->titleLayout->setContentsMargins(0, 0, 0, 0);
        this->titleLabel = new QLabel(this);
        auto font = this->titleLabel->font();
        font.setBold(true);
        font.setPointSize(12);
        this->titleLabel->setFont(font);
        this->titleLayout->addWidget(this->titleLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
        this->titleLayout->addStretch();
        this->mainLayout->addLayout(this->titleLayout);
    }

    void setTitleText(const QString &title)
    {
        this->titleLabel->setText(title);
    }
    void addHeaderWidget(QWidget *w)
    {
        this->titleLayout->addWidget(w);
    }
    void addContentWidget(QWidget *w)
    {
        this->mainLayout->addWidget(w);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(this->palette().base());
        painter.drawRoundedRect(this->rect(), 8, 8);
    }

private:
    QLabel *titleLabel;
    QVBoxLayout *mainLayout;
    QHBoxLayout *titleLayout;
};

class IconPreviewView : public QGraphicsView
{
public:
    explicit IconPreviewView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QLabel *scaleFactorLabel;
    QTimer scaleHideTimer;
};

IconPreviewView::IconPreviewView(QWidget *parent)
    : QGraphicsView(parent)
{
    this->setFrameShape(QFrame::NoFrame);
    auto palette = this->palette();
    palette.setBrush(QPalette::Base, Qt::transparent);
    this->setPalette(palette);
    this->setInteractive(true);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    this->setRenderHint(QPainter::SmoothPixmapTransform, true);
    this->setLineWidth(0);
    this->setForegroundBrush(Qt::transparent);
    this->setDragMode(ScrollHandDrag);
    this->setBackgroundBrush(Qt::white);

    this->scaleFactorLabel = new QLabel(this);
    this->scaleFactorLabel->setVisible(false);
    this->scaleHideTimer.setSingleShot(true);
    this->scaleHideTimer.setInterval(800);
    QObject::connect(&this->scaleHideTimer, &QTimer::timeout, this, [this]() {
        this->scaleFactorLabel->hide();
        this->update();
    });
}

void IconPreviewView::wheelEvent(QWheelEvent *event)
{
    qreal factor = qPow(1.2, event->angleDelta().y() / 240.0);
    // cap to 0.001 - 1000
    qreal actualFactor = qBound(0.001, factor, 1000.0);
    this->scale(actualFactor, actualFactor);
    this->scaleFactorLabel->setText(QString("%1%").arg(qRound(this->transform().m11() * 100)));
    this->scaleFactorLabel->adjustSize();
    QRect rect = scaleFactorLabel->rect();
    rect.moveCenter(this->rect().center());
    rect.moveTop(this->rect().top());
    rect = rect.adjusted(0, 5, 0, 5);
    this->scaleFactorLabel->move(rect.topLeft());
    this->scaleFactorLabel->setVisible(true);
    this->scaleHideTimer.start();
    event->accept();
}

void IconPreviewView::drawBackground(QPainter *painter, const QRectF &)
{
    painter->save();
    painter->resetTransform();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing);

    if (this->backgroundBrush().texture().isNull()) {
        painter->setBrush(this->backgroundBrush());
        painter->drawRoundedRect(this->viewport()->rect(), 8, 8);
    } else {
        QPainterPath path;
        path.addRoundedRect(this->viewport()->rect(), 8, 8);
        painter->setClipPath(path);
        painter->drawTiledPixmap(this->viewport()->rect(),
                                 this->backgroundBrush().texture());
    }

    painter->restore();
}

class DDciIconPreview : public AbstractBasePreview
{
public:
    explicit DDciIconPreview(QObject *parent = nullptr);
    virtual ~DDciIconPreview() override;

    void initialize(QWidget *window, QWidget *statusBar) override;
    QString title() const override;
    QWidget *statusBarWidget() const override;
    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;
    QWidget *contentWidget() const override;

protected:
    void initControlWidgets();
    void initPreviewWidgets();
    void initializeSettings(const QString &localUrl);
    DDciIconPalette generateDciIconPalette();
    void updateIconMatchedResult();
    void updatePixmap();
    void updatePixmapImpl();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void timerEvent(QTimerEvent *e) override;
    int getIconSize();

private:
    IconPreviewView *view;
    QGraphicsScene *scene;
    QGraphicsPixmapItem *iconItem;
    QWidget *mainWidget;
    QWidget *controlWidget;
    QComboBox *availableSizeCombo;
    QLabel *devicePixelRatioLabel;
    QLabel *paletteNosupportedText;
    QLineEdit *foregroundPaletteEdit;
    QLineEdit *backgroundPaletteEdit;
    QLineEdit *hightlightPaletteEdit;
    QLineEdit *hFPaletteEdit;
    IconOptionWidget *paletteWidget;
    QComboBox *themeCom;
    QComboBox *modeCom;
    QLineEdit *customSizeEdit;
    QUrl url;
    DDciIcon *dciIcon;
    DDciIconMatchResult dciIconMatched;
    QBasicTimer updateTimer;
    QString titleText;
};

DDciIconPreview::DDciIconPreview(QObject *parent)
    : AbstractBasePreview(parent), view(nullptr), scene(nullptr), mainWidget(nullptr), controlWidget(nullptr), availableSizeCombo(nullptr), devicePixelRatioLabel(nullptr), paletteNosupportedText(nullptr), foregroundPaletteEdit(nullptr), backgroundPaletteEdit(nullptr), hightlightPaletteEdit(nullptr), hFPaletteEdit(nullptr), paletteWidget(nullptr), themeCom(nullptr), modeCom(nullptr), customSizeEdit(nullptr), dciIcon(nullptr), dciIconMatched(nullptr)
{
    fmInfo() << "DCI icon preview: DDciIconPreview instance created";
}

DDciIconPreview::~DDciIconPreview()
{
    fmInfo() << "DCI icon preview: DDciIconPreview instance destroyed";
    delete dciIcon;
}

void DDciIconPreview::initialize(QWidget *window, QWidget *statusBar)
{
    Q_UNUSED(window);
    Q_UNUSED(statusBar);

    fmDebug() << "DCI icon preview: initializing preview widget";

    this->mainWidget = new QWidget;
    this->mainWidget->setFixedSize(1200, 800);
    QHBoxLayout *mainLayout = new QHBoxLayout(this->mainWidget);
    mainLayout->setContentsMargins(10, 20, 10, 20);

    this->initControlWidgets();
    this->initPreviewWidgets();

    QSplitter *splitter = new QSplitter(this->mainWidget);
    splitter->addWidget(this->controlWidget);
    splitter->addWidget(this->view);
    splitter->setSizes({ 80, 920 });
    mainLayout->addWidget(splitter);
    
    fmDebug() << "DCI icon preview: initialization completed";
}

void DDciIconPreview::initControlWidgets()
{
    this->controlWidget = new QWidget(this->mainWidget);
    this->controlWidget->setFocusPolicy(Qt::ClickFocus);
    QVBoxLayout *controlLayout = new QVBoxLayout(this->controlWidget);
    controlLayout->setContentsMargins(0, 0, 20, 0);
    controlLayout->setSpacing(10);

    IconOptionWidget *availableSizeWidget = new IconOptionWidget;
    availableSizeWidget->setTitleText(tr("Available sizes: "));
    this->availableSizeCombo = new QComboBox();
    this->availableSizeCombo->insertItem(this->availableSizeCombo->count(), tr("Custom Size"));
    availableSizeWidget->addHeaderWidget(this->availableSizeCombo);

    QWidget *customSizeWidget = new QWidget();
    QHBoxLayout *customSizeLayout = new QHBoxLayout(customSizeWidget);
    customSizeLayout->setContentsMargins(0, 0, 0, 0);

    this->customSizeEdit = new QLineEdit();
    QObject::connect(this->customSizeEdit, &QLineEdit::editingFinished, this, std::bind(&DDciIconPreview::updatePixmap, this));
    this->customSizeEdit->setClearButtonEnabled(true);
    this->customSizeEdit->setFixedWidth(120);
    this->customSizeEdit->setValidator(new QIntValidator(this->customSizeEdit));
    customSizeLayout->addStretch();
    customSizeLayout->addWidget(this->customSizeEdit);
    availableSizeWidget->addContentWidget(customSizeWidget);
    QObject::connect(this->availableSizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                     [=](int index) {
                         if (index == this->availableSizeCombo->count() - 1) {
                             this->customSizeEdit->setVisible(true);
                             this->customSizeEdit->setFocus();
                         } else {
                             this->updatePixmap();
                             this->customSizeEdit->setVisible(false);
                         }
                     });

    IconOptionWidget *devicePixelRatioWidget = new IconOptionWidget;
    devicePixelRatioWidget->setTitleText(tr("Device Pixel Ratio: "));
    this->devicePixelRatioLabel = new QLabel(this->controlWidget);
    devicePixelRatioWidget->addHeaderWidget(this->devicePixelRatioLabel);

    QWidget *devicePixelSliderWidget = new QWidget;
    devicePixelRatioWidget->addContentWidget(devicePixelSliderWidget);
    QHBoxLayout *devicePixelSliderLayout = new QHBoxLayout(devicePixelSliderWidget);
    devicePixelSliderLayout->setContentsMargins(0, 0, 0, 0);
    devicePixelSliderLayout->setSpacing(0);
    QSlider *devicePixelSlider = new QSlider(this->controlWidget);
    devicePixelSliderLayout->addWidget(devicePixelSlider);
    devicePixelSlider->setOrientation(Qt::Horizontal);
    devicePixelSlider->setRange(0, 30);
    devicePixelSlider->setSingleStep(5);
    QObject::connect(devicePixelSlider, &QSlider::valueChanged, this,
                     [this, devicePixelSlider]() {
                         this->devicePixelRatioLabel->setText(QString::number(devicePixelSlider->value() * 1.0 / 10, 'g', 2));
                         this->updatePixmap();
                     });
    devicePixelSlider->setValue(static_cast<int>(qApp->devicePixelRatio() * 10));

    IconOptionWidget *themeWidget = new IconOptionWidget;
    themeWidget->setTitleText(tr("Theme: "));
    this->themeCom = new QComboBox(this->controlWidget);
    this->themeCom->addItems({ tr("Light"), tr("Dark") });
    themeWidget->addHeaderWidget(this->themeCom);

    IconOptionWidget *modeWidget = new IconOptionWidget;
    modeWidget->setTitleText(tr("Mode: "));
    this->modeCom = new QComboBox(this->controlWidget);
    this->modeCom->addItems({ tr("Normal"), tr("Disabled"), tr("Hovered"), tr("Pressed") });
    modeWidget->addHeaderWidget(this->modeCom);

    this->paletteWidget = new IconOptionWidget;
    this->paletteWidget->setTitleText(tr("Palette"));

    this->paletteNosupportedText = new QLabel(tr("Current mode icon does not support the palette"), this->paletteWidget);
    this->paletteNosupportedText->setVisible(false);
    auto textFont = this->paletteNosupportedText->font();
    textFont.setPointSize(8);
    textFont.setItalic(true);
    this->paletteNosupportedText->setFont(textFont);
    auto palette = this->paletteNosupportedText->palette();
    palette.setBrush(QPalette::WindowText, QColor(220, 20, 60));
    this->paletteNosupportedText->setPalette(palette);
    this->paletteWidget->addHeaderWidget(this->paletteNosupportedText);

    QWidget *paletteContentWidget = new QWidget;
    this->paletteWidget->addContentWidget(paletteContentWidget);
    QVBoxLayout *paletteContentLayout = new QVBoxLayout(paletteContentWidget);
    paletteContentLayout->setContentsMargins(0, 0, 0, 0);
    paletteContentLayout->setSpacing(4);
    QHBoxLayout *foregroundPaletteLayout = new QHBoxLayout;
    foregroundPaletteLayout->setContentsMargins(10, 0, 0, 0);
    foregroundPaletteLayout->setSpacing(2);
    QLabel *foregroundPaletteTitle = new QLabel(tr("Foreground:"), this->controlWidget);
    foregroundPaletteTitle->setFixedWidth(140);
    auto font = foregroundPaletteTitle->font();
    font.setPointSize(10);
    foregroundPaletteTitle->setFont(font);
    this->foregroundPaletteEdit = new QLineEdit(this->controlWidget);
    this->foregroundPaletteEdit->setText(this->mainWidget->palette().windowText().color().name());
    this->foregroundPaletteEdit->setFixedWidth(100);
    this->foregroundPaletteEdit->setFont(font);
    QObject::connect(this->foregroundPaletteEdit, &QLineEdit::editingFinished, this, std::bind(&DDciIconPreview::updatePixmap, this));
    foregroundPaletteLayout->addWidget(foregroundPaletteTitle);
    foregroundPaletteLayout->addWidget(this->foregroundPaletteEdit, 1);
    QHBoxLayout *backgroundPaletteLayout = new QHBoxLayout;
    backgroundPaletteLayout->setContentsMargins(10, 0, 0, 0);
    backgroundPaletteLayout->setSpacing(2);
    QLabel *backgroundPaletteTitle = new QLabel(tr("Background:"), this->controlWidget);
    backgroundPaletteTitle->setFixedWidth(140);
    this->backgroundPaletteEdit = new QLineEdit(this->controlWidget);
    this->backgroundPaletteEdit->setText(this->mainWidget->palette().window().color().name());
    this->backgroundPaletteEdit->setFixedWidth(100);
    this->backgroundPaletteEdit->setFont(font);
    QObject::connect(this->backgroundPaletteEdit, &QLineEdit::editingFinished, this, std::bind(&DDciIconPreview::updatePixmap, this));
    backgroundPaletteLayout->addWidget(backgroundPaletteTitle);
    backgroundPaletteLayout->addWidget(this->backgroundPaletteEdit, 1);

    QHBoxLayout *hightlightPaletteLayout = new QHBoxLayout;
    hightlightPaletteLayout->setContentsMargins(10, 0, 0, 0);
    hightlightPaletteLayout->setSpacing(2);
    QLabel *hightlightPaletteTitle = new QLabel(tr("Highlight:"), this->controlWidget);
    hightlightPaletteTitle->setFixedWidth(140);
    this->hightlightPaletteEdit = new QLineEdit(this->controlWidget);
    this->hightlightPaletteEdit->setText(this->mainWidget->palette().highlight().color().name());
    this->hightlightPaletteEdit->setFixedWidth(100);
    this->hightlightPaletteEdit->setFont(font);
    QObject::connect(this->hightlightPaletteEdit, &QLineEdit::editingFinished, this, std::bind(&DDciIconPreview::updatePixmap, this));
    hightlightPaletteLayout->addWidget(hightlightPaletteTitle);
    hightlightPaletteLayout->addWidget(this->hightlightPaletteEdit, 1);
    QHBoxLayout *hFPaletteLayout = new QHBoxLayout;
    hFPaletteLayout->setContentsMargins(10, 0, 0, 0);
    hFPaletteLayout->setSpacing(2);
    QLabel *hFPaletteTitle = new QLabel(tr("HighlightForeground:"), this->controlWidget);
    hFPaletteTitle->setFixedWidth(140);
    hFPaletteTitle->setFont(font);
    this->hFPaletteEdit = new QLineEdit(this->controlWidget);
    this->hFPaletteEdit->setText(this->mainWidget->palette().highlightedText().color().name());
    this->hFPaletteEdit->setFixedWidth(100);
    this->hFPaletteEdit->setFont(font);
    QObject::connect(this->hFPaletteEdit, &QLineEdit::editingFinished, this, std::bind(&DDciIconPreview::updatePixmap, this));
    hFPaletteLayout->addWidget(hFPaletteTitle);
    hFPaletteLayout->addWidget(this->hFPaletteEdit, 1);
    paletteContentLayout->addLayout(foregroundPaletteLayout);
    paletteContentLayout->addLayout(backgroundPaletteLayout);
    paletteContentLayout->addLayout(hightlightPaletteLayout);
    paletteContentLayout->addLayout(hFPaletteLayout);

    IconOptionWidget *backgroundColorWidget = new IconOptionWidget;
    backgroundColorWidget->setTitleText(tr("Background Color: "));
    QComboBox *backgroundColorCom = new QComboBox(this->controlWidget);
    backgroundColorCom->addItems({ tr("White"), tr("Black"), tr("Transparent"), tr("Custom") });
    backgroundColorWidget->addHeaderWidget(backgroundColorCom);

    QWidget *customBackgroundWidget = new QWidget;
    backgroundColorWidget->addContentWidget(customBackgroundWidget);
    QHBoxLayout *customBackgroundLayout = new QHBoxLayout(customBackgroundWidget);
    customBackgroundLayout->setContentsMargins(0, 0, 0, 0);
    customBackgroundLayout->setSpacing(10);
    QLineEdit *cusBackEdit = new QLineEdit();
    cusBackEdit->setVisible(false);
    cusBackEdit->setFixedWidth(120);
    customBackgroundLayout->addStretch();
    customBackgroundLayout->addWidget(cusBackEdit);
    QObject::connect(cusBackEdit, &QLineEdit::editingFinished, this, [=]() {
        QColor color(cusBackEdit->text());
        if (!color.isValid())
            return;
        this->view->setBackgroundBrush(color);
    });
    QObject::connect(backgroundColorCom, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                     [=](int index) {
                         switch (index) {
                         case 0:
                             this->view->setBackgroundBrush(Qt::white);
                             cusBackEdit->setVisible(false);
                             break;
                         case 1:
                             this->view->setBackgroundBrush(Qt::black);
                             cusBackEdit->setVisible(false);
                             break;
                         case 2:
                             static const QPixmap &transparentTexture = generateTransparentTexture();
                             this->view->setBackgroundBrush(transparentTexture);
                             cusBackEdit->setVisible(false);
                             break;
                         default:
                             cusBackEdit->setVisible(true);
                             cusBackEdit->setFocus();
                         }
                     });

    controlLayout->addWidget(availableSizeWidget);
    controlLayout->addWidget(devicePixelRatioWidget);
    controlLayout->addWidget(themeWidget);
    controlLayout->addWidget(modeWidget);
    controlLayout->addWidget(this->paletteWidget);
    controlLayout->addWidget(backgroundColorWidget);
    controlLayout->addStretch();
}

void DDciIconPreview::initPreviewWidgets()
{
    this->view = new IconPreviewView();
    this->view->installEventFilter(this);
    this->scene = new QGraphicsScene(this->view);
    this->view->setScene(this->scene);

    this->iconItem = new QGraphicsPixmapItem;
    this->iconItem->setTransformationMode(Qt::SmoothTransformation);
    this->iconItem->setPos(this->scene->sceneRect().center());
    this->iconItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
    this->iconItem->setFlag(QGraphicsItem::ItemIsMovable, false);
    this->iconItem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    this->scene->addItem(this->iconItem);
}

void DDciIconPreview::initializeSettings(const QString &localUrl)
{
    fmDebug() << "DCI icon preview: initializing settings for file:" << localUrl;
    
    if (localUrl.isEmpty()) {
        fmWarning() << "DCI icon preview: empty file path provided";
        return;
    }

    if (this->dciIcon)
        delete this->dciIcon;
    this->dciIcon = new DDciIcon(localUrl);
    if (this->dciIcon->isNull()) {
        fmWarning() << "DCI icon preview: failed to load DCI icon from:" << localUrl;
        delete this->dciIcon;
        return;
    }

    auto availableSizes = this->dciIcon->availableSizes(DDciIcon::Light);
    fmDebug() << "DCI icon preview: found" << availableSizes.size() << "available sizes for:" << localUrl;
    
    for (int i = 0; i < availableSizes.size(); ++i)
        this->availableSizeCombo->insertItem(i, QString::number(availableSizes[i]));
    this->availableSizeCombo->setCurrentIndex(0);
    this->updatePixmap();
    
    fmInfo() << "DCI icon preview: settings initialized successfully for:" << localUrl;
}

DDciIconPalette DDciIconPreview::generateDciIconPalette()
{
    DDciIconPalette palette;
    palette.setForeground(QColor(this->foregroundPaletteEdit->text()));
    palette.setBackground(QColor(this->backgroundPaletteEdit->text()));
    palette.setHighlight(QColor(this->hightlightPaletteEdit->text()));
    palette.setHighlightForeground(QColor(this->hFPaletteEdit->text()));
    return palette;
}

void DDciIconPreview::updateIconMatchedResult()
{
    if (!this->dciIcon) {
        fmWarning() << "DCI icon preview: no DCI icon loaded for matching";
        return;
    }

    int iconSize = this->getIconSize();
    if (!iconSize) {
        fmDebug() << "DCI icon preview: invalid icon size for matching";
        this->dciIconMatched = nullptr;
        return;
    }

    DDciIcon::Theme theme = this->themeCom->currentIndex() == 0 ? DDciIcon::Light : DDciIcon::Dark;
    DDciIcon::Mode mode = DDciIcon::Mode(this->modeCom->currentIndex());
    this->dciIconMatched = this->dciIcon->matchIcon(iconSize, theme, mode, DDciIcon::DontFallbackMode);
    
    fmDebug() << "DCI icon preview: icon matched with size:" << iconSize << "theme:" << theme << "mode:" << mode;
}

static QPixmap invaildPixmap()
{
    static QString text = "Invalid icon";
    static QPixmap pixmap(150, 80);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setPen(QColor(219, 112, 147));
    p.setBrush(Qt::NoBrush);
    p.drawText(pixmap.rect(), Qt::AlignCenter | Qt::TextSingleLine, text);
    return pixmap;
}

void DDciIconPreview::updatePixmapImpl()
{
    auto pixmap = invaildPixmap();
    this->updateIconMatchedResult();

    do {
        if (!this->dciIconMatched)
            break;

        bool supportPalette = this->dciIcon->isSupportedAttribute(this->dciIconMatched, DDciIcon::HasPalette);
        this->paletteWidget->setEnabled(supportPalette);
        this->paletteNosupportedText->setVisible(!supportPalette);

        int iconSize = this->getIconSize();
        if (iconSize == 0)
            break;

        double devicePixelRatio = this->devicePixelRatioLabel->text().toDouble();
        if (qFuzzyCompare(devicePixelRatio, 0))
            break;

        auto palette = this->generateDciIconPalette();
        pixmap = this->dciIcon->pixmap(devicePixelRatio, iconSize, this->dciIconMatched, palette);
        pixmap.setDevicePixelRatio(qApp->devicePixelRatio());
    } while (false);

    this->iconItem->setPixmap(pixmap);
    QRectF rect = this->iconItem->boundingRect();
    rect.moveCenter(this->scene->sceneRect().center());
    this->iconItem->setPos(rect.topLeft());
}

void DDciIconPreview::updatePixmap()
{
    if (this->updateTimer.isActive())
        return;
    this->updateTimer.start(50, this);
}

bool DDciIconPreview::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this->view) {
        if (event->type() == QEvent::Resize) {
            QResizeEvent *re = static_cast<QResizeEvent *>(event);
            this->scene->setSceneRect(QRectF({ 0, 0 }, re->size()));
            QRectF rect = this->iconItem->boundingRect();
            rect.moveCenter(this->scene->sceneRect().center());
            this->iconItem->setPos(rect.topLeft());
        }
    }
    return false;
}

void DDciIconPreview::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == this->updateTimer.timerId()) {
        this->updateTimer.stop();
        this->updatePixmapImpl();
    }
    return AbstractBasePreview::timerEvent(e);
}

int DDciIconPreview::getIconSize()
{
    bool converted = false;
    int iconSize = this->availableSizeCombo->currentText().toInt(&converted);
    if (!converted) {
        if (this->availableSizeCombo->currentIndex() == this->availableSizeCombo->count() - 1)
            iconSize = this->customSizeEdit->text().toInt();
    }

    return iconSize;
}

QString DDciIconPreview::title() const
{
    return this->titleText;
}

QWidget *DDciIconPreview::statusBarWidget() const
{
    return nullptr;
}

bool DDciIconPreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "DCI icon preview: setting file URL:" << url;
    
    if (this->url == url) {
        fmDebug() << "DCI icon preview: URL unchanged, skipping:" << url;
        return true;
    }

    if (!url.fileName().endsWith(QLatin1String(".dci"))) {
        fmWarning() << "DCI icon preview: file does not have .dci extension:" << url;
        return false;
    }

    QUrl tmpUrl = UrlRoute::fromLocalFile(url.path());
    if (!url.isLocalFile()) {
        fmWarning() << "DCI icon preview: URL is not a local file:" << url;
        return false;
    }

    QByteArray format;
    QMimeDatabase mimeDatabase;
    const QMimeType &mt = mimeDatabase.mimeTypeForFile(url.toLocalFile(), QMimeDatabase::MatchContent);
    if (!mt.isValid()) {
        fmWarning() << "DCI icon preview: invalid MIME type for file:" << url;
        return false;
    }
    if (!mt.preferredSuffix().toLatin1().endsWith("dci")) {
        fmWarning() << "DCI icon preview: MIME type does not match DCI format:" << mt.preferredSuffix() << "for file:" << url;
        return false;
    }

    this->url = tmpUrl;
    this->initializeSettings(this->url.toLocalFile());
    this->titleText = QFileInfo(tmpUrl.toLocalFile()).fileName();
    
    fmInfo() << "DCI icon preview: file URL set successfully:" << url << "title:" << titleText;
    return true;
}

QUrl DDciIconPreview::fileUrl() const
{
    return this->url;
}

QWidget *DDciIconPreview::contentWidget() const
{
    return this->mainWidget;
}
#endif

AbstractBasePreview *DDciIconPreviewPlugin::create(const QString &)
{
#ifdef DTKGUI_CLASS_DDciIcon
    return new DDciIconPreview();
#endif
    return nullptr;
}
}   // namespace plugin_filepreview

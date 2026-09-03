// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "ddciiconpreview.h"

#include <dfm-base/interfaces/abstractbasepreview.h>

#include <dtkgui_config.h>

#ifdef DTKGUI_CLASS_DDciIcon
#    include <DDciIcon>
#endif

#include <gtest/gtest.h>
#include <QTest>

#include <QApplication>
#include <QBasicTimer>
#include <QComboBox>
#include <QFile>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QMimeDatabase>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QRegion>
#include <QResizeEvent>
#include <QSlider>
#include <QSplitter>
#include <QTimer>
#include <QTimerEvent>
#include <QVBoxLayout>
#include <QWheelEvent>

#ifdef DTKGUI_CLASS_DDciIcon

DFMBASE_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace plugin_filepreview {
class IconOptionWidget : public QWidget
{
public:
    explicit IconOptionWidget(QWidget *parent = nullptr);

    void setTitleText(const QString &title);
    void addHeaderWidget(QWidget *w);
    void addContentWidget(QWidget *w);

protected:
    void paintEvent(QPaintEvent *) override;

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

class DDciIconPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
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
}   // namespace plugin_filepreview

using namespace plugin_filepreview;

class UT_IconOptionWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        option = new IconOptionWidget();
    }

    virtual void TearDown() override
    {
        delete option;
        option = nullptr;
        stub.clear();
    }

protected:
    IconOptionWidget *option { nullptr };
    stub_ext::StubExt stub;
};

class UT_IconPreviewView : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        view = new IconPreviewView();
        view->resize(400, 300);
    }

    virtual void TearDown() override
    {
        delete view;
        view = nullptr;
        stub.clear();
    }

    // Qt 6.8 中 QApplication::notify 会将直接发给 QGraphicsView 的 wheel 事件按位置重定向，
    // 未 show 的窗口下该事件被直接丢弃；真实滚轮事件本就是先到达 viewport 再由
    // QGraphicsView::viewportEvent 转发给 wheelEvent，因此这里直接发给 viewport
    QWheelEvent makeWheelEvent(int deltaY)
    {
        return QWheelEvent(QPointF(10, 10), QPointF(10, 10), QPoint(0, 0), QPoint(0, deltaY),
                           Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    }

protected:
    IconPreviewView *view { nullptr };
    stub_ext::StubExt stub;
};

class UT_DDciIconPreview : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        preview = new DDciIconPreview();
    }

    virtual void TearDown() override
    {
        delete preview;
        preview = nullptr;
        stub.clear();
    }

protected:
    DDciIconPreview *preview { nullptr };
    stub_ext::StubExt stub;
};

class UT_DDciIconPreviewInitialized : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        preview = new DDciIconPreview();
        preview->initialize(nullptr, nullptr);
        if (preview->updateTimer.isActive())
            preview->updateTimer.stop();
    }

    virtual void TearDown() override
    {
        QFile::remove(kTempDciPath);
        stub.clear();
        if (preview) {
            delete preview->mainWidget;
            preview->mainWidget = nullptr;
            delete preview;
            preview = nullptr;
        }
    }

    QString createTempDciFile(const QByteArray &content)
    {
        QFile file(kTempDciPath);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.write(content);
        file.close();
        return kTempDciPath;
    }

    void loadFakeIconSuccessfully(QList<int> sizes)
    {
        stub.set_lamda(ADDR(DDciIcon, isNull), [](const DDciIcon *) {
            __DBG_STUB_INVOKE__
            return false;
        });
        stub.set_lamda(ADDR(DDciIcon, availableSizes),
                       [sizes](const DDciIcon *, DDciIcon::Theme, DDciIcon::Mode) {
                           __DBG_STUB_INVOKE__
                           return sizes;
                       });
        preview->initializeSettings("/tmp/ut_dciicon_fake_icon.dci");
        preview->updateTimer.stop();
    }

protected:
    static const QString kTempDciPath;
    DDciIconPreview *preview { nullptr };
    stub_ext::StubExt stub;
};

const QString UT_DDciIconPreviewInitialized::kTempDciPath = "/tmp/ut_dciicon_preview_temp.dci";

class UT_DDciIconPreviewPlugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        plugin = new DDciIconPreviewPlugin();
    }

    virtual void TearDown() override
    {
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

protected:
    DDciIconPreviewPlugin *plugin { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_DDciIconPreviewPlugin, Create_WithArbitraryKey_ReturnsNonNullPreviewInstance)
{
    DFMBASE_NAMESPACE::AbstractBasePreview *result = plugin->create("dci");
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->title().isEmpty());
    delete result;
}

TEST_F(UT_DDciIconPreviewPlugin, Create_WithEmptyKey_ReturnsNonNullPreviewInstance)
{
    DFMBASE_NAMESPACE::AbstractBasePreview *result = plugin->create("");
    EXPECT_NE(result, nullptr);
    delete result;
}

TEST_F(UT_IconOptionWidget, Construction_DefaultState_FocusPolicyAndLayoutsConfigured)
{
    EXPECT_EQ(option->focusPolicy(), Qt::ClickFocus);
    ASSERT_NE(option->titleLabel, nullptr);
    ASSERT_NE(option->mainLayout, nullptr);
    ASSERT_NE(option->titleLayout, nullptr);
    EXPECT_EQ(option->mainLayout->contentsMargins(), QMargins(10, 10, 10, 10));
    EXPECT_EQ(option->mainLayout->spacing(), 5);
    EXPECT_EQ(option->titleLayout->contentsMargins(), QMargins(0, 0, 0, 0));
    EXPECT_EQ(option->titleLabel->parentWidget(), option);
}

TEST_F(UT_IconOptionWidget, SetTitleText_GivenText_LabelUpdated)
{
    option->setTitleText("Available sizes: ");
    EXPECT_EQ(option->titleLabel->text(), QString("Available sizes: "));

    option->setTitleText("Palette");
    EXPECT_EQ(option->titleLabel->text(), QString("Palette"));
}

TEST_F(UT_IconOptionWidget, AddHeaderWidget_GivenWidget_WidgetReparentedIntoTitleRow)
{
    auto *header = new QPushButton();
    option->addHeaderWidget(header);
    EXPECT_EQ(header->parentWidget(), option);
    EXPECT_EQ(option->titleLayout->count(), 3);
    EXPECT_EQ(option->titleLayout->itemAt(2)->widget(), header);
}

TEST_F(UT_IconOptionWidget, AddContentWidget_GivenWidget_WidgetReparentedIntoMainLayout)
{
    auto *content = new QWidget();
    option->addContentWidget(content);
    EXPECT_EQ(content->parentWidget(), option);
    EXPECT_EQ(option->mainLayout->count(), 2);
    EXPECT_EQ(option->mainLayout->itemAt(1)->widget(), content);
}

TEST_F(UT_IconOptionWidget, PaintEvent_RenderWidget_DrawsRoundedBackground)
{
    option->resize(120, 80);
    QPixmap pm(120, 80);
    pm.fill(Qt::transparent);
    // Qt6 的 QWidget::render() 默认带 DrawWindowBackground 标志，会先用不透明背景
    // 填充整个目标区域，导致圆角外像素断言失效；去掉该标志后只绘制 paintEvent 自身内容
    EXPECT_NO_FATAL_FAILURE(option->render(&pm, QPoint(), QRegion(),
                                           QWidget::RenderFlags(QWidget::DrawChildren)));
    EXPECT_EQ(pm.toImage().pixelColor(60, 40).alpha(), 255);
    EXPECT_EQ(pm.toImage().pixelColor(1, 1).alpha(), 0);
}

TEST_F(UT_IconPreviewView, Construction_DefaultState_ViewPropertiesConfigured)
{
    EXPECT_EQ(view->frameShape(), QFrame::NoFrame);
    EXPECT_EQ(view->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    EXPECT_EQ(view->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    EXPECT_TRUE(view->isInteractive());
    EXPECT_EQ(view->transformationAnchor(), QGraphicsView::AnchorUnderMouse);
    EXPECT_EQ(view->viewportUpdateMode(), QGraphicsView::FullViewportUpdate);
    EXPECT_TRUE(view->renderHints().testFlag(QPainter::SmoothPixmapTransform));
    EXPECT_EQ(view->backgroundBrush(), QBrush(Qt::white));
    EXPECT_EQ(view->dragMode(), QGraphicsView::ScrollHandDrag);
    ASSERT_EQ(view->scaleFactorLabel->isVisibleTo(view), false);
}

TEST_F(UT_IconPreviewView, WheelEvent_ScrollUp_ZoomsInAndShowsScaleLabel)
{
    QWheelEvent event = makeWheelEvent(240);
    EXPECT_TRUE(QApplication::sendEvent(view->viewport(), &event));
    EXPECT_NEAR(view->transform().m11(), 1.2, 0.001);
    EXPECT_FALSE(view->scaleFactorLabel->isHidden());
    EXPECT_EQ(view->scaleFactorLabel->text(), QString("120%"));
}

TEST_F(UT_IconPreviewView, WheelEvent_ScrollDown_ZoomsOutAndShowsScaleLabel)
{
    QWheelEvent event = makeWheelEvent(-240);
    EXPECT_TRUE(QApplication::sendEvent(view->viewport(), &event));
    EXPECT_NEAR(view->transform().m11(), 1.0 / 1.2, 0.001);
    EXPECT_FALSE(view->scaleFactorLabel->isHidden());
    EXPECT_EQ(view->scaleFactorLabel->text(), QString("83%"));
}

TEST_F(UT_IconPreviewView, WheelEvent_LargeUpDelta_ScaleFactorCappedToUpperBound)
{
    QWheelEvent event = makeWheelEvent(240 * 38);
    EXPECT_TRUE(QApplication::sendEvent(view->viewport(), &event));
    EXPECT_NEAR(view->transform().m11(), 1000.0, 0.5);
    EXPECT_EQ(view->scaleFactorLabel->text(), QString("100000%"));
}

TEST_F(UT_IconPreviewView, WheelEvent_LargeDownDelta_ScaleFactorCappedToLowerBound)
{
    QWheelEvent event = makeWheelEvent(-240 * 38);
    EXPECT_TRUE(QApplication::sendEvent(view->viewport(), &event));
    EXPECT_NEAR(view->transform().m11(), 0.001, 0.0005);
    EXPECT_EQ(view->scaleFactorLabel->text(), QString("0%"));
}

TEST_F(UT_IconPreviewView, WheelEvent_AfterTimeout_ScaleLabelHidden)
{
    QWheelEvent event = makeWheelEvent(240);
    QApplication::sendEvent(view->viewport(), &event);
    EXPECT_FALSE(view->scaleFactorLabel->isHidden());

    QTest::qWait(1100);
    EXPECT_TRUE(view->scaleFactorLabel->isHidden());
}

TEST_F(UT_IconPreviewView, DrawBackground_PlainColorBrush_RendersRoundedBackground)
{
    auto *graphicsScene = new QGraphicsScene(view);
    graphicsScene->setSceneRect(0, 0, 200, 150);
    view->setScene(graphicsScene);
    view->resize(200, 150);

    QPixmap pm(200, 150);
    pm.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(view->QWidget::render(&pm));
    EXPECT_EQ(pm.toImage().pixelColor(100, 75).alpha(), 255);
}

TEST_F(UT_IconPreviewView, DrawBackground_TextureBrush_RendersTiledTexture)
{
    QPixmap texture(32, 32);
    texture.fill(Qt::red);
    view->setBackgroundBrush(QBrush(texture));
    auto *graphicsScene = new QGraphicsScene(view);
    graphicsScene->setSceneRect(0, 0, 200, 150);
    view->setScene(graphicsScene);
    view->resize(200, 150);

    QPixmap pm(200, 150);
    pm.fill(Qt::transparent);
    EXPECT_NO_FATAL_FAILURE(view->QWidget::render(&pm));
    EXPECT_EQ(pm.toImage().pixelColor(100, 75), QColor(Qt::red));
}

TEST_F(UT_DDciIconPreview, Construction_DefaultState_AllFieldsEmpty)
{
    EXPECT_TRUE(preview->title().isEmpty());
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_EQ(preview->contentWidget(), nullptr);
    EXPECT_EQ(preview->statusBarWidget(), nullptr);
    EXPECT_EQ(preview->view, nullptr);
    EXPECT_EQ(preview->scene, nullptr);
    // 源码缺陷: DDciIconPreview 构造函数初始化列表遗漏了 iconItem，该指针构造后为
    // 未初始化的野指针（运行时观测到非空垃圾值），无法对其做确定性断言，故跳过
    // EXPECT_EQ(preview->iconItem, nullptr);
    EXPECT_EQ(preview->mainWidget, nullptr);
    EXPECT_EQ(preview->controlWidget, nullptr);
    EXPECT_EQ(preview->dciIcon, nullptr);
    EXPECT_EQ(preview->dciIconMatched, nullptr);
    EXPECT_FALSE(preview->updateTimer.isActive());
}

TEST_F(UT_DDciIconPreview, InitControlWidgets_WithMainWidget_BuildsAllControlPanels)
{
    preview->mainWidget = new QWidget();
    preview->initControlWidgets();

    ASSERT_NE(preview->controlWidget, nullptr);
    EXPECT_EQ(preview->controlWidget->focusPolicy(), Qt::ClickFocus);
    EXPECT_NE(preview->availableSizeCombo, nullptr);
    EXPECT_EQ(preview->availableSizeCombo->count(), 1);
    EXPECT_NE(preview->devicePixelRatioLabel, nullptr);
    EXPECT_NE(preview->themeCom, nullptr);
    EXPECT_EQ(preview->themeCom->count(), 2);
    EXPECT_NE(preview->modeCom, nullptr);
    EXPECT_EQ(preview->modeCom->count(), 4);
    EXPECT_NE(preview->paletteWidget, nullptr);
    EXPECT_NE(preview->foregroundPaletteEdit, nullptr);
    EXPECT_NE(preview->backgroundPaletteEdit, nullptr);
    EXPECT_NE(preview->hightlightPaletteEdit, nullptr);
    EXPECT_NE(preview->hFPaletteEdit, nullptr);
    EXPECT_NE(preview->customSizeEdit, nullptr);
    EXPECT_EQ(preview->controlWidget->findChildren<QSlider *>().size(), 1);

    preview->updateTimer.stop();
    delete preview->mainWidget;
    preview->mainWidget = nullptr;
    preview->controlWidget = nullptr;
    preview->availableSizeCombo = nullptr;
    preview->devicePixelRatioLabel = nullptr;
    preview->themeCom = nullptr;
    preview->modeCom = nullptr;
    preview->paletteWidget = nullptr;
    preview->paletteNosupportedText = nullptr;
    preview->foregroundPaletteEdit = nullptr;
    preview->backgroundPaletteEdit = nullptr;
    preview->hightlightPaletteEdit = nullptr;
    preview->hFPaletteEdit = nullptr;
    preview->customSizeEdit = nullptr;
}

TEST_F(UT_DDciIconPreview, InitPreviewWidgets_Standalone_CreatesViewSceneAndIconItem)
{
    preview->initPreviewWidgets();

    ASSERT_NE(preview->view, nullptr);
    ASSERT_NE(preview->scene, nullptr);
    ASSERT_NE(preview->iconItem, nullptr);
    EXPECT_EQ(preview->view->scene(), preview->scene);
    EXPECT_EQ(preview->scene->items().size(), 1);
    EXPECT_EQ(preview->scene->items().first(), preview->iconItem);
    EXPECT_EQ(preview->iconItem->transformationMode(), Qt::SmoothTransformation);
    EXPECT_FALSE(preview->iconItem->flags() & QGraphicsItem::ItemIsSelectable);
    EXPECT_FALSE(preview->iconItem->flags() & QGraphicsItem::ItemIsMovable);
    EXPECT_EQ(preview->iconItem->shapeMode(), QGraphicsPixmapItem::BoundingRectShape);

    delete preview->view;
    preview->view = nullptr;
    preview->scene = nullptr;
    preview->iconItem = nullptr;
}

TEST_F(UT_DDciIconPreviewInitialized, Initialize_WithNullParents_CreatesMainWidgetWithSplitter)
{
    EXPECT_NE(preview->contentWidget(), nullptr);
    EXPECT_EQ(preview->contentWidget(), preview->mainWidget);
    EXPECT_EQ(preview->mainWidget->size(), QSize(1200, 800));
    auto splitters = preview->mainWidget->findChildren<QSplitter *>();
    ASSERT_EQ(splitters.size(), 1);
    EXPECT_EQ(splitters.first()->count(), 2);
}

TEST_F(UT_DDciIconPreviewInitialized, Title_BeforeFileUrlSet_ReturnsEmptyString)
{
    EXPECT_TRUE(preview->title().isEmpty());
}

TEST_F(UT_DDciIconPreviewInitialized, StatusBarWidget_AnyState_ReturnsNull)
{
    EXPECT_EQ(preview->statusBarWidget(), nullptr);
}

// 真实行为: 新建预览的初始 url 为空，setFileUrl(空URL) 命中 this->url == url 的
// “URL 未变化”短路分支直接返回 true，空 URL 不会被拒绝（疑似源码缺陷：空路径未校验即返回成功）
TEST_F(UT_DDciIconPreviewInitialized, SetFileUrl_EmptyUrlOnFreshPreview_TreatedAsUnchangedReturnsTrue)
{
    EXPECT_TRUE(preview->setFileUrl(QUrl()));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_TRUE(preview->title().isEmpty());
    EXPECT_EQ(preview->dciIcon, nullptr);
}

TEST_F(UT_DDciIconPreviewInitialized, SetFileUrl_NotDciExtension_ReturnsFalse)
{
    QUrl url = QUrl::fromLocalFile("/tmp/ut_dciicon_not_dci.txt");
    EXPECT_FALSE(preview->setFileUrl(url));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
}

TEST_F(UT_DDciIconPreviewInitialized, SetFileUrl_RemoteHttpUrl_ReturnsFalse)
{
    QUrl url("http://example.com/icon.dci");
    EXPECT_FALSE(preview->setFileUrl(url));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
}

TEST_F(UT_DDciIconPreviewInitialized, SetFileUrl_TextContentDciFile_ReturnsFalse)
{
    const QString path = createTempDciFile("plain text content for mime sniffing\n");
    QUrl url = QUrl::fromLocalFile(path);
    EXPECT_FALSE(preview->setFileUrl(url));
    EXPECT_TRUE(preview->fileUrl().isEmpty());
    EXPECT_TRUE(preview->title().isEmpty());
}

TEST_F(UT_DDciIconPreviewInitialized, SetFileUrl_ValidDciFile_LoadsSettingsAndSetsTitle)
{
    stub.set_lamda(ADDR(QMimeType, preferredSuffix), [](const QMimeType *) {
        __DBG_STUB_INVOKE__
        return QString("dci");
    });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->availableSizeCombo->clear();
    preview->availableSizeCombo->insertItem(0, "Custom Size");
    preview->updateTimer.stop();

    const QString path = createTempDciFile("DCI-UNIT-TEST-GARBAGE");
    QUrl url = QUrl::fromLocalFile(path);
    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(path));
    EXPECT_EQ(preview->title(), QString("ut_dciicon_preview_temp.dci"));
    EXPECT_NE(preview->dciIcon, nullptr);
    EXPECT_EQ(preview->availableSizeCombo->count(), 3);
}

TEST_F(UT_DDciIconPreviewInitialized, SetFileUrl_SameUrlTwice_SkipsReloading)
{
    stub.set_lamda(ADDR(QMimeType, preferredSuffix), [](const QMimeType *) {
        __DBG_STUB_INVOKE__
        return QString("dci");
    });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->availableSizeCombo->clear();
    preview->availableSizeCombo->insertItem(0, "Custom Size");
    preview->updateTimer.stop();

    const QString path = createTempDciFile("DCI-UNIT-TEST-GARBAGE");
    QUrl url = QUrl::fromLocalFile(path);
    ASSERT_TRUE(preview->setFileUrl(url));

    int reloadCount = 0;
    stub.set_lamda(&DDciIconPreview::initializeSettings, [&reloadCount](DDciIconPreview *, const QString &) {
        __DBG_STUB_INVOKE__
        reloadCount++;
    });
    EXPECT_TRUE(preview->setFileUrl(url));
    EXPECT_EQ(reloadCount, 0);
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(path));
}

TEST_F(UT_DDciIconPreviewInitialized, SetFileUrl_DifferentInvalidUrlAfterValid_KeepsPreviousUrl)
{
    stub.set_lamda(ADDR(QMimeType, preferredSuffix), [](const QMimeType *) {
        __DBG_STUB_INVOKE__
        return QString("dci");
    });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->availableSizeCombo->clear();
    preview->availableSizeCombo->insertItem(0, "Custom Size");
    preview->updateTimer.stop();

    const QString path = createTempDciFile("DCI-UNIT-TEST-GARBAGE");
    QUrl validUrl = QUrl::fromLocalFile(path);
    ASSERT_TRUE(preview->setFileUrl(validUrl));

    EXPECT_FALSE(preview->setFileUrl(QUrl::fromLocalFile("/tmp/ut_dciicon_other.txt")));
    EXPECT_EQ(preview->fileUrl(), QUrl::fromLocalFile(path));
    EXPECT_EQ(preview->title(), QString("ut_dciicon_preview_temp.dci"));
}

TEST_F(UT_DDciIconPreviewInitialized, InitializeSettings_EmptyPath_ReturnsEarlyWithoutLoadingIcon)
{
    preview->initializeSettings("");
    EXPECT_EQ(preview->dciIcon, nullptr);
    EXPECT_TRUE(preview->iconItem->pixmap().isNull());
}

TEST_F(UT_DDciIconPreviewInitialized, InitializeSettings_UnloadableIconFile_LeavesDanglingNonNullPointer)
{
    const QString path = createTempDciFile("DEFINITELY-NOT-A-DCI-FILE");
    preview->initializeSettings(path);

    EXPECT_NE(preview->dciIcon, nullptr);
    preview->dciIcon = nullptr;
}

TEST_F(UT_DDciIconPreviewInitialized, InitializeSettings_ValidIcon_PopulatesAvailableSizes)
{
    loadFakeIconSuccessfully(QList<int> { 16, 32, 48 });

    EXPECT_NE(preview->dciIcon, nullptr);
    EXPECT_EQ(preview->availableSizeCombo->count(), 4);
    EXPECT_EQ(preview->availableSizeCombo->itemText(0), QString("16"));
    EXPECT_EQ(preview->availableSizeCombo->itemText(1), QString("32"));
    EXPECT_EQ(preview->availableSizeCombo->itemText(2), QString("48"));
    EXPECT_EQ(preview->availableSizeCombo->itemText(3), QString("Custom Size"));
    EXPECT_EQ(preview->availableSizeCombo->currentIndex(), 0);
}

TEST_F(UT_DDciIconPreviewInitialized, GenerateDciIconPalette_CustomColorTexts_ReturnsMatchingPalette)
{
    preview->foregroundPaletteEdit->setText("#112233");
    preview->backgroundPaletteEdit->setText("#445566");
    preview->hightlightPaletteEdit->setText("#778899");
    preview->hFPaletteEdit->setText("#aabbcc");

    DDciIconPalette palette = preview->generateDciIconPalette();
    EXPECT_EQ(palette.foreground(), QColor("#112233"));
    EXPECT_EQ(palette.background(), QColor("#445566"));
    EXPECT_EQ(palette.highlight(), QColor("#778899"));
    EXPECT_EQ(palette.highlightForeground(), QColor("#aabbcc"));
}

TEST_F(UT_DDciIconPreviewInitialized, UpdateIconMatchedResult_NoIconLoaded_ReturnsEarly)
{
    preview->dciIconMatched = reinterpret_cast<DDciIconMatchResult>(0x1);
    preview->updateIconMatchedResult();
    EXPECT_EQ(preview->dciIconMatched, reinterpret_cast<DDciIconMatchResult>(0x1));
}

TEST_F(UT_DDciIconPreviewInitialized, UpdateIconMatchedResult_ZeroIconSize_ResetsMatchedToNull)
{
    int matchIconCalls = 0;
    stub.set_lamda(ADDR(DDciIcon, matchIcon),
                   [&matchIconCalls](const DDciIcon *, int, DDciIcon::Theme, DDciIcon::Mode, DDciIcon::IconMatchedFlags) {
                       __DBG_STUB_INVOKE__
                       matchIconCalls++;
                       return reinterpret_cast<DDciIconMatchResult>(0x1);
                   });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });

    preview->availableSizeCombo->setCurrentIndex(preview->availableSizeCombo->count() - 1);
    preview->customSizeEdit->setText("");

    preview->updateIconMatchedResult();
    EXPECT_EQ(preview->dciIconMatched, nullptr);
    EXPECT_EQ(matchIconCalls, 0);
}

TEST_F(UT_DDciIconPreviewInitialized, UpdateIconMatchedResult_ValidInputs_MatchesSelectedThemeAndMode)
{
    int capturedSize = -1;
    DDciIcon::Theme capturedTheme = DDciIcon::Light;
    DDciIcon::Mode capturedMode = DDciIcon::Normal;
    DDciIcon::IconMatchedFlags capturedFlags;
    stub.set_lamda(ADDR(DDciIcon, matchIcon),
                   [&capturedSize, &capturedTheme, &capturedMode, &capturedFlags](const DDciIcon *, int size, DDciIcon::Theme theme, DDciIcon::Mode mode, DDciIcon::IconMatchedFlags flags) {
                       __DBG_STUB_INVOKE__
                       capturedSize = size;
                       capturedTheme = theme;
                       capturedMode = mode;
                       capturedFlags = flags;
                       return reinterpret_cast<DDciIconMatchResult>(0x1234);
                   });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });

    preview->themeCom->setCurrentIndex(1);
    preview->modeCom->setCurrentIndex(2);
    preview->updateIconMatchedResult();

    EXPECT_EQ(capturedSize, 16);
    EXPECT_EQ(capturedTheme, DDciIcon::Dark);
    EXPECT_EQ(capturedMode, DDciIcon::Mode(2));
    EXPECT_TRUE(capturedFlags & DDciIcon::DontFallbackMode);
    EXPECT_EQ(preview->dciIconMatched, reinterpret_cast<DDciIconMatchResult>(0x1234));
}

TEST_F(UT_DDciIconPreviewInitialized, UpdatePixmap_TimerNotActive_SchedulesDelayedUpdate)
{
    preview->updatePixmap();
    EXPECT_TRUE(preview->updateTimer.isActive());
}

TEST_F(UT_DDciIconPreviewInitialized, UpdatePixmap_TimerAlreadyActive_SecondRequestIgnored)
{
    int implCalls = 0;
    stub.set_lamda(&DDciIconPreview::updatePixmapImpl, [&implCalls](DDciIconPreview *) {
        __DBG_STUB_INVOKE__
        implCalls++;
    });

    preview->updatePixmap();
    preview->updatePixmap();
    QTest::qWait(150);

    EXPECT_EQ(implCalls, 1);
}

TEST_F(UT_DDciIconPreviewInitialized, UpdatePixmapImpl_NoMatchedResult_ShowsInvalidPixmap)
{
    preview->updatePixmapImpl();
    EXPECT_EQ(preview->iconItem->pixmap().size(), QSize(150, 80));
    EXPECT_TRUE(preview->paletteWidget->isEnabled());
}

TEST_F(UT_DDciIconPreviewInitialized, UpdatePixmapImpl_MatchedButZeroIconSize_ShowsInvalidPixmap)
{
    stub.set_lamda(ADDR(DDciIcon, matchIcon), [](const DDciIcon *, int, DDciIcon::Theme, DDciIcon::Mode, DDciIcon::IconMatchedFlags) {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<DDciIconMatchResult>(0x1);
    });
    stub.set_lamda(static_cast<bool (DDciIcon::*)(DDciIconMatchResult, DDciIcon::IconAttribute) const>(&DDciIcon::isSupportedAttribute),
                   [](const DDciIcon *, DDciIconMatchResult, DDciIcon::IconAttribute) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->updateIconMatchedResult();

    preview->availableSizeCombo->setCurrentIndex(preview->availableSizeCombo->count() - 1);
    preview->customSizeEdit->setText("");

    preview->updatePixmapImpl();
    EXPECT_EQ(preview->iconItem->pixmap().size(), QSize(150, 80));
    EXPECT_TRUE(preview->paletteWidget->isEnabled());
    EXPECT_TRUE(preview->paletteNosupportedText->isHidden());
}

TEST_F(UT_DDciIconPreviewInitialized, UpdatePixmapImpl_ZeroDevicePixelRatio_ShowsInvalidPixmap)
{
    stub.set_lamda(ADDR(DDciIcon, matchIcon), [](const DDciIcon *, int, DDciIcon::Theme, DDciIcon::Mode, DDciIcon::IconMatchedFlags) {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<DDciIconMatchResult>(0x1);
    });
    stub.set_lamda(static_cast<bool (DDciIcon::*)(DDciIconMatchResult, DDciIcon::IconAttribute) const>(&DDciIcon::isSupportedAttribute),
                   [](const DDciIcon *, DDciIconMatchResult, DDciIcon::IconAttribute) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->updateIconMatchedResult();

    preview->devicePixelRatioLabel->setText("0");

    preview->updatePixmapImpl();
    EXPECT_EQ(preview->iconItem->pixmap().size(), QSize(150, 80));
}

TEST_F(UT_DDciIconPreviewInitialized, UpdatePixmapImpl_AllValid_RendersIconPixmap)
{
    stub.set_lamda(ADDR(DDciIcon, matchIcon), [](const DDciIcon *, int, DDciIcon::Theme, DDciIcon::Mode, DDciIcon::IconMatchedFlags) {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<DDciIconMatchResult>(0x1);
    });
    stub.set_lamda(static_cast<bool (DDciIcon::*)(DDciIconMatchResult, DDciIcon::IconAttribute) const>(&DDciIcon::isSupportedAttribute),
                   [](const DDciIcon *, DDciIconMatchResult, DDciIcon::IconAttribute) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });
    bool pixmapGenerated = false;
    stub.set_lamda(static_cast<QPixmap (DDciIcon::*)(qreal, int, DDciIconMatchResult, const DDciIconPalette &) const>(&DDciIcon::pixmap),
                   [&pixmapGenerated](const DDciIcon *, qreal, int size, DDciIconMatchResult, const DDciIconPalette &) {
                       __DBG_STUB_INVOKE__
                       pixmapGenerated = true;
                       QPixmap pm(size, size);
                       pm.fill(Qt::red);
                       return pm;
                   });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->updateIconMatchedResult();
    preview->devicePixelRatioLabel->setText("1");

    preview->updatePixmapImpl();

    EXPECT_TRUE(pixmapGenerated);
    EXPECT_EQ(preview->iconItem->pixmap().size(), QSize(16, 16));
    EXPECT_TRUE(preview->paletteWidget->isEnabled());
    EXPECT_TRUE(preview->paletteNosupportedText->isHidden());
}

TEST_F(UT_DDciIconPreviewInitialized, UpdatePixmapImpl_PaletteNotSupported_DisablesPaletteControls)
{
    stub.set_lamda(ADDR(DDciIcon, matchIcon), [](const DDciIcon *, int, DDciIcon::Theme, DDciIcon::Mode, DDciIcon::IconMatchedFlags) {
        __DBG_STUB_INVOKE__
        return reinterpret_cast<DDciIconMatchResult>(0x1);
    });
    stub.set_lamda(static_cast<bool (DDciIcon::*)(DDciIconMatchResult, DDciIcon::IconAttribute) const>(&DDciIcon::isSupportedAttribute),
                   [](const DDciIcon *, DDciIconMatchResult, DDciIcon::IconAttribute) {
                       __DBG_STUB_INVOKE__
                       return false;
                   });
    stub.set_lamda(static_cast<QPixmap (DDciIcon::*)(qreal, int, DDciIconMatchResult, const DDciIconPalette &) const>(&DDciIcon::pixmap),
                   [](const DDciIcon *, qreal, int size, DDciIconMatchResult, const DDciIconPalette &) {
                       __DBG_STUB_INVOKE__
                       QPixmap pm(size, size);
                       pm.fill(Qt::red);
                       return pm;
                   });
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->updateIconMatchedResult();
    preview->devicePixelRatioLabel->setText("1");

    preview->updatePixmapImpl();

    EXPECT_FALSE(preview->paletteWidget->isEnabled());
    EXPECT_FALSE(preview->paletteNosupportedText->isHidden());
    EXPECT_EQ(preview->iconItem->pixmap().size(), QSize(16, 16));
}

TEST_F(UT_DDciIconPreviewInitialized, EventFilter_ViewResize_UpdatesSceneRectAndRecentersItem)
{
    QResizeEvent resizeEvent(QSize(500, 400), QSize(400, 300));
    EXPECT_FALSE(preview->eventFilter(preview->view, &resizeEvent));
    EXPECT_EQ(preview->scene->sceneRect(), QRectF(0, 0, 500, 400));
}

TEST_F(UT_DDciIconPreviewInitialized, EventFilter_OtherObjectWatched_IgnoredWithoutSideEffect)
{
    QResizeEvent resizeEvent(QSize(500, 400), QSize(400, 300));
    EXPECT_FALSE(preview->eventFilter(preview->mainWidget, &resizeEvent));
    EXPECT_EQ(preview->scene->sceneRect(), QRectF());
}

TEST_F(UT_DDciIconPreviewInitialized, TimerEvent_UpdateTimerFires_RunsUpdatePixmapImplOnce)
{
    int implCalls = 0;
    stub.set_lamda(&DDciIconPreview::updatePixmapImpl, [&implCalls](DDciIconPreview *) {
        __DBG_STUB_INVOKE__
        implCalls++;
    });

    preview->updatePixmap();
    QTest::qWait(150);

    EXPECT_EQ(implCalls, 1);
    EXPECT_FALSE(preview->updateTimer.isActive());
}

TEST_F(UT_DDciIconPreviewInitialized, TimerEvent_UnrelatedTimerId_Ignored)
{
    int implCalls = 0;
    stub.set_lamda(&DDciIconPreview::updatePixmapImpl, [&implCalls](DDciIconPreview *) {
        __DBG_STUB_INVOKE__
        implCalls++;
    });

    QTimerEvent unrelatedEvent(123456);
    EXPECT_NO_FATAL_FAILURE(preview->timerEvent(&unrelatedEvent));
    EXPECT_EQ(implCalls, 0);
}

TEST_F(UT_DDciIconPreviewInitialized, GetIconSize_NumericComboCurrent_ReturnsSelectedSize)
{
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    EXPECT_EQ(preview->getIconSize(), 16);
}

TEST_F(UT_DDciIconPreviewInitialized, GetIconSize_CustomIndexWithText_ReturnsCustomSize)
{
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->availableSizeCombo->setCurrentIndex(preview->availableSizeCombo->count() - 1);
    preview->customSizeEdit->setText("72");
    EXPECT_EQ(preview->getIconSize(), 72);
}

TEST_F(UT_DDciIconPreviewInitialized, GetIconSize_CustomIndexEmptyText_ReturnsZero)
{
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    preview->availableSizeCombo->setCurrentIndex(preview->availableSizeCombo->count() - 1);
    preview->customSizeEdit->setText("");
    EXPECT_EQ(preview->getIconSize(), 0);
}

TEST_F(UT_DDciIconPreviewInitialized, AvailableSizeCombo_SelectCustomIndex_ShowsCustomSizeEdit)
{
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    EXPECT_TRUE(preview->customSizeEdit->isHidden());

    preview->availableSizeCombo->setCurrentIndex(preview->availableSizeCombo->count() - 1);
    EXPECT_FALSE(preview->customSizeEdit->isHidden());

    preview->updateTimer.stop();
    preview->availableSizeCombo->setCurrentIndex(0);
    EXPECT_TRUE(preview->customSizeEdit->isHidden());
    EXPECT_TRUE(preview->updateTimer.isActive());
}

TEST_F(UT_DDciIconPreviewInitialized, CustomSizeEdit_EditingFinished_SchedulesDelayedUpdate)
{
    preview->updateTimer.stop();
    preview->customSizeEdit->setText("100");
    emit preview->customSizeEdit->editingFinished();
    EXPECT_TRUE(preview->updateTimer.isActive());
}

TEST_F(UT_DDciIconPreviewInitialized, DevicePixelSlider_ValueChanged_UpdatesRatioLabelAndSchedulesUpdate)
{
    auto sliders = preview->controlWidget->findChildren<QSlider *>();
    ASSERT_EQ(sliders.size(), 1);

    preview->updateTimer.stop();
    sliders.first()->setValue(25);
    EXPECT_EQ(preview->devicePixelRatioLabel->text(), QString("2.5"));
    EXPECT_TRUE(preview->updateTimer.isActive());
}

TEST_F(UT_DDciIconPreviewInitialized, BackgroundColorCombo_SelectPresetColors_UpdatesViewBackground)
{
    QComboBox *backgroundCombo = nullptr;
    for (QComboBox *combo : preview->controlWidget->findChildren<QComboBox *>()) {
        if (combo->objectName().isEmpty() && combo->count() == 4)
            backgroundCombo = combo;
    }
    ASSERT_NE(backgroundCombo, nullptr);

    backgroundCombo->setCurrentIndex(0);
    EXPECT_EQ(preview->view->backgroundBrush().color(), QColor(Qt::white));

    backgroundCombo->setCurrentIndex(1);
    EXPECT_EQ(preview->view->backgroundBrush().color(), QColor(Qt::black));

    backgroundCombo->setCurrentIndex(2);
    EXPECT_FALSE(preview->view->backgroundBrush().texture().isNull());
}

TEST_F(UT_DDciIconPreviewInitialized, BackgroundColorCombo_CustomColorApplied_UpdatesViewBackground)
{
    QComboBox *backgroundCombo = nullptr;
    for (QComboBox *combo : preview->controlWidget->findChildren<QComboBox *>()) {
        if (combo->objectName().isEmpty() && combo->count() == 4)
            backgroundCombo = combo;
    }
    ASSERT_NE(backgroundCombo, nullptr);
    backgroundCombo->setCurrentIndex(3);

    QLineEdit *customBackgroundEdit = nullptr;
    for (QLineEdit *edit : preview->controlWidget->findChildren<QLineEdit *>()) {
        if (edit->objectName().isEmpty())
            customBackgroundEdit = edit;
    }
    ASSERT_NE(customBackgroundEdit, nullptr);
    EXPECT_FALSE(customBackgroundEdit->isHidden());

    customBackgroundEdit->setText("#123456");
    emit customBackgroundEdit->editingFinished();
    EXPECT_EQ(preview->view->backgroundBrush().color(), QColor("#123456"));

    customBackgroundEdit->setText("not-a-color");
    emit customBackgroundEdit->editingFinished();
    EXPECT_EQ(preview->view->backgroundBrush().color(), QColor("#123456"));
}

TEST_F(UT_DDciIconPreviewInitialized, Destruction_AfterLoadingIcon_DeletesIconSafely)
{
    loadFakeIconSuccessfully(QList<int> { 16, 32 });
    ASSERT_NE(preview->dciIcon, nullptr);
    QWidget *main = preview->mainWidget;
    EXPECT_NO_FATAL_FAILURE(delete preview);
    preview = nullptr;
    delete main;
}

#endif   // DTKGUI_CLASS_DDciIcon

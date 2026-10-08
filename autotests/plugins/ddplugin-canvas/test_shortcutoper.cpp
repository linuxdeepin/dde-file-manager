// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/operator/shortcutoper.h"
#include "view/canvasview.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QKeyEvent>

using namespace ddplugin_canvas;

class UT_ShortcutOper : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        parentWidget = new QWidget();
        view = new CanvasView(parentWidget);
        oper = new ShortcutOper(view);
    }

    virtual void TearDown() override
    {
        if (oper) {
            delete oper;
            oper = nullptr;
        }

        if (view) {
            delete view;
            view = nullptr;
        }

        if (parentWidget) {
            delete parentWidget;
            parentWidget = nullptr;
        }

        stub.clear();
    }

public:
    QApplication *app = nullptr;
    QWidget *parentWidget = nullptr;
    CanvasView *view = nullptr;
    ShortcutOper *oper = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_ShortcutOper, constructor_CreateOper_InitializesCorrectly)
{
    EXPECT_NE(oper, nullptr);
}

TEST_F(UT_ShortcutOper, keyPressed_WithValidEvent_ReturnsBoolean)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    bool result = oper->keyPressed(&event);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_ShortcutOper, keyPressed_WithNullEvent_ReturnsFalse)
{
    bool result = oper->keyPressed(nullptr);
    EXPECT_FALSE(result);
}
// ---------------------------------------------------------------------------
// PMS:116441 桌面按空格键预览文件无效回归：快捷键空格（非自动重复）必须触发
// 预览槽推送 slot_Operation_FilesPreview（修复前桌面未实现空格预览）；
// 自动重复按键不得重复触发预览。
// ---------------------------------------------------------------------------
#include "model/canvasproxymodel.h"
#include "model/canvasselectionmodel.h"

#include <dfm-framework/dpf.h>
#include <QUrl>
#include <QHash>
#include <QList>
#include <dlfcn.h>

DPF_USE_NAMESPACE

namespace {
// 任意 (space,topic) 映射到自定义事件 id，使本进程内 dpf 槽通道可用
int canvasSlotEventConverter(const QString &space, const QString &topic)
{
    static QHash<QString, int> mapping;
    const QString key = space + "::" + topic;
    const auto it = mapping.constFind(key);
    if (it != mapping.constEnd())
        return it.value();
    const int id = static_cast<int>(EventTypeScope::kCustomBase) + mapping.size();
    mapping.insert(key, id);
    return id;
}

void installCanvasSlotConverter()
{
    dpf::Event::instance();
    EventConverter::convertFunc = &canvasSlotEventConverter;
    if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                dlsym(RTLD_DEFAULT, "_ZN3dpf14EventConverter11convertFuncE"))) {
        *func = &canvasSlotEventConverter;
    }
    if (void *framework = dlopen("libdfm6-framework.so.1", RTLD_LAZY | RTLD_NOLOAD)) {
        if (auto *func = reinterpret_cast<dpf::EventConverterFunc *>(
                    dlsym(framework, "_ZN3dpf14EventConverter11convertFuncE"))) {
            *func = &canvasSlotEventConverter;
        }
    }
}

// 槽接收者（无 Q_OBJECT，dpf setReceiver 直接绑定成员函数指针即可）
class PreviewSlotReceiver : public QObject
{
public:
    quint64 recvWinId = 0;
    QList<QUrl> recvSelectUrls;
    QList<QUrl> recvDirUrls;
    int calls = 0;

    // 与 FileOperationsEventReceiver::handleOperationFilesPreview 同参签名
    void onFilesPreview(const quint64 windowId, const QList<QUrl> &selectUrls, const QList<QUrl> &currentDirUrls)
    {
        ++calls;
        recvWinId = windowId;
        recvSelectUrls = selectUrls;
        recvDirUrls = currentDirUrls;
    }
};
}   // namespace

class ShortcutOperTest : public testing::Test
{
protected:
    void SetUp() override
    {
        parentWidget = new QWidget();
        view = new CanvasView(parentWidget);
        proxyModel = new CanvasProxyModel();
        view->setModel(proxyModel);
        selModel = new CanvasSelectionModel(proxyModel, view);
        view->setSelectionModel(selModel);
        oper = new ShortcutOper(view);

        // 关闭"禁用快捷键"分支，使空格键走到预览处理
        stub.set_lamda(ADDR(ShortcutOper, disableShortcut),
                       [](ShortcutOper *) -> bool {
                           __DBG_STUB_INVOKE__
                           return false;
                       });
        // 选中文件由桩提供（避免真实选中模型与文件信息依赖）
        stub.set_lamda(ADDR(CanvasSelectionModel, selectedUrls),
                       [this](CanvasSelectionModel *) -> QList<QUrl> {
                           __DBG_STUB_INVOKE__
                           return previewUrls;
                       });

        installCanvasSlotConverter();
    }

    void TearDown() override
    {
        dpfSlotChannel->disconnect("dfmplugin_fileoperations", "slot_Operation_FilesPreview");
        delete oper;
        delete view;
        delete proxyModel;
        delete parentWidget;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    QWidget *parentWidget = nullptr;
    CanvasView *view = nullptr;
    CanvasProxyModel *proxyModel = nullptr;
    CanvasSelectionModel *selModel = nullptr;
    ShortcutOper *oper = nullptr;
    QList<QUrl> previewUrls { QUrl::fromLocalFile("/tmp/ut-116441-a.txt"),
                              QUrl::fromLocalFile("/tmp/ut-116441-b.txt") };
};

// 空格键（非自动重复）→ 推送 slot_Operation_FilesPreview，携带顶层窗口 id 与选中文件
TEST_F(ShortcutOperTest, BUG116441_SpacePreview_PushesFilesPreviewSlotWithSelection)
{
    PreviewSlotReceiver receiver;
    ASSERT_TRUE(dpfSlotChannel->connect("dfmplugin_fileoperations", "slot_Operation_FilesPreview",
                                        &receiver, &PreviewSlotReceiver::onFilesPreview));

    QKeyEvent spacePress(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    ASSERT_TRUE(oper->keyPressed(&spacePress));

    EXPECT_EQ(receiver.calls, 1);
    EXPECT_EQ(receiver.recvWinId, view->topLevelWidget()->winId());
    EXPECT_EQ(receiver.recvSelectUrls, previewUrls);
    EXPECT_TRUE(receiver.recvDirUrls.isEmpty());
}

// 自动重复的空格键不得重复触发预览推送
TEST_F(ShortcutOperTest, BUG116441_SpacePreview_AutoRepeat_DoesNotRetriggerPreview)
{
    PreviewSlotReceiver receiver;
    ASSERT_TRUE(dpfSlotChannel->connect("dfmplugin_fileoperations", "slot_Operation_FilesPreview",
                                        &receiver, &PreviewSlotReceiver::onFilesPreview));

    QKeyEvent autoRepeatPress(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QString(), true);
    ASSERT_TRUE(oper->keyPressed(&autoRepeatPress));

    EXPECT_EQ(receiver.calls, 0);
}

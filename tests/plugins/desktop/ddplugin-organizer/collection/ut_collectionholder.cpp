// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/collectionmodel_p.h"
#include "view/collectionframe_p.h"
#include "collection/collectionholder_p.h"
#include "collection/collectionholder.h"
#include "view/collectionwidget_p.h"
#include "view/collectionview_p.h"
#include "interface/canvasmanagershell.h"
#include "mode/normalized/fileclassifier.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"
#include"view/collectiontitlebar_p.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

TEST(CollectionHolder, construct)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    EXPECT_EQ(holder.id(), QString("testuuid"));
    EXPECT_TRUE(holder.d->styleTimer.isSingleShot());

    QString id;
    QObject::connect(&holder, &CollectionHolder::styleChanged, &holder,
                     [&id](const QString &in){
        id = in;
    });

    emit holder.d->styleTimer.timeout({});
    EXPECT_EQ(id, holder.id());
}

TEST(CollectionHolder, onAdjustFrameSizeMode)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    QString id;
    QObject::connect(&holder, &CollectionHolder::styleChanged, &holder,
                     [&id](const QString &in){
        id = in;
    });

    ASSERT_EQ(holder.d->sizeMode, CollectionFrameSize::kSmall);
    holder.d->onAdjustFrameSizeMode(CollectionFrameSize::kLarge);

    EXPECT_EQ(holder.d->sizeMode, CollectionFrameSize::kLarge);
    EXPECT_EQ(id, holder.id());
}

TEST(CollectionHolder, setSurface)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    EXPECT_EQ(holder.surface(), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;

    Surface sur;
    holder.setSurface(&sur);
    EXPECT_EQ(holder.surface(), &sur);
    EXPECT_EQ(frame->parent(), &sur);
}

TEST(CollectionHolder, setShell)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    CollectionWidget w(QString("testuuid"), nullptr);
    holder.d->widget = &w;
    CollectionView view("", nullptr);
    w.d->view = &view;

    {
        CanvasManagerShell shell;
        holder.setCanvasManagerShell(&shell);
        EXPECT_EQ(view.d->canvasManagerShell, &shell);
    }

    {
        CanvasViewShell shell;
        holder.setCanvasViewShell(&shell);
        EXPECT_EQ(view.d->canvasViewShell, &shell);
    }

    {
        CanvasGridShell shell;
        holder.setCanvasGridShell(&shell);
        EXPECT_EQ(view.d->canvasGridShell, &shell);
    }

    {
        CanvasModelShell shell;
        holder.setCanvasModelShell(&shell);
        EXPECT_EQ(view.d->canvasModelShell, &shell);
    }
}

TEST(CollectionHolder, widget)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    EXPECT_EQ(holder.widget(), nullptr);

    CollectionWidget w(QString("testuuid"), nullptr);
    holder.d->widget = &w;

    EXPECT_EQ(holder.widget(), &w);

    holder.setName("test");
    EXPECT_EQ(holder.name(), QString("test"));
}

TEST(CollectionHolder, createFrame)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    CollectionModel model;
    Surface sur;

    holder.createFrame(&sur, &model);
    ASSERT_NE(holder.d->frame, nullptr);
    EXPECT_EQ(holder.d->frame->parent(), &sur);
    EXPECT_EQ(holder.d->model, &model);

    ASSERT_NE(holder.d->widget, nullptr);
    EXPECT_EQ(holder.d->widget->geometry(), QRect( QPoint(0,0),holder.d->frame->size()));
    EXPECT_EQ(holder.d->frame->widget(), holder.d->widget);

    QString id;
    QObject::connect(&holder, &CollectionHolder::sigRequestClose, &holder,
                     [&id](const QString &in){
        id = in;
    });
    emit holder.d->widget->sigRequestClose("1");
    EXPECT_EQ(id, QString("1"));

    stub_ext::StubExt stub;
    CollectionFrameSize adjust = kSmall;
    stub.set_lamda(&CollectionHolderPrivate::onAdjustFrameSizeMode,[&adjust]
                   (CollectionHolderPrivate *, const CollectionFrameSize &size){
        adjust = size;
    });

    emit holder.d->widget->sigRequestAdjustSizeMode(CollectionFrameSize::kLarge);
    EXPECT_EQ(adjust, CollectionFrameSize::kLarge);

    ASSERT_FALSE(holder.d->styleTimer.isActive());
    emit holder.d->frame->geometryChanged();
    EXPECT_TRUE(holder.d->styleTimer.isActive());
}

TEST(CollectionHolder, style)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    holder.d->screenIndex = 2;

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->setGeometry(QRect(10,10,20,20));
    holder.d->sizeMode = CollectionFrameSize::kLarge;

    auto style = holder.style();
    EXPECT_EQ(style.key, QString("testuuid"));
    EXPECT_EQ(style.screenIndex, 2);
    EXPECT_EQ(style.rect, QRect(10,10,20,20));
    EXPECT_EQ(style.sizeMode, CollectionFrameSize::kLarge);
}

TEST(CollectionHolder, setStyle)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    holder.d->screenIndex = 2;

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->setGeometry(QRect(10,10,20,20));
    holder.d->sizeMode = CollectionFrameSize::kLarge;

    CollectionWidget w(QString("testuuid"), nullptr);
    holder.d->widget = &w;
    w.d->titleBar->d->size = CollectionFrameSize::kLarge;

    CollectionStyle style;
    style.key = "test";
    style.screenIndex = 1;
    style.sizeMode = CollectionFrameSize::kSmall;
    style.rect = QRect(0,0,10,10);

    holder.setStyle(style);
    EXPECT_EQ(holder.d->screenIndex, 2);
    EXPECT_EQ(holder.d->frame->geometry(), QRect(10,10,20,20));
    EXPECT_EQ(holder.d->sizeMode, CollectionFrameSize::kLarge);
    EXPECT_EQ(holder.d->widget->d->titleBar->d->size, CollectionFrameSize::kLarge);

    style.key = "testuuid";
    holder.setStyle(style);
    EXPECT_EQ(holder.d->screenIndex, 1);
    EXPECT_EQ(holder.d->frame->geometry(), QRect(0,0,10,10));
    EXPECT_EQ(holder.d->sizeMode, CollectionFrameSize::kSmall);
    EXPECT_EQ(holder.d->widget->d->titleBar->d->size, CollectionFrameSize::kSmall);
}

TEST(CollectionHolder, setMovable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;

    holder.setMovable(true);

    EXPECT_TRUE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameMovable));

    holder.setMovable(false);
    EXPECT_FALSE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameMovable));
}

TEST(CollectionHolder, movable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->d->frameFeatures |= CollectionFrame::CollectionFrameMovable;
    EXPECT_TRUE(holder.movable());

    frame->d->frameFeatures &= ~CollectionFrame::CollectionFrameMovable;
    EXPECT_FALSE(holder.movable());
}

TEST(CollectionHolder, setClosable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    CollectionWidget w(QString("testuuid"), nullptr);
    holder.d->widget = &w;

    holder.setClosable(true);

    EXPECT_TRUE(w.d->titleBar->d->closable);
    EXPECT_TRUE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameClosable));

    holder.setClosable(false);
    EXPECT_FALSE(w.d->titleBar->d->closable);
    EXPECT_FALSE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameClosable));
}

TEST(CollectionHolder, closable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->d->frameFeatures |= CollectionFrame::CollectionFrameClosable;
    EXPECT_TRUE(holder.closable());

    frame->d->frameFeatures &= ~CollectionFrame::CollectionFrameClosable;
    EXPECT_FALSE(holder.closable());
}

TEST(CollectionHolder, setFloatable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;

    holder.setFloatable(true);

    EXPECT_TRUE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameFloatable));

    holder.setFloatable(false);
    EXPECT_FALSE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameFloatable));
}

TEST(CollectionHolder, floatable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->d->frameFeatures |= CollectionFrame::CollectionFrameFloatable;
    EXPECT_TRUE(holder.floatable());

    frame->d->frameFeatures &= ~CollectionFrame::CollectionFrameFloatable;
    EXPECT_FALSE(holder.floatable());
}

TEST(CollectionHolder, setHiddableCollection)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;

    holder.setHiddableCollection(true);

    EXPECT_TRUE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameHiddable));

    holder.setHiddableCollection(false);
    EXPECT_FALSE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameHiddable));
}

TEST(CollectionHolder, hiddableCollection)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->d->frameFeatures |= CollectionFrame::CollectionFrameHiddable;
    EXPECT_TRUE(holder.hiddableCollection());

    frame->d->frameFeatures &= ~CollectionFrame::CollectionFrameHiddable;
    EXPECT_FALSE(holder.hiddableCollection());
}

TEST(CollectionHolder, setStretchable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;

    holder.setStretchable(true);

    EXPECT_TRUE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameStretchable));

    holder.setStretchable(false);
    EXPECT_FALSE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameStretchable));
}

TEST(CollectionHolder, stretchable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->d->frameFeatures |= CollectionFrame::CollectionFrameStretchable;
    EXPECT_TRUE(holder.stretchable());

    frame->d->frameFeatures &= ~CollectionFrame::CollectionFrameStretchable;
    EXPECT_FALSE(holder.stretchable());
}

TEST(CollectionHolder, setAdjustable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;

    CollectionWidget w(QString("testuuid"), nullptr);
    holder.d->widget = &w;

    holder.setAdjustable(true);

    EXPECT_TRUE(w.d->titleBar->d->adjustable);
    EXPECT_TRUE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameAdjustable));

    holder.setAdjustable(false);
    EXPECT_FALSE(w.d->titleBar->d->adjustable);
    EXPECT_FALSE(frame->d->frameFeatures.testFlag(CollectionFrame::CollectionFrameAdjustable));
}

TEST(CollectionHolder, adjustable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionFrame *frame = new CollectionFrame;
    holder.d->frame = frame;
    frame->d->frameFeatures |= CollectionFrame::CollectionFrameAdjustable;
    EXPECT_TRUE(holder.adjustable());

    frame->d->frameFeatures &= ~CollectionFrame::CollectionFrameAdjustable;
    EXPECT_FALSE(holder.adjustable());
}

TEST(CollectionHolder, setRenamable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionWidget w(QString("testuuid"), nullptr);
    holder.d->widget = &w;

    holder.setRenamable(true);

    EXPECT_TRUE(w.d->titleBar->d->renamable);

    holder.setRenamable(false);
    EXPECT_FALSE(w.d->titleBar->d->renamable);
}

TEST(CollectionHolder, renamable)
{
    CollectionHolder holder(QString("testuuid"), nullptr);

    CollectionWidget w(QString("testuuid"), nullptr);
    holder.d->widget = &w;

    w.d->titleBar->d->renamable = true;
    EXPECT_TRUE(holder.renamable());

    w.d->titleBar->d->renamable = false;
    EXPECT_FALSE(holder.renamable());
}

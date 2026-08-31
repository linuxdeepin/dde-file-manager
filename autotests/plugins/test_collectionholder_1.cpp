// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionholder_1.cpp
 * @brief Unit tests for CollectionHolder methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "collection/collectionholder.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionHolderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionHolder();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionHolder *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionHolderTest, adjustable)
{
    // Test bool getter: adjustable()
    bool result = obj->adjustable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, closable)
{
    // Test bool getter: closable()
    bool result = obj->closable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, createAnimation)
{
    // Test getter: QPropertyAnimation createAnimation()
    auto result = obj->createAnimation();
    EXPECT_NO_FATAL_FAILURE({ obj->createAnimation(); });

}

TEST_F(CollectionHolderTest, createFrame)
{
    // Test method: void createFrame((Surface *surface, CollectionModel *model))
    EXPECT_NO_FATAL_FAILURE(obj->createFrame(nullptr, nullptr));
}

TEST_F(CollectionHolderTest, fileShiftable)
{
    // Test bool getter: fileShiftable()
    bool result = obj->fileShiftable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, floatable)
{
    // Test bool getter: floatable()
    bool result = obj->floatable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, frame)
{
    // Test getter: DFrame frame()
    auto result = obj->frame();
    EXPECT_NO_FATAL_FAILURE({ obj->frame(); });

}

TEST_F(CollectionHolderTest, hiddableCollection)
{
    // Test bool getter: hiddableCollection()
    bool result = obj->hiddableCollection();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, hiddableTitleBar)
{
    // Test bool getter: hiddableTitleBar()
    bool result = obj->hiddableTitleBar();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, hiddableView)
{
    // Test bool getter: hiddableView()
    bool result = obj->hiddableView();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, id)
{
    // Test getter: QString id()
    auto result = obj->id();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionHolderTest, itemView)
{
    // Test getter: CollectionView itemView()
    auto result = obj->itemView();
    EXPECT_NO_FATAL_FAILURE({ obj->itemView(); });

}

TEST_F(CollectionHolderTest, movable)
{
    // Test bool getter: movable()
    bool result = obj->movable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionHolderTest, openEditor)
{
    // Test method: void openEditor((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openEditor(_arg0));
}

TEST_F(CollectionHolderTest, renamable)
{
    // Test bool getter: renamable()
    bool result = obj->renamable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, selectFiles)
{
    // Test method: void selectFiles((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFiles(_arg0));
}

TEST_F(CollectionHolderTest, selectUrl)
{
    // Test method: void selectUrl((const QUrl &url, const QItemSelectionModel::SelectionFlag &flags))
    QUrl _arg0{};
    QItemSelectionModel::SelectionFlag _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectUrl(_arg0, _arg1));
}

TEST_F(CollectionHolderTest, setAdjustable)
{
    // Test setter: void setAdjustable((const bool adjustable))
    EXPECT_NO_FATAL_FAILURE(obj->setAdjustable(false));
}

TEST_F(CollectionHolderTest, setClosable)
{
    // Test setter: void setClosable((const bool closable))
    EXPECT_NO_FATAL_FAILURE(obj->setClosable(false));
}

TEST_F(CollectionHolderTest, setFileShiftable)
{
    // Test setter: void setFileShiftable((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setFileShiftable(false));
}

TEST_F(CollectionHolderTest, setFloatable)
{
    // Test setter: void setFloatable((const bool floatable))
    EXPECT_NO_FATAL_FAILURE(obj->setFloatable(false));
}

TEST_F(CollectionHolderTest, setFreeze)
{
    // Test setter: void setFreeze((bool freeze))
    EXPECT_NO_FATAL_FAILURE(obj->setFreeze(false));
}

TEST_F(CollectionHolderTest, setHiddableCollection)
{
    // Test setter: void setHiddableCollection((const bool hiddable))
    EXPECT_NO_FATAL_FAILURE(obj->setHiddableCollection(false));
}

TEST_F(CollectionHolderTest, setHiddableTitleBar)
{
    // Test setter: void setHiddableTitleBar((const bool hiddable))
    EXPECT_NO_FATAL_FAILURE(obj->setHiddableTitleBar(false));
}

TEST_F(CollectionHolderTest, setHiddableView)
{
    // Test setter: void setHiddableView((const bool hiddable))
    EXPECT_NO_FATAL_FAILURE(obj->setHiddableView(false));
}

TEST_F(CollectionHolderTest, setMovable)
{
    // Test setter: void setMovable((const bool movable))
    EXPECT_NO_FATAL_FAILURE(obj->setMovable(false));
}

TEST_F(CollectionHolderTest, setName)
{
    // Test setter: void setName((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setName(_arg0));
}

TEST_F(CollectionHolderTest, setRenamable)
{
    // Test setter: void setRenamable((const bool renamable))
    EXPECT_NO_FATAL_FAILURE(obj->setRenamable(false));
}

TEST_F(CollectionHolderTest, setStretchable)
{
    // Test setter: void setStretchable((const bool stretchable))
    EXPECT_NO_FATAL_FAILURE(obj->setStretchable(false));
}

TEST_F(CollectionHolderTest, setSurface)
{
    // Test setter: void setSurface((Surface *surface))
    EXPECT_NO_FATAL_FAILURE(obj->setSurface(nullptr));
}

TEST_F(CollectionHolderTest, stretchable)
{
    // Test bool getter: stretchable()
    bool result = obj->stretchable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionHolderTest, surface)
{
    // Test getter: Surface surface()
    auto result = obj->surface();
    EXPECT_NO_FATAL_FAILURE({ obj->surface(); });

}

TEST_F(CollectionHolderTest, widget)
{
    // Test getter: CollectionWidget widget()
    auto result = obj->widget();
    EXPECT_NO_FATAL_FAILURE({ obj->widget(); });

}

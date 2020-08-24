#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>
#include <QTextEdit>
#include <QTimer>

#define private public
#include "views/fileitem.h"

TEST(FileIconItemTest,init)
{
    FileIconItem w;
    EXPECT_NE(nullptr,w.icon);
    EXPECT_NE(nullptr,w.edit);
}

TEST(FileIconItemTest,get_icon_label)
{
    FileIconItem w;
    EXPECT_EQ(w.icon,w.getIconLabel());
}

TEST(FileIconItemTest,get_text_edit)
{
    FileIconItem w;
    EXPECT_EQ(w.edit,w.getTextEdit());
}

TEST(FileIconItemTest,opacity)
{
    FileIconItem w;
    EXPECT_EQ(nullptr,w.opacityEffect);
    EXPECT_EQ(1,w.opacity());

    w.setOpacity(0.5);
    EXPECT_NE(nullptr,w.opacityEffect);
    EXPECT_EQ(0.5,w.opacity());
}

TEST(FileIconItemTest,set_opacity)
{
    FileIconItem w;
    w.setOpacity(0.5);
    EXPECT_NE(nullptr,w.opacityEffect);
    EXPECT_EQ(0.5,w.opacity());

    w.setOpacity(1.0);
    EXPECT_EQ(nullptr,w.opacityEffect);
    EXPECT_EQ(1,w.opacity());

    w.setOpacity(1.1);
    EXPECT_EQ(nullptr,w.opacityEffect);
    EXPECT_EQ(1,w.opacity());
}

TEST(FileIconItemTest,set_maxchar_size)
{
    FileIconItem w;
    w.setMaxCharSize(10);
    EXPECT_EQ(10, w.maxCharSize);
}

TEST(FileIconItemTest,edit_textstack_currentitem)
{
    FileIconItem w;
    EXPECT_EQ(QString(""), w.editTextStackCurrentItem());

    w.pushItemToEditTextStack("test");
    EXPECT_EQ(QString("test"), w.editTextStackCurrentItem());
}

TEST(FileIconItemTest,edit_textstack_back_with_empty)
{
    FileIconItem w;
    EXPECT_EQ(QString(""), w.edit->toPlainText());
    w.editTextStackBack();
    EXPECT_EQ(QString(""), w.editTextStackCurrentItem());
}

TEST(FileIconItemTest,edit_textstack_advance_with_empty)
{
    FileIconItem w;
    EXPECT_EQ(QString(""), w.edit->toPlainText());
    w.editTextStackAdvance();
    EXPECT_EQ(QString(""), w.editTextStackCurrentItem());
    EXPECT_EQ(QString(""), w.edit->toPlainText());
}

TEST(FileIconItemTest,edit_undo_redo)
{
    FileIconItem w;
    QString str("test");
    w.show();
    QTest::keyClicks(w.edit, str);
    QEventLoop loop;
    QTimer::singleShot(100,&w,[&loop](){
        loop.exit();
    });
    loop.exec();
    EXPECT_EQ(str,w.edit->toPlainText());
    w.editUndo();
    QString undoStr = str.left(str.size() - 1);
    EXPECT_EQ(undoStr, w.edit->toPlainText());

    w.editRedo();
    EXPECT_EQ(str, w.edit->toPlainText());
}

TEST(FileIconItemTest,show_alert_message)
{
    FileIconItem w;
    QString str("test");
    EXPECT_NO_FATAL_FAILURE(w.showAlertMessage(str));
}

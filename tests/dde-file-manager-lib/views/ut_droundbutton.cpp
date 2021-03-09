/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *
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

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>


#define private public
#include <views/droundbutton.h>
#include <QTest>

TEST(DRoundButton,setRadiusF){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };

    bt.setRadiusF(0.5);
    EXPECT_DOUBLE_EQ(static_cast<double>(0.5), bt.m_radius.first);
}

TEST(DRoundButton,setRadius){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };

    std::size_t tempNum = static_cast<std::size_t>(5);
    bt.setRadius(tempNum);
    EXPECT_DOUBLE_EQ(tempNum, bt.m_radius.second);
}

TEST(DRoundButton,setCheckable){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };

    bt.setCheckable(true);
    EXPECT_TRUE(bt.m_checkable);
}

TEST(DRoundButton,setChecked){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };

    bt.setCheckable(true);
    bt.m_paintStatus = DRoundButton::PaintStatus::normal;
    bt.setChecked(false);
    EXPECT_TRUE(DRoundButton::PaintStatus::normal == bt.m_paintStatus);
    bt.setChecked(true);
    EXPECT_TRUE(DRoundButton::PaintStatus::checked == bt.m_paintStatus);
    bt.setChecked(true);
    EXPECT_TRUE(DRoundButton::PaintStatus::checked == bt.m_paintStatus);
    bt.setChecked(false);
    EXPECT_TRUE(DRoundButton::PaintStatus::normal == bt.m_paintStatus);
}

TEST(DRoundButton,isChecked){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };

    bt.setCheckable(true);
    bt.m_paintStatus = DRoundButton::PaintStatus::normal;
    bt.setChecked(true);
    auto expectedValue = DRoundButton::PaintStatus::checked == bt.m_paintStatus;

    EXPECT_TRUE(expectedValue == bt.isChecked());
}

TEST(DRoundButton,isHovered){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };
    auto expectedValue = bt.testAttribute(Qt::WA_UnderMouse) == bt.isHovered();
    EXPECT_TRUE(expectedValue);
}

TEST(DRoundButton,color){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };
    auto expectedValue = bt.m_allStatusColors.first().second == bt.color();
    EXPECT_TRUE(expectedValue);
}

TEST(DRoundButton,mousePressEvent){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };
    QTest::mousePress(&bt, Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint());
    auto expectedValue = bt.m_paintStatus == DRoundButton::PaintStatus::pressed;
    EXPECT_TRUE(expectedValue);
}

TEST(DRoundButton,mouseReleaseEvent){
    DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
                    { QColor{"#d8d8d8"}, QColor{"#cecece"} }
                   };
    bt.setCheckable(true);
    bt.setChecked(true);
    QTest::mouseRelease(&bt, Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint());
    auto expectedValue = bt.m_paintStatus == DRoundButton::PaintStatus::normal;
    EXPECT_TRUE(expectedValue);

    bt.setChecked(false);
    QTest::mouseRelease(&bt, Qt::MouseButton::LeftButton, Qt::KeyboardModifiers(), QPoint());
    expectedValue = bt.m_paintStatus == DRoundButton::PaintStatus::checked;
    EXPECT_TRUE(expectedValue);
}

TEST(DRoundButton,paintEvent){
    // 阻塞CI
    // DRoundButton bt{{ QColor{"#ff9311"}, QColor{"#ffa503"} },
    //                 { QColor{"#d8d8d8"}, QColor{"#cecece"} }
    //                };
    // bt.setPaintStatus(DRoundButton::PaintStatus::normal);
    // bt.show();
    // qApp->processEvents();
    // EXPECT_EQ(DRoundButton::PaintStatus::normal,bt.m_paintStatus);

    // bt.setPaintStatus(DRoundButton::PaintStatus::hover);
    // bt.repaint();
    // qApp->processEvents();
    // EXPECT_EQ(DRoundButton::PaintStatus::hover,bt.m_paintStatus);

    // bt.setPaintStatus(DRoundButton::PaintStatus::pressed);
    // bt.repaint();
    // qApp->processEvents();
    // EXPECT_EQ(DRoundButton::PaintStatus::pressed,bt.m_paintStatus);

    // bt.setPaintStatus(DRoundButton::PaintStatus::checked);
    // bt.repaint();
    // qApp->processEvents();
    // EXPECT_EQ(DRoundButton::PaintStatus::checked,bt.m_paintStatus);
}



#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "stub.h"

#define private public
#define protected public
#include <views/dfmsplitter.h>
#include <QObject>
#include <QEvent>
#include <QGuiApplication>

static bool inThere = false; //验证是否执行
class TempObjectForDFMSplitter: public QObject
{
public:
    TempObjectForDFMSplitter() {
        m_dsp = new DFMSplitter(Qt::Horizontal, nullptr);
    }
    ~TempObjectForDFMSplitter(){}
    void doTestDFMSplitterMoveSplitter(){
        QWidget *wdgLeft = new QWidget() ;
        QWidget *wdgRight = new QWidget;
        DFMSplitter dsp(Qt::Horizontal, nullptr);
        dsp.addWidget(wdgLeft);
        dsp.addWidget(wdgRight);
        dsp.moveSplitter(0,0);
        m_moveSplitter = true;
    }


public:
    bool m_moveSplitter{ false };
    DFMSplitter *m_dsp{ nullptr };
};

TEST(DFMSplitterTest, moveSplitterTest){
    TempObjectForDFMSplitter testObj;
    testObj.doTestDFMSplitterMoveSplitter();
    EXPECT_TRUE(testObj.m_moveSplitter);
}

TEST(DFMSplitterHandleTest, enterEvent){
    Stub stub;
    auto utSetOverrideCursor = static_cast<void(*)(const QCursor &)>([](const QCursor &){
        inThere = true;
    }) ;
    stub.set(ADDR(QGuiApplication, setOverrideCursor), utSetOverrideCursor);

    inThere = false;
    QSplitter tempSpltr;
    DFMSplitterHandle dsh(Qt::Orientation::Vertical, &tempSpltr);
    QEvent et(QEvent::HoverEnter);
    dsh.enterEvent(&et);

    EXPECT_TRUE(inThere);
}

TEST(DFMSplitterHandleTest, leaveEvent){
    Stub stub;
    auto utRestoreOverrideCursor = static_cast<void(*)(const QCursor &)>([](const QCursor &){
        inThere = true;
    }) ;
    stub.set(ADDR(QGuiApplication, restoreOverrideCursor), utRestoreOverrideCursor);

    inThere = false;
    QSplitter tempSpltr;
    DFMSplitterHandle dsh(Qt::Orientation::Vertical, &tempSpltr);
    QEvent et(QEvent::HoverLeave);
    dsh.leaveEvent(&et);

    EXPECT_TRUE(inThere);
}

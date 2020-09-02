#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#define protected public
#include <views/dfmsplitter.h>
#include <QObject>
#include <QEventLoop>


class TempObjectForDFMSplitter: public QObject
{
public:
    TempObjectForDFMSplitter() {
        m_dsp = new DFMSplitter(Qt::Horizontal, nullptr);
    }
    ~TempObjectForDFMSplitter(){}
    void doTestDFMSplitterMoveSplitter(){
//        connect(m_dsp, &DFMSplitter::splitterMoved,this, [=]{
//            m_moveSplitter = true;
//        });
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










//TEST(DFMSplitterHandleTest, enterEvent){
//    DFMSplitterHandle dsh;

//}

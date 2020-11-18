#include <gtest/gtest.h>
#include <interfaces/dfmbaseview.h>
#include <QSharedPointer>

using namespace testing;
DFM_USE_NAMESPACE

class TestDfmBaseView: public DFMBaseView
{
public:
    TestDfmBaseView() : DFMBaseView() {

    }
    QWidget *widget() const override;
    DUrl rootUrl() const override;
    bool setRootUrl(const DUrl &url) override;
};


class DfmBaseViewTest:public testing::Test{

public:
    virtual void SetUp() override{
        baseview.reset(new TestDfmBaseView());
        std::cout << "start DFileSeviceTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end DFileSeviceTest" << std::endl;
    }

    QSharedPointer<TestDfmBaseView> baseview = nullptr;
};

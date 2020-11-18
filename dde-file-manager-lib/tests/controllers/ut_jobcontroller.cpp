#include <gtest/gtest.h>

#include "controllers/jobcontroller.h"
#include "controllers/filecontroller.h"
#include "dfmevent.h"

using namespace testing;

using namespace testing;

class JobControllerTest:public testing::Test{

public:
    QSharedPointer<JobController> jobcontroller;
    QSharedPointer<FileController> filecontroller;
    virtual void SetUp() override{
        init("~/Desktop",true);
        std::cout << "start JobControllerTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end JobControllerTest" << std::endl;
    }

    void init(const char *path,bool bresetfilecontroller = false) {
        if (bresetfilecontroller) {
            filecontroller.reset(new FileController());
        }
        DUrl url;
        url.fromLocalFile(path);
        bool isgvfs = false;
        QDirIterator::IteratorFlags flags = static_cast<QDirIterator::IteratorFlags>(DDirIterator::SortINode);
        auto director = filecontroller->createDirIterator(dMakeEventPointer<DFMCreateDiriterator>(nullptr, url, QStringList(),
                                                                                                  QDir::AllEntries, flags, false, isgvfs));
        jobcontroller.reset(new JobController(url,director));
    }
};

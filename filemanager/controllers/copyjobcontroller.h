#ifndef COPYJOBCONTROLLER_H
#define COPYJOBCONTROLLER_H

#include <QtCore>
#include <QtDBus>

class CopyJobInterface;
class CopyjobWorker;

class CopyJobController : public QObject
{
    Q_OBJECT
public:
    explicit CopyJobController(QObject *parent = 0);
    ~CopyJobController();

    void initConnect();
};

#endif // COPYJOBCONTROLLER_H

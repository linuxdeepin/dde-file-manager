#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

#include <QObject>

class FileInfoGatherer;
class IconProvider;
class RecentHistoryManager;

class FileController : public QObject
{
    Q_OBJECT

public:
    explicit FileController(QObject *parent = 0);
    ~FileController();

    void initGatherer();
    void initRecentManager();
    void initIconProvider();
    void initConnect();

public slots:
    void getIcon(const QString &url) const;
    void onFetchFileInformation(const QString &url, int filter);

private:
    FileInfoGatherer *gatherer = Q_NULLPTR;
    RecentHistoryManager *recentManager = Q_NULLPTR;
    QThread *gathererThread = Q_NULLPTR;

    IconProvider *iconProvider = Q_NULLPTR;
};

#endif // FILECONTROLLER_H

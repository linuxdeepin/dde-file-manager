#ifndef FILECALCULATIONUTILS_H
#define FILECALCULATIONUTILS_H

#include "dfmplugin_propertydialog_global.h"

#include <QObject>
#include <QList>

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class FileCalculationUtils : public QObject
{
    Q_OBJECT
public:
    struct FileStatisticInfo
    {
        qint64 totalSize { 0 };
        qint32 fileCount { 0 };
    };

    explicit FileCalculationUtils(QObject *parent = nullptr);

    virtual ~FileCalculationUtils() override;

    void startThread(const QList<QUrl> &files);

signals:
    void sigFileChange(qint32 count);

    void sigTotalChange(qint64 totalSize);

    void sigStartRun(const QList<QUrl> &files);

public slots:
    void statisticsFileInfo(const QList<QUrl> &files);

private:
    quint16 getMemoryPageSize();
    void statisticFilesSize(const QUrl &url, FileStatisticInfo *&sizeInfo);

private:
    QThread *thread { nullptr };
    FileStatisticInfo *info { nullptr };
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // FILECALCULATIONUTILS_H

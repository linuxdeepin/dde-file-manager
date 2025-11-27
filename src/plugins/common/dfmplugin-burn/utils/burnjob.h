// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNJOB_H
#define BURNJOB_H

#include "dfmplugin_burn_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <dfm-burn/dopticaldiscmanager.h>

#include <QThread>

namespace dfmplugin_burn {

class AbstractBurnJob : public QThread
{
    Q_OBJECT

public:
    enum JobType {
        kOpticalBurn,
        kOpticalBlank,
        kOpticalImageBurn,
        kOpticalCheck,
        kOpticalImageDump
    };

    enum PropertyType {
        KStagingUrl,
        kImageUrl,
        kVolumeName,
        kSpeeds,
        kBurnOpts
    };

    enum JobPhase {
        kReady,
        kWriteData,
        kCheckData
    };

    using ProperyMap = QMap<PropertyType, QVariant>;

public:
    explicit AbstractBurnJob(const QString &dev, const JobHandlePointer handler);
    virtual ~AbstractBurnJob() override {}

    QVariantMap currentDeviceInfo() const;
    QVariant property(PropertyType type) const;
    void setProperty(PropertyType type, const QVariant &val);
    void addTask();

protected:
    virtual bool fileSystemLimitsValid();
    virtual void updateMessage(JobInfoPointer ptr);
    virtual void updateSpeed(JobInfoPointer ptr, DFMBURN::JobStatus status, const QString &speed);
    virtual void readFunc(int progressFd, int checkFd);
    virtual void writeFunc(int progressFd, int checkFd);
    virtual void finishFunc(bool verify, bool verifyRet);
    virtual void work() = 0;

    void run() override;
    bool readyToWork();
    void workingInSubProcess();
    [[nodiscard]] DFMBURN::DOpticalDiscManager *createManager(int fd);
    QByteArray updatedInSubProcess(DFMBURN::JobStatus status, int progress, const QString &speed, const QStringList &message);
    void comfort();
    bool mediaChangDected();

signals:
    void requestErrorMessageDialog(const QString &title, const QString &message);
    void requestFailureDialog(int type, const QString &err, const QStringList &details);
    void requestCompletionDialog(const QString &msg, const QString &icon);
    void requestCloseTab(const QUrl &url);
    void requestReloadDisc(const QString &devId);
    void burnFinished(int type, bool reuslt);

public slots:
    void onJobUpdated(DFMBURN::JobStatus status, int progress, const QString &speed, const QStringList &message);

protected:
    QString curDev;
    QString curDevId;
    QVariantMap curDeviceInfo;
    JobHandlePointer jobHandlePtr {};
    ProperyMap curProperty;
    JobType firstJobType;
    JobType curJobType;
    int lastProgress {};
    int curPhase {};
    QString lastError;
    QStringList lastSrcMessages;
    DFMBURN::JobStatus lastStatus;
    bool jobSuccess {};   // delete staging files if sucess
};

class EraseJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit EraseJob(const QString &dev, const JobHandlePointer handler);
    virtual ~EraseJob() override {}

signals:
    void eraseFinished(bool result);

protected:
    virtual void updateMessage(JobInfoPointer ptr) override;
    virtual void work() override;
};

class BurnISOFilesJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit BurnISOFilesJob(const QString &dev, const JobHandlePointer handler);
    virtual ~BurnISOFilesJob() override {}

protected:
    virtual bool fileSystemLimitsValid() override;
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void work() override;
};

class BurnISOImageJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit BurnISOImageJob(const QString &dev, const JobHandlePointer handler);
    virtual ~BurnISOImageJob() override {}

protected:
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void work() override;
};

class BurnUDFFilesJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit BurnUDFFilesJob(const QString &dev, const JobHandlePointer handler);
    virtual ~BurnUDFFilesJob() override {}

protected:
    virtual bool fileSystemLimitsValid() override;
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void work() override;
    virtual void finishFunc(bool verify, bool verifyRet) override;
};

class DumpISOImageJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit DumpISOImageJob(const QString &dev, const JobHandlePointer handler);
    virtual ~DumpISOImageJob() override {}

signals:
    void requestOpticalDumpISOSuccessDialog(const QUrl &imageUrl);
    void requestOpticalDumpISOFailedDialog();

protected:
    virtual void updateMessage(JobInfoPointer ptr) override;
    virtual void updateSpeed(JobInfoPointer ptr, DFMBURN::JobStatus status, const QString &speed) override;
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void finishFunc(bool verify, bool verifyRet) override;
    virtual void work() override;
};
}   // namespace dfmplugin_burn
Q_DECLARE_METATYPE(DFMBURN::BurnOptions)

#endif   // BURNJOB_H

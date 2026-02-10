// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNCHECKSTRATEGY_H
#define BURNCHECKSTRATEGY_H

#include <QObject>
#include <QFileInfo>

namespace dfmplugin_burn {

class BurnCheckStrategy : public QObject
{
    Q_OBJECT

public:
    explicit BurnCheckStrategy(const QString &path, QObject *parent = nullptr);
    bool check();
    QString lastError() const;
    QString lastInvalidName() const;

private:
    bool validFile(const QFileInfo &info);
    QString autoFeed(const QString &text) const;

protected:
    bool validCommonFileNameBytes(const QString &fileName);
    bool validComontFilePathBytes(const QString &filePath);
    bool validCommonFilePathDeepLength(const QString &filePath);

    virtual bool validFileNameCharacters(const QString &fileName);
    virtual bool validFilePathCharacters(const QString &filePath);
    virtual bool validFileNameBytes(const QString &fileName);
    virtual bool validFilePathBytes(const QString &filePath);
    virtual bool validFilePathDeepLength(const QString &filePath);

protected:
    QString invalidName;
    QString errorMsg;
    QString currentStagePath;
};

class ISO9660CheckStrategy final : public BurnCheckStrategy
{
    Q_OBJECT

public:
    explicit ISO9660CheckStrategy(const QString &path, QObject *parent = nullptr);

protected:
    bool validFileNameCharacters(const QString &fileName) override;
    bool validFilePathDeepLength(const QString &filePath) override;
};

class JolietCheckStrategy final : public BurnCheckStrategy
{
    Q_OBJECT

public:
    explicit JolietCheckStrategy(const QString &path, QObject *parent = nullptr);

protected:
    bool validFileNameCharacters(const QString &fileName) override;
    bool validFilePathCharacters(const QString &filePath) override;
};

class RockRidgeCheckStrategy final : public BurnCheckStrategy
{
    Q_OBJECT

public:
    explicit RockRidgeCheckStrategy(const QString &path, QObject *parent = nullptr);

protected:
    bool validFileNameBytes(const QString &fileName) override;
    bool validFilePathBytes(const QString &filePath) override;
    bool validFilePathDeepLength(const QString &filePath) override;
};

class UDFCheckStrategy final : public BurnCheckStrategy
{
    Q_OBJECT

public:
    explicit UDFCheckStrategy(const QString &path, QObject *parent = nullptr);

protected:
    bool validFileNameBytes(const QString &fileName) override;
    bool validFilePathBytes(const QString &filePath) override;
};

}   // namespace

#endif   // BURNCHECKSTRATEGY_H

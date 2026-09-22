// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_COVBEANS_H
#define UT_COVBEANS_H

#include <QObject>
#include <QString>

/**
 * @brief Coverage beans for test_sqlitehelper_cov.cpp.
 *
 * The real bean classes used by the dfm-upgrade tool (dfm_upgrade::TagProperty,
 * dfm_upgrade::FileTagInfo, dfm_upgrade::VirtualEntryData, global-namespace
 * OldTagProperty / OldFileProperty / SqliteMaster) and by the smb browser
 * plugin (dfmplugin_smbbrowser::VirtualEntryData) are compiled into other
 * binaries and are NOT linked into ut-dfm-base.  To make the ut-dfm-base test
 * binary execute the exact SqliteHelper / SqliteHandle / SqliteQueryable
 * template instantiations named in the coverage gap list, this header
 * re-declares classes with the SAME namespaces and class names (meta-object
 * driven, so Q_PROPERTY field lists are what the helpers reflect over).
 * There is no ODR conflict because the original definitions are not part of
 * this binary.
 */
class OldTagProperty : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "ut_cov_old_tag_property")
    Q_PROPERTY(int tag_index READ tagIndex WRITE setTagIndex)
    Q_PROPERTY(QString tag_name READ tagName WRITE setTagName)
    Q_PROPERTY(QString tag_color READ tagColor WRITE setTagColor)
public:
    explicit OldTagProperty(QObject *parent = nullptr)
        : QObject(parent) {}
    int tagIndex() const { return m_index; }
    void setTagIndex(int v) { m_index = v; }
    QString tagName() const { return m_name; }
    void setTagName(const QString &v) { m_name = v; }
    QString tagColor() const { return m_color; }
    void setTagColor(const QString &v) { m_color = v; }
private:
    int m_index = 0;
    QString m_name;
    QString m_color;
};

class OldFileProperty : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "ut_cov_old_file_property")
    Q_PROPERTY(int property_index READ propertyIndex WRITE setPropertyIndex)
    Q_PROPERTY(QString file_path READ filePath WRITE setFilePath)
    Q_PROPERTY(QString file_property READ getFileProperty WRITE setFileProperty)
public:
    explicit OldFileProperty(QObject *parent = nullptr)
        : QObject(parent) {}
    int propertyIndex() const { return m_index; }
    void setPropertyIndex(int v) { m_index = v; }
    QString filePath() const { return m_path; }
    void setFilePath(const QString &v) { m_path = v; }
    QString getFileProperty() const { return m_property; }
    void setFileProperty(const QString &v) { m_property = v; }
private:
    int m_index = 0;
    QString m_path;
    QString m_property;
};

class SqliteMaster : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "sqlite_master")
    Q_PROPERTY(QString type READ getType WRITE setType)
    Q_PROPERTY(QString name READ getName WRITE setName)
    Q_PROPERTY(QString tbl_name READ getTblName WRITE setTblName)
    Q_PROPERTY(int rootpage READ getRootpage WRITE setRootpage)
    Q_PROPERTY(QString sql READ getSql WRITE setSql)
public:
    explicit SqliteMaster(QObject *parent = nullptr)
        : QObject(parent) {}
    QString getType() const { return m_type; }
    void setType(const QString &v) { m_type = v; }
    QString getName() const { return m_name; }
    void setName(const QString &v) { m_name = v; }
    QString getTblName() const { return m_tblName; }
    void setTblName(const QString &v) { m_tblName = v; }
    int getRootpage() const { return m_rootpage; }
    void setRootpage(int v) { m_rootpage = v; }
    QString getSql() const { return m_sql; }
    void setSql(const QString &v) { m_sql = v; }
private:
    QString m_type;
    QString m_name;
    QString m_tblName;
    int m_rootpage = 0;
    QString m_sql;
};

namespace dfm_upgrade {

class TagProperty : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "ut_cov_tag_property")
    Q_PROPERTY(int tag_index READ tagIndex WRITE setTagIndex)
    Q_PROPERTY(QString tag_name READ tagName WRITE setTagName)
    Q_PROPERTY(QString tag_color READ tagColor WRITE setTagColor)
public:
    explicit TagProperty(QObject *parent = nullptr)
        : QObject(parent) {}
    int tagIndex() const { return m_index; }
    void setTagIndex(int v) { m_index = v; }
    QString tagName() const { return m_name; }
    void setTagName(const QString &v) { m_name = v; }
    QString tagColor() const { return m_color; }
    void setTagColor(const QString &v) { m_color = v; }
private:
    int m_index = 0;
    QString m_name;
    QString m_color;
};

class FileTagInfo : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "ut_cov_file_tag_info")
    Q_PROPERTY(int file_index READ fileIndex WRITE setFileIndex)
    Q_PROPERTY(QString file_path READ filePath WRITE setFilePath)
    Q_PROPERTY(QString tag_name READ tagName WRITE setTagName)
public:
    explicit FileTagInfo(QObject *parent = nullptr)
        : QObject(parent) {}
    int fileIndex() const { return m_index; }
    void setFileIndex(int v) { m_index = v; }
    QString filePath() const { return m_path; }
    void setFilePath(const QString &v) { m_path = v; }
    QString tagName() const { return m_name; }
    void setTagName(const QString &v) { m_name = v; }
private:
    int m_index = 0;
    QString m_path;
    QString m_name;
};

class VirtualEntryData : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "ut_cov_upgrade_ved")
    Q_PROPERTY(int ved_index READ vedIndex WRITE setVedIndex)
    Q_PROPERTY(QString ved_display READ vedDisplay WRITE setVedDisplay)
public:
    explicit VirtualEntryData(QObject *parent = nullptr)
        : QObject(parent) {}
    int vedIndex() const { return m_index; }
    void setVedIndex(int v) { m_index = v; }
    QString vedDisplay() const { return m_display; }
    void setVedDisplay(const QString &v) { m_display = v; }
private:
    int m_index = 0;
    QString m_display;
};

}   // namespace dfm_upgrade

namespace dfmplugin_smbbrowser {

class VirtualEntryData : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "ut_cov_smb_ved")
    Q_PROPERTY(int ved_index READ vedIndex WRITE setVedIndex)
    Q_PROPERTY(QString ved_name READ vedName WRITE setVedName)
    Q_PROPERTY(QString ved_type READ vedType WRITE setVedType)
public:
    explicit VirtualEntryData(QObject *parent = nullptr)
        : QObject(parent) {}
    int vedIndex() const { return m_index; }
    void setVedIndex(int v) { m_index = v; }
    QString vedName() const { return m_name; }
    void setVedName(const QString &v) { m_name = v; }
    QString vedType() const { return m_type; }
    void setVedType(const QString &v) { m_type = v; }
private:
    int m_index = 0;
    QString m_name;
    QString m_type;
};

}   // namespace dfmplugin_smbbrowser

#endif   // UT_COVBEANS_H

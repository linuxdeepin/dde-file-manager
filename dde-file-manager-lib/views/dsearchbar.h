#ifndef DSEARCHBAR_H
#define DSEARCHBAR_H

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QListView>
#include <QMenu>
#include <QHBoxLayout>
#include <QCompleter>
#include <QStringListModel>
#include <QStringList>
#include <QDirModel>
#include <QPushButton>
#include "durl.h"

class DFMEvent;

class DSearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit DSearchBar(QWidget *parent = 0);
    ~DSearchBar();
    void setPopup(QListWidget * popup);
    QListWidget * getPopupList();
    QAction * setClearAction();
    QAction * removeClearAction();
    QAction * setJumpToAction();
    QAction * removeJumpToAction();
    QAction * setSearchAction();
    QAction * removeSearchAction();
    bool isActive();
    void setActive(bool active);
    QAction * getClearAction();
    bool hasScheme();
    bool isSearchFile();
    bool isBookmarkFile();
    bool isComputerFile();
    bool isLocalFile();
    bool isTrashFile();
    bool isUserShareFile();
    bool isPath();
    void setCurrentUrl(const DUrl &path);
private:
    void initData();
    void initUI();
    void keyUpDown(int key);
    void recommended(const QString& inputText);
    void complete(const QString & str);
    QStringList splitPath(const QString &path);
    QListWidget * m_list;
    QCompleter * m_completer;
    QCompleter * m_historyCompleter;
    QAction * m_clearAction;
    QAction * m_searchAction;
    QAction * m_jumpToAction;
    QStringListModel * m_stringListMode;
    QStringList m_historyList;
    QDirModel * m_dirModel;
    bool m_isActive = false;
    void initConnections();
    QString m_text;
    bool m_disableCompletion = false;
    bool m_searchStart = false;
    DUrl m_currentUrl;
    QScrollBar* m_listVerticalScrollBar = NULL;
public slots:
    void doTextChanged(QString text);
    void searchHistoryLoaded(const QStringList &list);
    void historySaved();
    void setCompleter(const QString &text);
    void completeText(QListWidgetItem * item);
    void currentUrlChanged(const DFMEvent &event);
    void clearText();
    void hideCompleter();
    void handleApplicationChanged(QWidget * old, QWidget * now);
    void setText(const QString &text);
    void jumpTo();
    void search();
protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    bool event(QEvent *e);
    bool eventFilter(QObject *obj, QEvent *e);
    void resizeEvent(QResizeEvent *e);
    void moveEvent(QMoveEvent *e);
    void mousePressEvent(QMouseEvent *e);

signals:
    void searchBarFocused();
    void focusedOut();
};

#endif // DSEARCHBAR_H

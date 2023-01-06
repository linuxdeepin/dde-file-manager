// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRENAMEBAR_H
#define DRENAMEBAR_H


#include <QList>
#include <QFrame>
#include <memory>
#include <array>


#include "shutil/filebatchprocess.h"


class DRenameBarPrivate;


///###: This structure record DRenameBar's state, content.
class RecordRenameBarState
{
private:
    QPair<QString, QString> m_patternFirst{};
    QPair<QString, DFileService::AddTextFlags> m_patternSecond{};
    QPair<QString, QString> m_patternThird{};
    std::array<bool, 3> m_buttonStateInThreePattern{{false}};
    std::size_t m_currentPattern{ 0 };
    QList<DUrl> m_selectedUrls{};
    std::atomic<bool> m_visible{ false };



public:
    RecordRenameBarState() = default;
    ~RecordRenameBarState() = default;


    ///###: copy!
    RecordRenameBarState(const RecordRenameBarState &other);
    RecordRenameBarState(RecordRenameBarState &&other);

    ///###: move!
    RecordRenameBarState &operator=(const RecordRenameBarState &other);
    RecordRenameBarState &operator=(RecordRenameBarState &&other);

    ///###: clear!
    void clear();

    ///###: set!
    template<typename Type, typename ...Types>
    RecordRenameBarState(Type &&arg, Types &&... args);

    inline void setPatternFirstItemContent(const QPair<QString, QString> &patternOneItemsContent)
    {
        m_patternFirst = patternOneItemsContent;
    }

    inline void setPatternSecondItemContent(const QPair<QString, DFileService::AddTextFlags> &patternTwoItemsContent)
    {
        m_patternSecond = patternTwoItemsContent;
    }

    inline void setPatternThirdItemContent(const QPair<QString, QString> &patternThreeItemsContent)
    {
        m_patternThird = patternThreeItemsContent;
    }

    inline void setbuttonStateInThreePattern(const std::array<bool, 3> &buttonsState)
    {
        m_buttonStateInThreePattern = buttonsState;
    }

    inline void setCurrentPattern(const std::size_t &currentPattern)
    {
        m_currentPattern = currentPattern;
    }

    inline void setSelectedUrls(const QList<DUrl> &urlList)
    {
        m_selectedUrls = urlList;
    }

    inline void setStoredValueOfVisible(const bool &value)
    {
        m_visible.store(value, std::memory_order_seq_cst);
    }


    ///###: get!
    inline const QPair<QString, QString> &getPatternFirstItemContent()const noexcept
    {
        return m_patternFirst;
    }
    inline const QPair<QString, DFileService::AddTextFlags> &getPatternSecondItemContent()const noexcept
    {
        return m_patternSecond;
    }
    inline const QPair<QString, QString> &getPatternThirdItemContent()const noexcept
    {
        return m_patternThird;
    }
    inline const std::array<bool, 3> &getbuttonStateInThreePattern()const noexcept
    {
        return m_buttonStateInThreePattern;
    }
    inline const std::size_t &getCurrentPattern()const noexcept
    {
        return m_currentPattern;
    }
    inline const QList<DUrl> &getSelectedUrl()const noexcept
    {
        return m_selectedUrls;
    }
    inline bool getVisibleValue()const noexcept
    {
        return m_visible.load(std::memory_order_seq_cst);
    }
};



class DRenameBar : public QFrame
{
    Q_OBJECT
public:
    explicit DRenameBar(QWidget *parent = nullptr);//###: init widgets and layout widgets when constructing DRenameBarPrivate.
    virtual ~DRenameBar() override = default;

    DRenameBar(const DRenameBar &) = delete;
    DRenameBar &operator=(const DRenameBar &) = delete;


    void storeUrlList(const QList<DUrl> &list)noexcept;
    void resetRenameBar()noexcept; //###: do not delete item! just clear content
    std::unique_ptr<RecordRenameBarState> getCurrentState()const;// return state and parameters by this function.
    void loadState(std::unique_ptr<RecordRenameBarState> &state);

    virtual void setVisible(bool value) override;

protected:
    virtual void keyPressEvent(QKeyEvent *event)override;

signals:
    void requestReplaceOperator();
    void clickCancelButton();
    void clickRenameButton();

    void visibleChanged(bool value);


private slots:
    void onVisibleChanged(bool value)noexcept;
    void onRenamePatternChanged(const int &index)noexcept;

    void onReplaceOperatorFileNameChanged(const QString &text)noexcept;
    void onReplaceOperatorDestNameChanged(const QString &textChanged)noexcept;
    void onAddOperatorAddedContentChanged(const QString &text)noexcept;
    void onAddTextPatternChanged(const int &index)noexcept;
    void onCustomOperatorFileNameChanged()noexcept;
    void onCustomOperatorSNNumberChanged();
    void eventDispatcher();


private:
    ///###: just connect signals and slots(through d pointer).
    void initConnect();

    QSharedPointer<DRenameBarPrivate> d_ptr{ nullptr };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DRenameBar)
};

#endif // DRENAMEBAR_H

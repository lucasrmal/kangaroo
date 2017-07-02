#ifndef REWARDSPROGRAM_H
#define REWARDSPROGRAM_H

#include <KangarooLib/model/stored.h>
#include <KangarooLib/amount.h>

struct RewardTier
{
    RewardTier() : id(KLib::Constants::NO_ID), rate(0) {}
    RewardTier(int _id, const QString& _name, const KLib::Amount& _rate) :
        id(_id),
        name(_name),
        rate(_rate) {}

    int             id;
    QString         name;
    KLib::Amount    rate;

    static const QString TAG;
};

class RewardsProgram : public KLib::IStored
{
    Q_OBJECT

    public:
        RewardsProgram();

        QString     name() const        { return m_name; }
        QString     currency() const  { return m_currency; }
        const QList<RewardTier>& tiers() const { return m_tiers; }
        const RewardTier& tier(int _id) const;

        bool isInUse() const;
        bool tierExists(int _idTier) const;

        void setName(const QString& _name);
        void setCurrency(const QString& _code);
        void setTiers(const QList<RewardTier>& _tiers);
        void setTierName(int _id, const QString& _name);
        void setTierRate(int _id, const KLib::Amount& _rate);

        static QString validateTiers(const QList<RewardTier>& _tiers);

        static const QString TAG;

        static const int REWARDS_ACCOUNT_TYPE;
        static const QString PROP_REWARDS_PROGRAM;
        static const QString PROP_REWARDS_TARGET_ACCOUNT;
        static const QString PROP_REWARDS_SOURCE_ACCOUNT;

        static const QString PROP_TRANS_ID_TIER;
        static const QString PROP_TRANS_REWARD_AMOUNT;

    protected:
        void load(QXmlStreamReader& _reader);
        void save(QXmlStreamWriter& _writer) const;

    private:
        QString m_name;
        QString m_currency;
        QList<RewardTier> m_tiers;

        RewardTier& notconst_tier(int _id);

        friend class RewardsProgramManager;
};

class RewardsProgramManager : public KLib::IStored
{
    Q_OBJECT

        RewardsProgramManager();
    public:
        RewardsProgram* add(const QString& _name,
                            const QString& _currency,
                            const QList<RewardTier>& _tiers);
        RewardsProgram* get(int _id) const;
        void remove(int _id);

        int count() const { return m_programs.size(); }

        const QList<RewardsProgram*> programs() const { return m_programs.values(); }

        static RewardsProgramManager* instance() { return m_instance; }

        static const QString TAG;

    signals:
        void programAdded(RewardsProgram* _p);
        void programRemoved(RewardsProgram* _p);
        void programModified(RewardsProgram* _p);

    private slots:
        void onModified();

    private:
        QHash<int, RewardsProgram*> m_programs;
        int m_nextId;

        static RewardsProgramManager* m_instance;

    protected:
        void load(QXmlStreamReader& _reader);
        void save(QXmlStreamWriter& _writer) const;
        void unload();

};

#endif // REWARDPROGRAM_H

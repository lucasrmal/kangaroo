#include "rewardsprogram.h"
#include <KangarooLib/controller/io.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/account.h>

#include <QXmlStreamReader>

using namespace KLib;

RewardsProgramManager* RewardsProgramManager::m_instance = new RewardsProgramManager();

const QString RewardTier::TAG = "rewardstier";
const QString RewardsProgram::TAG = "rewardsprogram";
const QString RewardsProgramManager::TAG = "rewardsprograms";

const QString RewardsProgram::PROP_REWARDS_PROGRAM = "rewardsprogram";
const QString RewardsProgram::PROP_REWARDS_TARGET_ACCOUNT = "rewardsaccount";
const QString RewardsProgram::PROP_REWARDS_SOURCE_ACCOUNT = "rewardsincomeaccount";

const QString RewardsProgram::PROP_TRANS_ID_TIER = "reward_idtier";
const QString RewardsProgram::PROP_TRANS_REWARD_AMOUNT = "reward_amount";
const int RewardsProgram::REWARDS_ACCOUNT_TYPE = 200;

RewardsProgram::RewardsProgram()
{
}

const RewardTier& RewardsProgram::tier(int _id) const
{
    auto i = m_tiers.begin();
    for (; i != m_tiers.end(); ++i)
    {
        if ((*i).id == _id)
        {
            return *i;
        }
    }

    ModelException::throwException(tr("No such tier!"), this);
    return *i; //Just to shut up the compiler, will never be executed
}

bool RewardsProgram::isInUse() const
{
    for (Account* a : Account::getTopLevel()->accounts())
    {
        if (a->properties()->contains(PROP_REWARDS_PROGRAM) &&
            a->properties()->get(PROP_REWARDS_PROGRAM).toInt() == m_id)
        {
            return true;
        }
    }

    return false;
}

bool RewardsProgram::tierExists(int _idTier) const
{
    for (const RewardTier& t : m_tiers)
    {
        if (t.id == _idTier)
        {
            return true;
        }
    }

    return false;
}

RewardTier& RewardsProgram::notconst_tier(int _id)
{
    auto i = m_tiers.begin();
    for (; i != m_tiers.end(); ++i)
    {
        if ((*i).id == _id)
        {
            return *i;
        }
    }

    ModelException::throwException(tr("No such tier!"), this);
    return *i; //Just to shut up the compiler, will never be executed
}

void RewardsProgram::setName(const QString& _name)
{
    if (_name.isEmpty())
    {
        ModelException::throwException(tr("The name of the rewards program cannot be empty."), this);
    }
    else if (_name != m_name)
    {
        m_name = _name;

        if (!onHoldToModify())
            emit modified();
    }
}

void RewardsProgram::setCurrency(const QString& _code)
{
    //Cannot change the currency if is in use (some accounts use the currency)
    if (isInUse())
    {
        ModelException::throwException(tr("Cannot change the currency of a rewards program that is in use."), this);
    }

    //Check if the currency exists
    CurrencyManager::instance()->get(_code);

    if (m_currency != _code)
    {
        m_currency = _code;

        if (!onHoldToModify())
            emit modified();
    }
}

void RewardsProgram::setTiers(const QList<RewardTier>& _tiers)
{
    QString v = validateTiers(_tiers);

    if (!v.isEmpty())
    {
        ModelException::throwException(v, this);
    }

    //All checked, we can save
    m_tiers = _tiers;

    if (!onHoldToModify())
        emit modified();

}

QString RewardsProgram::validateTiers(const QList<RewardTier>& _tiers)
{
    QSet<int> s;

    for (const RewardTier& t : _tiers)
    {
        if (s.contains(t.id))
        {
            return tr("Duplicate id in the reward tiers.");
        }
        else if (t.id < 0)
        {
            return tr("Invalid tier id: %1").arg(t.id);
        }
        else if (t.name.isEmpty())
        {
            return tr("Empty name for tier %1").arg(t.id);
        }
        else if (t.rate < 0)
        {
            return tr("Invalid rate for tier %1").arg(t.id);
        }

        s.insert(t.id);
    }

    return "";
}

void RewardsProgram::setTierName(int _id, const QString& _name)
{
    if (_name.isEmpty())
    {
        ModelException::throwException(tr("Tier name cannot be empty"), this);
    }
    else
    {
        notconst_tier(_id).name = _name;
    }

    if (!onHoldToModify())
        emit modified();
}

void RewardsProgram::setTierRate(int _id, const KLib::Amount& _rate)
{
    if (_rate < 0)
    {
        ModelException::throwException(tr("Tier rate cannot be negative"), this);
    }
    else
    {
        notconst_tier(_id).rate = _rate;
    }

    if (!onHoldToModify())
        emit modified();
}

void RewardsProgram::load(QXmlStreamReader& _reader)
{
    QXmlStreamAttributes attributes = _reader.attributes();

    m_id        = IO::getAttribute("id", attributes).toInt();
    m_name      = IO::getAttribute("name", attributes);
    m_currency  = IO::getAttribute("currency", attributes);
    int numT    = IO::getAttribute("numtiers", attributes).toInt();

    if (numT > 0 && _reader.readNextStartElement())
    {
        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == RewardsProgram::TAG))
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == RewardTier::TAG)
            {
                RewardTier t;
                attributes = _reader.attributes();

                t.id    = IO::getAttribute("id", attributes).toInt();
                t.name  = IO::getAttribute("name", attributes);
                t.rate  = Amount::fromStoreable(IO::getAttribute("rate", attributes));

                m_tiers << t;
            }

            _reader.readNext();
        }
    }
}

void RewardsProgram::save(QXmlStreamWriter& _writer) const
{
    _writer.writeStartElement(RewardsProgram::TAG);
    _writer.writeAttribute("id", QString::number(m_id));
    _writer.writeAttribute("name", m_name);
    _writer.writeAttribute("currency", m_currency);
    _writer.writeAttribute("numtiers", QString::number(m_tiers.count()));

    for (const RewardTier& t: m_tiers)
    {
        _writer.writeEmptyElement(RewardTier::TAG);
        _writer.writeAttribute("id", QString::number(t.id));
        _writer.writeAttribute("name", t.name);
        _writer.writeAttribute("rate", t.rate.toStoreable());
    }

    _writer.writeEndElement();

}

////////// Manager //////////

RewardsProgramManager::RewardsProgramManager() :
    m_nextId(0)
{
}

RewardsProgram* RewardsProgramManager::add(const QString& _name,
                                           const QString& _currency,
                                           const QList<RewardTier>& _tiers)
{
    //Validate the name;
    if (_name.isEmpty())
    {
        ModelException::throwException(tr("The name of the rewards program cannot be empty"), this);
    }

    //Check if the currency exists
    CurrencyManager::instance()->get(_currency);

    //Validate the tiers
    QString v = RewardsProgram::validateTiers(_tiers);

    if (!v.isEmpty())
    {
        ModelException::throwException(v, this);
    }

    //Add the program
    RewardsProgram* p = new RewardsProgram();
    p->m_id         = m_nextId++;
    p->m_name       = _name;
    p->m_currency   = _currency;
    p->m_tiers      = _tiers;
    m_programs[p->m_id] = p;

    emit modified();
    emit programAdded(p);

    connect(p, SIGNAL(modified()), this, SLOT(onModified()));

    return p;
}

RewardsProgram* RewardsProgramManager::get(int _id) const
{
    if (m_programs.contains(_id))
    {
        return m_programs[_id];
    }
    else
    {
        ModelException::throwException(tr("No such rewards program: %1").arg(_id), this);
        return nullptr; //To please compiler, will never execute
    }
}

void RewardsProgramManager::remove(int _id)
{
    if (m_programs.contains(_id))
    {
        RewardsProgram* p = m_programs[_id];

        //Check if it's in use...
        if (p->isInUse())
        {
            ModelException::throwException(tr("The rewards program \"%1\" is in use!").arg(p->name()), this);
        }

        m_programs.remove(_id);
        emit programRemoved(p);
        emit modified();
        p->deleteLater();
    }
    else
    {
        ModelException::throwException(tr("No such rewards program: %1").arg(_id), this);
    }
}

void RewardsProgramManager::onModified()
{
    RewardsProgram* p = qobject_cast<RewardsProgram*>(sender());

    if (p)
    {
        emit programModified(p);
        emit modified();
    }
}

void RewardsProgramManager::load(QXmlStreamReader& _reader)
{
    unload();

    // While not at end
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == RewardsProgramManager::TAG))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == RewardsProgram::TAG)
        {
            RewardsProgram* o = new RewardsProgram();
            o->load(_reader);
            m_nextId = std::max(m_nextId, o->m_id + 1);
            m_programs[o->m_id] = o;
            connect(o, SIGNAL(modified()), this, SLOT(onModified()));
        }

        _reader.readNext();
    }
}

void RewardsProgramManager::save(QXmlStreamWriter& _writer) const
{
    for (RewardsProgram* o : m_programs)
    {
        o->save(_writer);
    }
}

void RewardsProgramManager::unload()
{
    for (RewardsProgram* i : m_programs)
    {
        i->deleteLater();
    }

    m_programs.clear();
    m_nextId = 0;
}


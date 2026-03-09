#include <algorithm>
#include <array>
#include <memory>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

enum class Faction
{
    Red,
    Blue
};

constexpr std::size_t faction_index(Faction faction)
{
    return (faction == Faction::Red) ? 0u : 1u;
}

constexpr std::string_view to_string(Faction faction)
{
    switch (faction)
    {
        case Faction::Red:  return "Red";
        case Faction::Blue: return "Blue";
    }

    return "Unknown";
}

class GameWorld;
struct GameContext;

class IGameObject
{
public:
    virtual ~IGameObject() = default;



    virtual std::string_view name() const = 0;
    virtual Faction faction() const = 0;

    virtual bool isAlive() const = 0;
    virtual int  power() const = 0;

    virtual void receiveDamage(int damage) = 0;
    virtual void takeTurn(GameContext& context) = 0;
};








class TurnBasedObject : public virtual IGameObject
{
public:
    void takeTurn(GameContext& context) final
    {
        if (!isAlive())
        {
            return;
        }

        onTurnStarted(context);

        if (canAct(context))
        {
            performTurn(context);
        }

        onTurnFinished(context);
    }

protected:
    virtual void onTurnStarted(GameContext&) {}
    virtual bool canAct(const GameContext&) const { return isAlive(); }
    virtual void performTurn(GameContext& context) = 0;
    virtual void onTurnFinished(GameContext&) {}
};

struct GameContext
{
    GameWorld& world;
    int turn;
};

class GameWorld
{
public:
    void addRoot(const std::shared_ptr<IGameObject>& object)
    {
        if (object)
        {
            m_roots.push_back(object);
        }
    }



    std::shared_ptr<IGameObject> findWeakestEnemy(Faction faction) const
    {
        std::shared_ptr<IGameObject> result;

        for (const auto& object : m_roots)
        {
            if (!object->isAlive() || object->faction() == faction)
            {
                continue;
            }

            if (!result || object->power() < result->power())
            {
                result = object;
            }
        }

        return result;
    }



    std::shared_ptr<IGameObject> findStrongestEnemy(Faction faction) const
    {
        std::shared_ptr<IGameObject> result;

        for (const auto& object : m_roots)
        {
            if (!object->isAlive() || object->faction() == faction)
            {
                continue;
            }

            if (!result || object->power() > result->power())
            {
                result = object;
            }
        }

        return result;
    }



    void addResources(Faction faction, int amount)
    {
        m_resources[faction_index(faction)] += amount;
    }

    int resources(Faction faction) const
    {
        return m_resources[faction_index(faction)];
    }



    bool hasConflict() const
    {
        bool redAlive  = false;
        bool blueAlive = false;

        for (const auto& object : m_roots)
        {
            if (!object->isAlive())
            {
                continue;
            }

            if (object->faction() == Faction::Red)
            {
                redAlive = true;
            }
            else if (object->faction() == Faction::Blue)
            {
                blueAlive = true;
            }
        }

        return redAlive && blueAlive;
    }



    void runTurn()
    {
        ++m_turn;

        std::print("\n================ TURN {} ================\n", m_turn);

        GameContext context{ *this, m_turn };

        for (auto& object : m_roots)
        {
            if (object->isAlive())
            {
                object->takeTurn(context);
            }
        }

        printState();
    }



    void printState() const
    {
        std::print("\nWorld state after turn {}\n", m_turn);
        std::print("Resources: Red = {}, Blue = {}\n",
                   resources(Faction::Red),
                   resources(Faction::Blue));

        for (const auto& object : m_roots)
        {
            std::print("  {:<20} | faction = {:<4} | power = {:<3} | alive = {}\n",
                       object->name(),
                       to_string(object->faction()),
                       object->power(),
                       object->isAlive() ? "yes" : "no");
        }
    }

private:
    int m_turn = 0;
    std::vector<std::shared_ptr<IGameObject>> m_roots;
    std::array<int, 2> m_resources{ 200, 200 };
};





class StrategicObject : public TurnBasedObject
{
public:
    StrategicObject(std::string name, Faction faction, int hitPoints, int attack)
        : m_name(std::move(name))
        , m_faction(faction)
        , m_hitPoints(hitPoints)
        , m_attack(attack)
    {
    }



    std::string_view name() const override
    {
        return m_name;
    }

    Faction faction() const override
    {
        return m_faction;
    }

    bool isAlive() const override
    {
        return m_hitPoints > 0;
    }

    int power() const override
    {
        return isAlive() ? m_attack : 0;
    }

    void receiveDamage(int damage) override
    {
        if (!isAlive())
        {
            return;
        }

        damage = std::max(0, damage);
        m_hitPoints = std::max(0, m_hitPoints - damage);

        std::print("{} takes {} damage. HP = {}\n", m_name, damage, m_hitPoints);

        if (m_hitPoints == 0)
        {
            std::print("{} is destroyed.\n", m_name);
        }
    }

protected:
    int attackValue() const
    {
        return m_attack;
    }

private:
    std::string m_name;
    Faction m_faction;
    int m_hitPoints;
    int m_attack;
};

class Infantry final : public StrategicObject
{
public:
    using StrategicObject::StrategicObject;

private:
    void performTurn(GameContext& context) override
    {
        auto target = context.world.findWeakestEnemy(faction());

        if (!target)
        {
            return;
        }

        const int damage = attackValue();

        std::print("[Turn {}] Infantry {} attacks {} for {}\n",
                   context.turn, name(), target->name(), damage);

        target->receiveDamage(damage);
    }
};

class Archer final : public StrategicObject
{
public:
    using StrategicObject::StrategicObject;

private:
    void performTurn(GameContext& context) override
    {
        auto target = context.world.findWeakestEnemy(faction());

        if (!target)
        {
            return;
        }

        const int damage = attackValue() + 5;

        std::print("[Turn {}] Archer {} shoots {} for {}\n",
                   context.turn, name(), target->name(), damage);

        target->receiveDamage(damage);
    }
};

class Cavalry final : public StrategicObject
{
public:
    using StrategicObject::StrategicObject;

private:
    void performTurn(GameContext& context) override
    {
        auto target = context.world.findStrongestEnemy(faction());

        if (!target)
        {
            return;
        }

        int damage = attackValue();

        if (context.turn % 2 == 1)
        {
            damage += 10;
            std::print("[Turn {}] Cavalry {} performs a charge on {} for {}\n",
                       context.turn, name(), target->name(), damage);
        }
        else
        {
            std::print("[Turn {}] Cavalry {} attacks {} for {}\n",
                       context.turn, name(), target->name(), damage);
        }

        target->receiveDamage(damage);
    }
};

class Tower final : public StrategicObject
{
public:
    using StrategicObject::StrategicObject;

private:
    void performTurn(GameContext& context) override
    {
        auto target = context.world.findStrongestEnemy(faction());

        if (!target)
        {
            return;
        }

        const int damage = attackValue();

        std::print("[Turn {}] Tower {} fires at {} for {}\n",
                   context.turn, name(), target->name(), damage);

        target->receiveDamage(damage);
    }
};

class Mine final : public StrategicObject
{
public:
    Mine(std::string name, Faction faction, int hitPoints, int income)
        : StrategicObject(std::move(name), faction, hitPoints, 0)
        , m_income(income)
    {
    }

private:
    void performTurn(GameContext& context) override
    {
        context.world.addResources(faction(), m_income);

        std::print("[Turn {}] Mine {} generates {} resources for {}\n",
                   context.turn, name(), m_income, to_string(faction()));
    }

private:
    int m_income;
};


class Army final : public TurnBasedObject
{
public:
    Army(std::string name, Faction faction)
        : m_name(std::move(name))
        , m_faction(faction)
    {
    }

    void add(const std::shared_ptr<IGameObject>& child)
    {
        if (!child)
        {
            return;
        }

        if (child->faction() != m_faction)
        {
            throw std::logic_error("Cannot add enemy object into army");
        }

        m_children.push_back(child);
    }



    std::string_view name() const override
    {
        return m_name;
    }

    Faction faction() const override
    {
        return m_faction;
    }

    bool isAlive() const override
    {
        for (const auto& child : m_children)
        {
            if (child->isAlive())
            {
                return true;
            }
        }

        return false;
    }

    int power() const override
    {
        int total = 0;

        for (const auto& child : m_children)
        {
            total += child->power();
        }

        return total;
    }

    void receiveDamage(int damage) override
    {
        cleanup();

        auto frontline = chooseFrontline();

        if (!frontline)
        {
            std::print("{} has no defenders left.\n", m_name);
            return;
        }

        std::print("{} redirects {} damage to {}\n",
                   m_name, damage, frontline->name());

        frontline->receiveDamage(damage);

        cleanup();
    }

private:
    void onTurnStarted(GameContext&) override
    {
        cleanup();
    }

    void performTurn(GameContext& context) override
    {
        std::print("[Turn {}] Army {} starts coordinated action\n",
                   context.turn, m_name);

        for (auto& child : m_children)
        {
            if (child->isAlive())
            {
                child->takeTurn(context);
            }
        }

        cleanup();
    }

    void onTurnFinished(GameContext&) override
    {
        cleanup();
    }

    void cleanup()
    {
        m_children.erase(
            std::remove_if(
                m_children.begin(),
                m_children.end(),
                [](const std::shared_ptr<IGameObject>& child)
                {
                    return !child || !child->isAlive();
                }),
            m_children.end());
    }

    std::shared_ptr<IGameObject> chooseFrontline()
    {
        std::shared_ptr<IGameObject> result;

        for (auto& child : m_children)
        {
            if (!child->isAlive())
            {
                continue;
            }

            if (!result || child->power() > result->power())
            {
                result = child;
            }
        }

        return result;
    }

private:
    std::string m_name;
    Faction m_faction;
    std::vector<std::shared_ptr<IGameObject>> m_children;
};

class UnitBuilder
{
public:
    enum class Kind
    {
        Infantry,
        Archer,
        Cavalry
    };

    UnitBuilder& named(std::string name)
    {
        m_name = std::move(name);
        return *this;
    }

    UnitBuilder& forFaction(Faction faction)
    {
        m_faction = faction;
        return *this;
    }

    UnitBuilder& asInfantry()
    {
        m_kind = Kind::Infantry;
        m_hitPoints = 120;
        m_attack = 25;
        return *this;
    }

    UnitBuilder& asArcher()
    {
        m_kind = Kind::Archer;
        m_hitPoints = 80;
        m_attack = 20;
        return *this;
    }

    UnitBuilder& asCavalry()
    {
        m_kind = Kind::Cavalry;
        m_hitPoints = 150;
        m_attack = 30;
        return *this;
    }

    UnitBuilder& veteran()
    {
        m_hitPoints += 25;
        m_attack += 8;
        return *this;
    }

    std::shared_ptr<IGameObject> build() const
    {
        switch (m_kind)
        {
            case Kind::Infantry:
                return std::make_shared<Infantry>(m_name, m_faction, m_hitPoints, m_attack);

            case Kind::Archer:
                return std::make_shared<Archer>(m_name, m_faction, m_hitPoints, m_attack);

            case Kind::Cavalry:
                return std::make_shared<Cavalry>(m_name, m_faction, m_hitPoints, m_attack);
        }

        throw std::logic_error("Unknown unit kind");
    }

private:
    std::string m_name = "Unit";
    Faction m_faction = Faction::Red;
    Kind m_kind = Kind::Infantry;
    int m_hitPoints = 120;
    int m_attack = 25;
};

class BuildingBuilder
{
public:
    enum class Kind
    {
        Tower,
        Mine
    };

    BuildingBuilder& named(std::string name)
    {
        m_name = std::move(name);
        return *this;
    }

    BuildingBuilder& forFaction(Faction faction)
    {
        m_faction = faction;
        return *this;
    }

    BuildingBuilder& asTower()
    {
        m_kind = Kind::Tower;
        m_hitPoints = 220;
        m_attack = 35;
        m_income = 0;
        return *this;
    }

    BuildingBuilder& asMine()
    {
        m_kind = Kind::Mine;
        m_hitPoints = 160;
        m_attack = 0;
        m_income = 60;
        return *this;
    }

    BuildingBuilder& fortified()
    {
        m_hitPoints += 40;
        return *this;
    }

    std::shared_ptr<IGameObject> build() const
    {
        switch (m_kind)
        {
            case Kind::Tower:
                return std::make_shared<Tower>(m_name, m_faction, m_hitPoints, m_attack);

            case Kind::Mine:
                return std::make_shared<Mine>(m_name, m_faction, m_hitPoints, m_income);
        }

        throw std::logic_error("Unknown building kind");
    }

private:
    std::string m_name = "Building";
    Faction m_faction = Faction::Red;
    Kind m_kind = Kind::Tower;
    int m_hitPoints = 200;
    int m_attack = 30;
    int m_income = 50;
};

class ScenarioDirector
{
public:
    static GameWorld createDemoScenario()
    {
        GameWorld world;

        auto redArmy = std::make_shared<Army>("Red Vanguard", Faction::Red);
        redArmy->add(UnitBuilder{}.named("R-Inf-1").forFaction(Faction::Red).asInfantry().build());
        redArmy->add(UnitBuilder{}.named("R-Arc-1").forFaction(Faction::Red).asArcher().build());

        auto redReserve = std::make_shared<Army>("Red Reserve", Faction::Red);
        redReserve->add(UnitBuilder{}.named("R-Cav-1").forFaction(Faction::Red).asCavalry().veteran().build());
        redReserve->add(UnitBuilder{}.named("R-Inf-2").forFaction(Faction::Red).asInfantry().build());
        redArmy->add(redReserve);

        auto blueArmy = std::make_shared<Army>("Blue Legion", Faction::Blue);
        blueArmy->add(UnitBuilder{}.named("B-Inf-1").forFaction(Faction::Blue).asInfantry().veteran().build());
        blueArmy->add(UnitBuilder{}.named("B-Arc-1").forFaction(Faction::Blue).asArcher().build());

        auto blueRaiders = std::make_shared<Army>("Blue Raiders", Faction::Blue);
        blueRaiders->add(UnitBuilder{}.named("B-Cav-1").forFaction(Faction::Blue).asCavalry().build());
        blueRaiders->add(UnitBuilder{}.named("B-Inf-2").forFaction(Faction::Blue).asInfantry().build());
        blueArmy->add(blueRaiders);


        world.addRoot(redArmy);
        world.addRoot(blueArmy);

        world.addRoot(BuildingBuilder{}.named("Red Mine").forFaction(Faction::Red).asMine().build());
        world.addRoot(BuildingBuilder{}.named("Blue Mine").forFaction(Faction::Blue).asMine().fortified().build());

        world.addRoot(BuildingBuilder{}.named("Red Tower").forFaction(Faction::Red).asTower().build());
        world.addRoot(BuildingBuilder{}.named("Blue Tower").forFaction(Faction::Blue).asTower().build());

        return world;
    }
};

int main()
{
    GameWorld world = ScenarioDirector::createDemoScenario();

    world.printState();

    while (world.hasConflict())
    {
        world.runTurn();

        if (world.resources(Faction::Red) > 600 || world.resources(Faction::Blue) > 600)
        {
            std::print("\nEconomic domination condition reached.\n");
            break;
        }
    }

    std::print("\nSimulation finished.\n");
}
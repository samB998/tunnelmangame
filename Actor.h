#ifndef ACTOR_H_
#define ACTOR_H_
#include "StudentWorld.h"

#include "GraphObject.h"
class StudentWorld;

class BaseForEverything : public GraphObject {
public:
    BaseForEverything(StudentWorld* world, int imageID, int startX, int startY,
                      Direction dir = right, double size = 1.0, unsigned int depth = 0);
    virtual ~BaseForEverything() = default;
    virtual void doSomething() = 0;
    bool isAlive() const;
    void setDead();

    StudentWorld* getWorld() const;
    virtual bool isProtester() const { return false; }
    virtual void annoy(int amount);

private:
    bool m_alive;
    StudentWorld* m_world;
};



class Barrel : public BaseForEverything {
public:
    Barrel(StudentWorld* world, int startX, int startY);
    virtual void doSomething() override;
};



class Boulder : public BaseForEverything {
public:
    Boulder(StudentWorld* world, int startX, int startY);
    virtual void doSomething() override;

private:
    enum class State { stable, waiting, falling };
    State state;
    int ticks;
};
class Earth : public BaseForEverything {
public:
    Earth(StudentWorld* world, int startX, int startY)
        : BaseForEverything(world, TID_EARTH, startX, startY, right, 0.25, 3) {
        setVisible(true);

    }

    virtual ~Earth() = default;

    virtual void doSomething() override {
    }
};

class SonarKit : public BaseForEverything {
public:
    SonarKit(StudentWorld* world, int startX, int startY, int level);
    virtual void doSomething() override;
private:
    int m_lifetimeTicks;
};

class Squirt : public BaseForEverything {
public:
    Squirt(StudentWorld* world, int startX, int startY, Direction dir);
    virtual void doSomething() override;

private:
    int m_travelDistance;
};

class Tunnelman : public BaseForEverything {
public:
    Tunnelman(StudentWorld* world);
    virtual ~Tunnelman() = default;
    virtual void doSomething() override;

    int getHitPoints() const;
    void decreaseHitPoints(int amount);
    int getWaterUnits() const;
    void increaseWaterUnits(int amount);
    int getSonarChargeCount() const;
    void increaseSonarChargeCount(int amount);
    int getGoldCount() const;
    void increaseGoldCount(int amount);

    void annoy(int amount);

private:
    int m_hitPoints;
    int m_waterUnits;
    int m_sonarCharges;
    int m_goldNuggets;
    void rmvEarthTunnel();
    bool pMove(int x, int y);
    bool fireSquirt();
    bool useSonar();
    void dropGold();
};

class GoldNugget : public BaseForEverything {
public:
    GoldNugget(StudentWorld* world, int startX, int startY, bool visible, bool pickupByProtester, bool permanent);
    virtual void doSomething() override;

private:
    bool m_pickupByProtester;
    bool m_permanent;
    int m_lifetimeTicks;
};


class WaterPool : public BaseForEverything {
public:
    WaterPool(StudentWorld* world, int startX, int startY, int level);
    virtual void doSomething() override;
private:
    int m_lifetimeTicks;
};

class Protester : public BaseForEverything {
protected:
    int m_hitPoints;
    int m_numSquaresToMove;
    int m_ticksSinceLastShout;
    int m_ticksSinceLastTurn;
    int m_restingTicks;
    bool m_leaveOilField;

    double distanceToTunnelman() const;
    bool isTunnelmanInLineOfSight() const;
    Direction getPathToExit();
    bool canMoveInDirection(Direction dir) const;
    bool inLineOfSight() const;
    bool facingTunnelman() const;
    bool tryPerpendicularTurn();
    void setMoveDirection();
    void moveInCurrentDirection();
    virtual void moveToExit();
    Direction directionToMove() const;
    Direction getDirectionToTunnelman() const;
    Direction getPathToTunnelman();


public:
    Protester(StudentWorld* world, int imageID, int hitPoints);
    virtual void doSomething();
    virtual void annoy(int amount);
    virtual void bribeWithGold();
    bool isLeaving() const { return m_leaveOilField; }
    int getTicksSinceLastShout() const { return m_ticksSinceLastShout; }
    int getTicksSinceLastTurn() const { return m_ticksSinceLastTurn; }
    int getRestingTicks() const { return m_restingTicks; }
    int getNumSquaresToMove() const { return m_numSquaresToMove; }
    void setTicksSinceLastShout(int ticks) { m_ticksSinceLastShout = ticks; }
    void setTicksSinceLastTurn(int ticks) { m_ticksSinceLastTurn = ticks; }
    void setRestingTicks(int ticks) { m_restingTicks = ticks; }
    void setNumSquaresToMove(int squares) { m_numSquaresToMove = squares; }
    virtual bool isProtester() const { return true; }
};

class RegularProtester : public Protester {
public:
    RegularProtester(StudentWorld* world);
    virtual void bribeWithGold() override;

};

class HardcoreProtester : public Protester {
public:
    HardcoreProtester(StudentWorld* world);

    virtual void doSomething() override;
    virtual void bribeWithGold() override;

private:
    bool canReachTunnelman(int M) const;
    int m_stareTimer;
};



#endif // ACTOR_H_

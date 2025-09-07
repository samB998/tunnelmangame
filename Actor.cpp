#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
using Direction = GraphObject::Direction;

BaseForEverything::BaseForEverything(StudentWorld* world, int imageID, int startX, int startY,
                                   Direction dir, double size, unsigned int depth)
    : GraphObject(imageID, startX, startY, dir, size, depth)
    , m_alive(true)
    , m_world(world) {
}

bool BaseForEverything::isAlive() const {
    return m_alive;
}

void BaseForEverything::setDead() {
    m_alive = false;
}

StudentWorld* BaseForEverything::getWorld() const {
    return m_world;
}

void BaseForEverything::annoy(int amount) {
}

Barrel::Barrel(StudentWorld* world, int startX, int startY)
    : BaseForEverything(world, TID_BARREL, startX, startY, right, 1.0, 2) {
    setVisible(false);
}

void Barrel::doSomething() {
    if (!isAlive())
        return;

    if (!isVisible()) {
        double distance = getWorld()->distanceToTunnelman(getX(), getY());
        if (distance <= 4.0) {
            setVisible(true);
            return;
        }
    }
    else {
        double distance = getWorld()->distanceToTunnelman(getX(), getY());
        if (distance <= 3.0) {
            setDead();
            getWorld()->playSound(SOUND_FOUND_OIL);
            getWorld()->increaseScore(1000);
            getWorld()->decrementBarrels();
        }
    }
}
Boulder::Boulder(StudentWorld* world, int startX, int startY)
    : BaseForEverything(world, TID_BOULDER, startX, startY, down, 1.0, 1),
      state(State::stable), ticks(0) {
    setVisible(true);
}

void Boulder::doSomething() {
    if (!isAlive()) {
        return;
    }
    bool earthBelow;
    int xCoord = getX();
    int yCoord = getY() - 1;
    if (state == State::stable) {
         earthBelow = false;
        for (int x = xCoord; x < xCoord + 4; x++) {
            if (getWorld()->isEarthAt(x, yCoord)) {
                earthBelow = true;
                break;
            }
        }
        if (!earthBelow) {
            state = State::waiting;
            ticks = 30;
        }
    } else if (state == State::waiting) {
        ticks--;
        if (ticks <= 0) {
            state = State::falling;
            getWorld()->playSound(SOUND_FALLING_ROCK);
        }
    } else if (state == State::falling) {
        int newY = getY() - 1;
        if (newY < 0 || getWorld()->isBlocked(getX(), newY)) {
            setDead();
            return;
        }
        for (auto actor : getWorld()->getActors()) {
            if ((actor->getID() == TID_PLAYER || actor->isProtester()) &&
                std::sqrt(std::pow(actor->getX() - getX(), 2) + std::pow(actor->getY() - newY, 2)) <= 3.0) {
                actor->annoy(100);
                }
        }
        moveTo(getX(), newY);
    }
}
void RegularProtester::bribeWithGold() {
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);

    getWorld()->increaseScore(25);

    m_leaveOilField = true;
    m_restingTicks = 0;
}

GoldNugget::GoldNugget(StudentWorld* world, int startX, int startY, bool visible, bool pickupByProtester, bool permanent)
    : BaseForEverything(world, TID_GOLD, startX, startY, right, 1.0, 2)
    , m_pickupByProtester(pickupByProtester)
    , m_permanent(permanent)
    , m_lifetimeTicks(permanent ? -1 : 100) {
    setVisible(visible);
}

void GoldNugget::doSomething() {
    if (!isAlive())
        return;

    if (!isVisible() && getWorld()->distanceToTunnelman(getX(), getY()) <= 4.0) {
        setVisible(true);
        return;
    }

    if (m_pickupByProtester) {
        for (auto actor : getWorld()->getActors()) {
            if (actor->isProtester() && actor->isAlive()) {
                double dist = sqrt(pow(actor->getX() - getX(), 2) + pow(actor->getY() - getY(), 2));
                if (dist <= 3.0) {

                    Protester* protester = static_cast<Protester*>(actor);
                    protester->bribeWithGold();
                    
                    setDead();
                    return;
                }
            }
        }

        if (!m_permanent) {
            m_lifetimeTicks--;
            if (m_lifetimeTicks <= 0)
                setDead();
        }
    } else {
        if (getWorld()->distanceToTunnelman(getX(), getY()) <= 3.0) {
            setDead();
            getWorld()->playSound(SOUND_GOT_GOODIE);
            getWorld()->increaseScore(10);
            getWorld()->getTunnelman()->increaseGoldCount(1);
            return;
        }
    }
}

SonarKit::SonarKit(StudentWorld* world, int startX, int startY, int level)
    : BaseForEverything(world, TID_SONAR, startX, startY, right, 1.0, 2) {
    setVisible(true);
    m_lifetimeTicks = std::max(100, 300 - 10 * level);
}

void SonarKit::doSomething() {
    if (!isAlive())
        return;

    if (getWorld()->distanceToTunnelman(getX(), getY()) <= 3.0) {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->getTunnelman()->increaseSonarChargeCount(1);
        getWorld()->increaseScore(75);
        return;
    }

    m_lifetimeTicks--;
    if (m_lifetimeTicks <= 0)
        setDead();
}
Squirt::Squirt(StudentWorld* world, int startX, int startY, Direction dir)
    : BaseForEverything(world, TID_WATER_SPURT, startX, startY, dir, 1.0, 1),
      m_travelDistance(4) {
    setVisible(true);
}

void Squirt::doSomething() {
    if (!isAlive())
        return;

    if (getWorld()->annoyProtestersAt(getX(), getY(), 3.0, 2)) {
        setDead();
        return;
    }

    if (getWorld()->annoyProtestersAt(getX(), getY(), 3.0, 2)) {
        setDead();
        return;
    }

    if (m_travelDistance <= 0) {
        setDead();
        return;
    }

    int nextX = getX();
    int nextY = getY();

    switch (getDirection()) {
        case left:
            nextX -= 1;
        break;
        case right:
            nextX += 1;
        break;
        case up:
            nextY += 1;
        break;
        case down:
            nextY -= 1;
        break;
        default:
            break;
    }

    if (nextX < 0 || nextX >= 64 || nextY < 0 || nextY >= 60 ||
        getWorld()->isEarthAt(nextX, nextY) || getWorld()->isBoulderAt(nextX, nextY)) {
        setDead();
        return;
        }

    moveTo(nextX, nextY);
    m_travelDistance--;
}

Tunnelman::Tunnelman(StudentWorld* world)
    : BaseForEverything(world, TID_PLAYER, 30, 60, right, 1.0, 0),
      m_hitPoints(10), m_waterUnits(5), m_sonarCharges(1), m_goldNuggets(0) {
    setVisible(true);
}

void Tunnelman::doSomething() {
    if (!isAlive()) {
        return;
    }
    rmvEarthTunnel();
    int ch;
    if (getWorld()->getKey(ch)) {
        if (ch == KEY_PRESS_ESCAPE) {
            setDead();
            getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
            return;
        }
        if (ch == KEY_PRESS_SPACE) {
            fireSquirt();
            return;
        }
        if (ch == KEY_PRESS_LEFT) {
            if (getDirection() != left) {
                setDirection(left);
            } else {
                pMove(getX() - 1, getY());
            }
            return;
        }
        if (ch == KEY_PRESS_RIGHT) {
            if (getDirection() != right) {
                setDirection(right);
            } else {
                pMove(getX() + 1, getY());
            }
            return;
        }
        if (ch == KEY_PRESS_UP) {
            if (getDirection() != up) {
                setDirection(up);
            } else {
                pMove(getX(), getY() + 1);
            }
            return;
        }
        if (ch == KEY_PRESS_DOWN) {
            if (getDirection() != down) {
                setDirection(down);
            } else {
                pMove(getX(), getY() - 1);
            }
            return;
        }
        if (ch == 'Z' || ch == 'z') {
            if (m_sonarCharges > 0) {
                useSonar();
            }
            return;
        }
        if (ch == KEY_PRESS_TAB) {
            if (m_goldNuggets > 0) {
                dropGold();
                m_goldNuggets--;
            }
            return;
        }
    }
}

bool Tunnelman::pMove(int x, int y) {
    if (x < 0 || x > 60 || y < 0 || y > 60) {
        return false;
    }
    if (getWorld()->isBoulderAt(x, y)) {
        return false;
    }
    moveTo(x, y);
    return true;
}

void Tunnelman::rmvEarthTunnel() {
    bool earthDigged = false;
    for (int x = getX(); x < getX() + 4; ++x) {
        for (int y = getY(); y < getY() + 4; ++y) {
            if (x >= 0 && x < 64 && y >= 0 && y < 60) {
                if (getWorld()->removeEarth(x, y)) {
                    earthDigged = true;
                }
            }
        }
    }
    if (earthDigged) {
        getWorld()->playSound(SOUND_DIG);
    }
}

bool Tunnelman::fireSquirt() {
    if (m_waterUnits <= 0) {
        return false;
    }
    int sX = 0, sY = 0;
    if (getDirection() == left) {
        sX = -4;
    } else if (getDirection() == right) {
        sX = 4;
    } else if (getDirection() == up) {
        sY = 4;
    } else if (getDirection() == down) {
        sY = -4;
    }
    int squirtX = getX() + sX;
    int squirtY = getY() + sY;
    bool hasEarth = getWorld()->isEarthAt(squirtX, squirtY);
    bool nearBoulder = getWorld()->isBoulderNearby(squirtX, squirtY, 3.0);
    if (squirtX < 0 || squirtX > 60 || squirtY < 0 || squirtY > 60 || hasEarth || nearBoulder) {
        getWorld()->playSound(SOUND_PLAYER_SQUIRT);
        m_waterUnits--;
        return false;
    }
    getWorld()->addActor(new Squirt(getWorld(), squirtX, squirtY, getDirection()));
    getWorld()->playSound(SOUND_PLAYER_SQUIRT);
    m_waterUnits--;
    return true;
}

bool Tunnelman::useSonar() {
    if (m_sonarCharges > 0) {
        getWorld()->revealHiddenObjects(getX(), getY(), 12.0);
        getWorld()->playSound(SOUND_SONAR);
        --m_sonarCharges;
        return true;
    }
    return false;
}

void Tunnelman::dropGold() {
    getWorld()->addActor(new GoldNugget(getWorld(), getX(), getY(), true, true, false));
}


int Tunnelman::getHitPoints() const {
    return m_hitPoints;
}

void Tunnelman::decreaseHitPoints(int amount) {
    m_hitPoints -= amount;
    if (m_hitPoints <= 0) {
        setDead();
        getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
    }
}

int Tunnelman::getWaterUnits() const {
    return m_waterUnits;
}

void Tunnelman::increaseWaterUnits(int amount) {
    m_waterUnits += amount;
}

int Tunnelman::getSonarChargeCount() const {
    return m_sonarCharges;
}

void Tunnelman::increaseSonarChargeCount(int amount) {
    m_sonarCharges += amount;
}

int Tunnelman::getGoldCount() const {
    return m_goldNuggets;
}

void Tunnelman::increaseGoldCount(int amount) {
    m_goldNuggets += amount;
}

void Tunnelman::annoy(int amount) {
    decreaseHitPoints(amount);
}

WaterPool::WaterPool(StudentWorld* world, int startX, int startY, int level)
    : BaseForEverything(world, TID_WATER_POOL, startX, startY, right, 1.0, 2) {
    setVisible(true);
    m_lifetimeTicks = std::max(100, 300 - 10 * level);
}

void WaterPool::doSomething() {
    if (!isAlive())
        return;

    if (getWorld()->distanceToTunnelman(getX(), getY()) <= 3.0) {
        setDead();
        getWorld()->playSound(SOUND_GOT_GOODIE);
        getWorld()->getTunnelman()->increaseWaterUnits(5);
        getWorld()->increaseScore(100);
        return;
    }

    m_lifetimeTicks--;
    if (m_lifetimeTicks <= 0)
        setDead();
}

Protester::Protester(StudentWorld* world, int imageID, int hitPoints)
    : BaseForEverything(world, imageID, 60, 60, left, 1.0, 0)
    , m_hitPoints(hitPoints)
    , m_numSquaresToMove(rand() % 53 + 8)
    , m_ticksSinceLastShout(0)
    , m_ticksSinceLastTurn(0)
    , m_leaveOilField(false) {
    setVisible(true);
    int level = world->getLevel();
    m_restingTicks = std::max(0, (int)(3 - level/4));
}


void Protester::doSomething() {
    if (!isAlive())
        return;

    if (m_restingTicks > 0) {
        m_restingTicks--;
        return;
    }

    m_ticksSinceLastShout++;
    m_ticksSinceLastTurn++;

    if (m_leaveOilField) {
        if (getX() == 60 && getY() == 60) {
            setDead();
            return;
        }
        moveToExit();
        m_restingTicks = std::max(0, (int)(3 - getWorld()->getLevel()/4));
        return;
    }

    if (facingTunnelman() && getWorld()->distanceToTunnelman(getX(), getY()) <= 4.0 && m_ticksSinceLastShout >= 15) {
        getWorld()->playSound(SOUND_PROTESTER_YELL);
        getWorld()->getTunnelman()->annoy(2);
        m_ticksSinceLastShout = 0;
        m_restingTicks = std::max(0, (int)(3 - getWorld()->getLevel()/4));
        return;
    }

    if (inLineOfSight() && getWorld()->distanceToTunnelman(getX(), getY()) > 4.0) {
        Direction dirToTunnelman = getDirectionToTunnelman();

        if (canMoveInDirection(dirToTunnelman)) {
            setDirection(dirToTunnelman);
            moveInCurrentDirection();
            m_numSquaresToMove = 0;
            m_restingTicks = std::max(0, (int)(3 - getWorld()->getLevel()/4));
            return;
        }
    }

    m_numSquaresToMove--;
    if (m_numSquaresToMove <= 0) {
        setMoveDirection();
    } else if (tryPerpendicularTurn()) {
        m_numSquaresToMove = rand() % 53 + 8;
    }

    moveInCurrentDirection();
    m_restingTicks = std::max(0, (int)(3 - getWorld()->getLevel()/4));
}

void Protester::annoy(int amount) {
    if (m_leaveOilField)
        return;

    m_hitPoints -= amount;

    if (m_hitPoints <= 0) {
        m_leaveOilField = true;
        getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
        m_restingTicks = 0;

        if (amount >= 100)
            getWorld()->increaseScore(500);
        else
            getWorld()->increaseScore(100);
    } else {
        getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
        int level = getWorld()->getLevel();
        m_restingTicks = std::max(50, 100 - level * 10);
    }
}

void HardcoreProtester::bribeWithGold() {
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);

    getWorld()->increaseScore(50);

    int level = getWorld()->getLevel();
    m_stareTimer = std::max(50, 100 - level * 10);
}

void Protester::moveToExit() {
    Direction nextMove = getPathToExit();

    setDirection(nextMove);
    moveInCurrentDirection();
}
void Protester::bribeWithGold() {
    m_leaveOilField = true;
    getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
    getWorld()->increaseScore(25);

    int level = getWorld()->getLevel();
    m_restingTicks = 0;
}

bool Protester::canMoveInDirection(Direction dir) const {
    int nextX = getX();
    int nextY = getY();
    switch (dir) {
        case up: nextY++; break;
        case down: nextY--; break;
        case left: nextX--; break;
        case right: nextX++; break;
    }
    if (nextX < 0 || nextX > 60 || nextY < 0 || nextY > 60)
        return false;
    return !getWorld()->isBlocked(nextX, nextY);
}

bool Protester::inLineOfSight() const {
return isTunnelmanInLineOfSight();

}

bool Protester::facingTunnelman() const {
    Direction facingDir = getDirection();
    int tunnelmanX = getWorld()->getTunnelman()->getX();
    int tunnelmanY = getWorld()->getTunnelman()->getY();

    if (facingDir == left && tunnelmanX < getX() && abs(tunnelmanY - getY()) <= 4)
        return true;
    if (facingDir == right && tunnelmanX > getX() && abs(tunnelmanY - getY()) <= 4)
        return true;
    if (facingDir == up && tunnelmanY > getY() && abs(tunnelmanX - getX()) <= 4)
        return true;
    if (facingDir == down && tunnelmanY < getY() && abs(tunnelmanX - getX()) <= 4)
        return true;

    return false;
}

bool Protester::tryPerpendicularTurn() {
    if (m_ticksSinceLastTurn < 200)
        return false;

    Direction currentDir = getDirection();
    std::vector<Direction> validDirs;

    if (currentDir == left || currentDir == right) {
        if (canMoveInDirection(up))
            validDirs.push_back(up);
        if (canMoveInDirection(down))
            validDirs.push_back(down);
    } else {
        if (canMoveInDirection(left))
            validDirs.push_back(left);
        if (canMoveInDirection(right))
            validDirs.push_back(right);
    }

    if (!validDirs.empty()) {
        setDirection(validDirs[rand() % validDirs.size()]);
        m_ticksSinceLastTurn = 0;
        return true;
    }

    return false;
}

void Protester::setMoveDirection() {
    std::vector<Direction> validDirs;
    if (canMoveInDirection(up)) validDirs.push_back(up);
    if (canMoveInDirection(down)) validDirs.push_back(down);
    if (canMoveInDirection(left)) validDirs.push_back(left);
    if (canMoveInDirection(right)) validDirs.push_back(right);

    if (!validDirs.empty()) {
        setDirection(validDirs[rand() % validDirs.size()]);
        m_numSquaresToMove = rand() % 53 + 8;
    }
}

void Protester::moveInCurrentDirection() {
    int nextX = getX();
    int nextY = getY();

    switch (getDirection()) {
        case up: nextY++; break;
        case down: nextY--; break;
        case left: nextX--; break;
        case right: nextX++; break;
    }

    if (canMoveInDirection(getDirection()))
        moveTo(nextX, nextY);
    else
        m_numSquaresToMove = 0;
}

RegularProtester::RegularProtester(StudentWorld* world)
    : Protester(world, TID_PROTESTER, 5) {
}
HardcoreProtester::HardcoreProtester(StudentWorld* world)
    : Protester(world, TID_HARD_CORE_PROTESTER, 20)
    , m_stareTimer(0) {
}

void HardcoreProtester::doSomething() {
    if (!isAlive())
        return;

    if (getRestingTicks() > 0) {
        setRestingTicks(getRestingTicks() - 1);
        return;
    }

    if (m_stareTimer > 0) {
        m_stareTimer--;
        return;
    }

    if (isLeaving()) {
        if (getX() == 60 && getY() == 60) {
            setDead();
            return;
        }
        moveToExit();
        return;
    }

    setTicksSinceLastShout(getTicksSinceLastShout() + 1);
    setTicksSinceLastTurn(getTicksSinceLastTurn() + 1);

    if (facingTunnelman() && getWorld()->distanceToTunnelman(getX(), getY()) <= 4.0 && getTicksSinceLastShout() >= 15) {
        getWorld()->playSound(SOUND_PROTESTER_YELL);
        getWorld()->getTunnelman()->annoy(2);
        setTicksSinceLastShout(0);
        return;
    }

    if (getWorld()->distanceToTunnelman(getX(), getY()) > 4.0) {
        int M = 16 + getWorld()->getLevel() * 2;
        if (canReachTunnelman(M)) {
Direction nextMove = getPathToTunnelman();

            setDirection(nextMove);
            moveInCurrentDirection();
            return;
        }
    }

    if (inLineOfSight() && getWorld()->distanceToTunnelman(getX(), getY()) > 4.0) {
        Direction dirToTunnelman = getDirectionToTunnelman();
        if (canMoveInDirection(dirToTunnelman)) {
            setDirection(dirToTunnelman);
            moveInCurrentDirection();
            setNumSquaresToMove(0);
            return;
        }
    }

    setNumSquaresToMove(getNumSquaresToMove() - 1);
    if (getNumSquaresToMove() <= 0) {
        setMoveDirection();
    } else if (tryPerpendicularTurn()) {
        setNumSquaresToMove(rand() % 53 + 8);
    }

    moveInCurrentDirection();
}


bool HardcoreProtester::canReachTunnelman(int M) const {
    int startX = getX();
    int startY = getY();
    int tunnelmanX = getWorld()->getTunnelman()->getX();
    int tunnelmanY = getWorld()->getTunnelman()->getY();

    if (sqrt(pow(startX - tunnelmanX, 2) + pow(startY - tunnelmanY, 2)) <= M) {
        return true;
    }

    int visited[64][60] = {0};
    int queueX[64 * 60];
    int queueY[64 * 60];
    int queueSteps[64 * 60];
    int front = 0, back = 0;

    queueX[back] = startX;
    queueY[back] = startY;
    queueSteps[back] = 0;
    back++;
    visited[startX][startY] = 1;

    while (front < back) {
        int currentX = queueX[front];
        int currentY = queueY[front];
        int steps = queueSteps[front];
        front++;

        if (steps >= M) {
            continue;
        }

        int directions[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        for (int i = 0; i < 4; i++) {
            int newX = currentX + directions[i][0];
            int newY = currentY + directions[i][1];

            if (newX >= 0 && newX < 64 && newY >= 0 && newY < 60 &&
                !visited[newX][newY] && !getWorld()->isBlocked(newX, newY)) {
                if (newX == tunnelmanX && newY == tunnelmanY) {
                    return true;
                }
                queueX[back] = newX;
                queueY[back] = newY;
                queueSteps[back] = steps + 1;
                back++;
                visited[newX][newY] = 1;
                }
        }
    }

    return false;
}






double Protester::distanceToTunnelman() const {
    int tunnelmanX = getWorld()->getTunnelman()->getX();
    int tunnelmanY = getWorld()->getTunnelman()->getY();
    return sqrt(pow(getX() - tunnelmanX, 2) + pow(getY() - tunnelmanY, 2));
}

GraphObject::Direction Protester::getDirectionToTunnelman() const {
    int tunnelmanX = getWorld()->getTunnelman()->getX();
    int tunnelmanY = getWorld()->getTunnelman()->getY();

    if (abs(getX() - tunnelmanX) < abs(getY() - tunnelmanY))
        return (tunnelmanY > getY()) ? up : down;
    return (tunnelmanX > getX()) ? right : left;
}

bool Protester::isTunnelmanInLineOfSight() const {
    int tunnelmanX = getWorld()->getTunnelman()->getX();
    int tunnelmanY = getWorld()->getTunnelman()->getY();
    if (getX() == tunnelmanX) {
        int yStart = std::min(getY(), tunnelmanY) + 1;
        int yEnd = std::max(getY(), tunnelmanY);
        for (int y = yStart; y < yEnd; y++) {
            if (getWorld()->isBlocked(getX(), y)) {
                return false;
            }
        }
        return true;
    } else if (getY() == tunnelmanY) {
        int xStart = std::min(getX(), tunnelmanX) + 1;
        int xEnd = std::max(getX(), tunnelmanX);
        for (int x = xStart; x < xEnd; x++) {
            if (getWorld()->isBlocked(x, getY())) {
                return false;
            }
        }
        return true;
    }
    return false;
}

GraphObject::Direction Protester::getPathToExit() {
    int maze[64][60];
    int qX[3840], qY[3840];
    int qFront = 0, qBack = 0;

    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 60; j++)
            maze[i][j] = -1;

    qX[qBack] = 60;
    qY[qBack] = 60;
    qBack++;
    maze[60][60] = 0;

    while (qFront < qBack) {
        int x = qX[qFront];
        int y = qY[qFront];
        qFront++;

        if (x == getX() && y == getY()) {
            int minDist = maze[x][y];
            if (x > 0 && maze[x-1][y] == minDist - 1) return left;
            if (x < 63 && maze[x+1][y] == minDist - 1) return right;
            if (y > 0 && maze[x][y-1] == minDist - 1) return down;
            if (y < 59 && maze[x][y+1] == minDist - 1) return up;
            return left;
        }

        if (x > 0 && maze[x-1][y] == -1 && !getWorld()->isBlocked(x-1, y)) {
            maze[x-1][y] = maze[x][y] + 1;
            qX[qBack] = x-1;
            qY[qBack] = y;
            qBack++;
        }
        if (x < 63 && maze[x+1][y] == -1 && !getWorld()->isBlocked(x+1, y)) {
            maze[x+1][y] = maze[x][y] + 1;
            qX[qBack] = x+1;
            qY[qBack] = y;
            qBack++;
        }
        if (y > 0 && maze[x][y-1] == -1 && !getWorld()->isBlocked(x, y-1)) {
            maze[x][y-1] = maze[x][y] + 1;
            qX[qBack] = x;
            qY[qBack] = y-1;
            qBack++;
        }
        if (y < 59 && maze[x][y+1] == -1 && !getWorld()->isBlocked(x, y+1)) {
            maze[x][y+1] = maze[x][y] + 1;
            qX[qBack] = x;
            qY[qBack] = y+1;
            qBack++;
        }
    }
    return left;
}

GraphObject::Direction Protester::getPathToTunnelman() {
    int maze[64][60];
    int qX[3840], qY[3840];
    int qFront = 0, qBack = 0;
    int tunnelmanX = getWorld()->getTunnelman()->getX();
    int tunnelmanY = getWorld()->getTunnelman()->getY();

    for (int i = 0; i < 64; i++)
        for (int j = 0; j < 60; j++)
            maze[i][j] = -1;

    qX[qBack] = tunnelmanX;
    qY[qBack] = tunnelmanY;
    qBack++;
    maze[tunnelmanX][tunnelmanY] = 0;

    while (qFront < qBack) {
        int x = qX[qFront];
        int y = qY[qFront];
        qFront++;

        if (x == getX() && y == getY()) {
            int minDist = maze[x][y];
            if (x > 0 && maze[x-1][y] == minDist - 1) return left;
            if (x < 63 && maze[x+1][y] == minDist - 1) return right;
            if (y > 0 && maze[x][y-1] == minDist - 1) return down;
            if (y < 59 && maze[x][y+1] == minDist - 1) return up;
            return left;
        }

        if (x > 0 && maze[x-1][y] == -1 && !getWorld()->isBlocked(x-1, y)) {
            maze[x-1][y] = maze[x][y] + 1;
            qX[qBack] = x-1;
            qY[qBack] = y;
            qBack++;
        }
        if (x < 63 && maze[x+1][y] == -1 && !getWorld()->isBlocked(x+1, y)) {
            maze[x+1][y] = maze[x][y] + 1;
            qX[qBack] = x+1;
            qY[qBack] = y;
            qBack++;
        }
        if (y > 0 && maze[x][y-1] == -1 && !getWorld()->isBlocked(x, y-1)) {
            maze[x][y-1] = maze[x][y] + 1;
            qX[qBack] = x;
            qY[qBack] = y-1;
            qBack++;
        }
        if (y < 59 && maze[x][y+1] == -1 && !getWorld()->isBlocked(x, y+1)) {
            maze[x][y+1] = maze[x][y] + 1;
            qX[qBack] = x;
            qY[qBack] = y+1;
            qBack++;
        }
    }
    return left;
}


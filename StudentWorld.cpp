#include "StudentWorld.h"
#include "Actor.h"
#include <cmath>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sstream>

using namespace std;

StudentWorld::StudentWorld(std::string assetDir)
    : GameWorld(assetDir), m_tunnelman(nullptr), m_ticks(0) {
    for (int x = 0; x < 64; ++x)
        for (int y = 0; y < 60; ++y)
            m_earth[x][y] = nullptr;
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

bool StudentWorld::annoyProtestersAt(int x, int y, double radius, int amount) {
    bool annoyed = false;
    for (auto actor : m_everything) {
        if (actor->isProtester()) {
            double dist = sqrt(pow(actor->getX() - x, 2) + pow(actor->getY() - y, 2));
            if (dist <= radius) {
                actor->annoy(amount);
                annoyed = true;
            }
        }
    }
    return annoyed;
}

int StudentWorld::init() {

    m_ticks = 0;
    int T = std::max(25, 200 - static_cast<int>(getLevel()));
    m_ticksSinceLastProtester = T;

    for (int x = 0; x < 64; ++x) {
        for (int y = 0; y < 60; ++y) {
            if (x >= 30 && x <= 33 && y >= 4 && y <= 59)
                m_earth[x][y] = nullptr;
            else
                m_earth[x][y] = new Earth(this, x, y);
        }
    }
    m_tunnelman = new Tunnelman(this);
    int level = getLevel();
    int G = std::max(5 - level / 2, 2);

    int B = std::min(level / 2 + 2, 9);
    int L = std::min(2 + level, 21);
    m_barrelsLeft = L;
    std::vector<Position> objectPositions;
    for (int i = 0; i < B; ++i) {
        int x, y;
        bool validPosition;
        do {
            x = rand() % 61;
            y = rand() % 37 + 20;
            validPosition = true;
            if (x >= 27 && x <= 33) {
                validPosition = false;
                continue;
            }
            for (const auto& pos : objectPositions) {
                double distance = sqrt(pow(x - pos.x, 2) + pow(y - pos.y, 2));
                if (distance <= 6.0) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);
        for (int i = x; i < x + 4 && i < 64; ++i) {
            for (int j = y; j < y + 4 && j < 60; ++j) {
                delete m_earth[i][j];
                m_earth[i][j] = nullptr;
            }
        }
        Boulder* boulder = new Boulder(this, x, y);
        m_everything.push_back(boulder);
        objectPositions.push_back({x, y});
    }
    for (int i = 0; i < G; ++i) {
        int x, y;
        bool validPosition;
        do {
            x = rand() % 61;
            y = rand() % 57;
            validPosition = true;
            if (x >= 27 && x <= 33) {
                validPosition = false;
                continue;
            }
            bool hasEarth = true;
            for (int i = x; i < x + 4 && i < 64; ++i) {
                for (int j = y; j < y + 4 && j < 60; ++j) {
                    if (m_earth[i][j] == nullptr) {
                        hasEarth = false;
                        break;
                    }
                }
                if (!hasEarth)
                    break;
            }
            if (!hasEarth) {
                validPosition = false;
                continue;
            }
            for (const auto& pos : objectPositions) {
                double distance = sqrt(pow(x - pos.x, 2) + pow(y - pos.y, 2));
                if (distance <= 6.0) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);
        GoldNugget* nugget = new GoldNugget(this, x, y, true, false, true);
        nugget->setVisible(false);
        m_everything.push_back(nugget);
        objectPositions.push_back({x, y});
    }
    for (int i = 0; i < L; ++i) {
        int x, y;
        bool validPosition;
        do {
            x = rand() % 61;
            y = rand() % 57;
            validPosition = true;
            if (x >= 27 && x <= 33) {
                validPosition = false;
                continue;
            }
            bool hasEarth = true;
            for (int i = x; i < x + 4 && i < 64; ++i) {
                for (int j = y; j < y + 4 && j < 60; ++j) {
                    if (m_earth[i][j] == nullptr) {
                        hasEarth = false;
                        break;
                    }
                }
                if (!hasEarth)
                    break;
            }
            if (!hasEarth) {
                validPosition = false;
                continue;
            }
            for (const auto& pos : objectPositions) {
                double distance = sqrt(pow(x - pos.x, 2) + pow(y - pos.y, 2));
                if (distance <= 6.0) {
                    validPosition = false;
                    break;
                }
            }
        } while (!validPosition);
        Barrel* barrel = new Barrel(this, x, y);
        barrel->setVisible(false);
        m_everything.push_back(barrel);
        objectPositions.push_back({x, y});
    }
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
    m_ticks++;
    m_ticksSinceLastProtester++;

    int T = std::max(25, 200 - static_cast<int>(getLevel()));
    int P = std::min(15, static_cast<int>(2 + getLevel() * 1.5));

    if (m_ticksSinceLastProtester >= T &&
        std::count_if(m_everything.begin(), m_everything.end(),
                      [](BaseForEverything* actor) { return actor->isProtester() && actor->isAlive(); }) < P) {

        int probabilityOfHardcore = std::min(90, static_cast<int>(getLevel()) * 10 + 30);
        int randNum = rand() % 100;

        if (randNum < probabilityOfHardcore) {
            HardcoreProtester* protester = new HardcoreProtester(this);
            m_everything.push_back(protester);
        } else {
            RegularProtester* protester = new RegularProtester(this);
            m_everything.push_back(protester);
        }

        m_ticksSinceLastProtester = 0;
                      }

    updateDisplayText();
    m_tunnelman->doSomething();
    if (!m_tunnelman->isAlive()) {
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    for (auto actor : m_everything) {
        if (actor->isAlive()) {
            actor->doSomething();
            if (!m_tunnelman->isAlive()) {
                decLives();
                return GWSTATUS_PLAYER_DIED;
            }
            if (m_barrelsLeft == 0) {
                playSound(SOUND_FINISHED_LEVEL);
                return GWSTATUS_FINISHED_LEVEL;
            }
        }
    }
    int G = getLevel() * 25 + 300;
    if (rand() % G == 0) {
        if (rand() % 5 == 0) {
            SonarKit* sonar = new SonarKit(this, 0, 60, getLevel());
            m_everything.push_back(sonar);
        } else {
            int x, y;
            bool found = false;
            for (int i = 0; i < 100; ++i) {
                x = rand() % 61;
                y = rand() % 61;
                bool areaClear = true;
                for (int xx = x; xx < x + 4 && xx < 64; ++xx) {
                    for (int yy = y; yy < y + 4 && yy < 60; ++yy) {
                        if (isEarthAt(xx, yy)) {
                            areaClear = false;
                            break;
                        }
                    }
                    if (!areaClear)
                        break;
                }
                if (areaClear) {
                    found = true;
                    break;
                }
            }
            if (found) {
                WaterPool* water = new WaterPool(this, x, y, getLevel());
                m_everything.push_back(water);
            }
        }
    }

    auto it = m_everything.begin();
    while (it != m_everything.end()) {
        if (!(*it)->isAlive()) {
            delete *it;
            it = m_everything.erase(it);
        } else {
            ++it;
        }
    }

    if (!m_tunnelman->isAlive()) {
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }
    if (m_barrelsLeft == 0) {
        playSound(SOUND_FINISHED_LEVEL);
        return GWSTATUS_FINISHED_LEVEL;
    }
    return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::updateDisplayText() {
    int score = getScore();
    int level = getLevel();
    int lives = getLives();
    int health = m_tunnelman->getHitPoints() * 10;
    int squirts = m_tunnelman->getWaterUnits();
    int gold = m_tunnelman->getGoldCount();
    int sonar = m_tunnelman->getSonarChargeCount();
    int barrelsLeft = m_barrelsLeft;
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss << "Lvl: " << std::setw(2) << level << "  "
        << "Lives: " << lives << "  "
        << "Hlth: " << setw(3) << health << "%  "
        << "Wtr: " << setw(2) << squirts << "  "
        << "Gld: " << setw(2) << gold << "  "
        << "Oil Left: " << setw(2) << barrelsLeft << "  "
        << "Sonar: " << setw(2) << sonar << "  "
        << "Scr: " << setfill('0') << setw(6) << score;
    setGameStatText(oss.str());
}

void StudentWorld::cleanUp() {
    delete m_tunnelman;
    m_tunnelman = nullptr;
    for (auto actor : m_everything) {
        delete actor;
    }
    m_everything.clear();
    for (int x = 0; x < 64; ++x) {
        for (int y = 0; y < 60; ++y) {
            delete m_earth[x][y];
            m_earth[x][y] = nullptr;
        }
    }
}
GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

bool StudentWorld::removeEarth(int x, int y) {
    if (x < 0 || x >= 64 || y < 0 || y >= 60)
        return false;
    if (m_earth[x][y]) {
        delete m_earth[x][y];
        m_earth[x][y] = nullptr;
        return true;
    }
    return false;
}

bool StudentWorld::isEarthAt(int x, int y) const {
    if (x < 0 || x >= 64 || y < 0 || y >= 60)
        return false;
    return m_earth[x][y] != nullptr;
}

bool StudentWorld::isBoulderAt(int x, int y) const {
    for (auto actor : m_everything) {
        if (actor->getID() == TID_BOULDER) {
            if (actor->getX() == x && actor->getY() == y)
                return true;
        }
    }
    return false;
}

bool StudentWorld::isBlocked(int x, int y) const {
    if (isEarthAt(x, y))
        return true;
    for (auto actor : m_everything) {
        if (actor->getID() == TID_BOULDER) {
            if (actor->getX() <= x && x < actor->getX() + 4 &&
                actor->getY() <= y && y < actor->getY() + 4)
                return true;
        }
    }
    return false;
}

bool StudentWorld::isBoulderNearby(int x, int y, double radius) const {
    for (auto actor : m_everything) {
        if (actor->getID() == TID_BOULDER) {
            double dist = sqrt(pow(actor->getX() - x, 2) + pow(actor->getY() - y, 2));
            if (dist <= radius)
                return true;
        }
    }
    return false;
}

void StudentWorld::addActor(BaseForEverything* actor) {
    m_everything.push_back(actor);
}

void StudentWorld::revealHiddenObjects(int x, int y, double radius) {
    for (auto actor : m_everything) {
        if (!actor->isVisible()) {
            double dist = sqrt(pow(actor->getX() - x, 2) + pow(actor->getY() - y, 2));
            if (dist <= radius)
                actor->setVisible(true);
        }
    }
}
double StudentWorld::distanceToTunnelman(int x, int y) const {
    int tunnelmanX = m_tunnelman->getX();
    int tunnelmanY = m_tunnelman->getY();
    return sqrt(pow(x - tunnelmanX, 2) + pow(y - tunnelmanY, 2));
}


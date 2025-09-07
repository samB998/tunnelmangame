#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <vector>

class BaseForEverything;
class Tunnelman;
class Earth;

class StudentWorld : public GameWorld {
public:
    struct Position {
        int x;
        int y;
    };

    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();

    bool removeEarth(int x, int y);
    bool isEarthAt(int x, int y) const;
    bool isBoulderAt(int x, int y) const;
    bool isBlocked(int x, int y) const;
    bool isBoulderNearby(int x, int y, double radius) const;
    void addActor(BaseForEverything* actor);
    void revealHiddenObjects(int x, int y, double radius);
    bool annoyProtestersAt(int x, int y, double radius, int amount);
    void decrementBarrels() { m_barrelsLeft--; }
    Tunnelman* getTunnelman() const { return m_tunnelman; }
    const std::vector<BaseForEverything*>& getActors() const { return m_everything; }
    double distanceToTunnelman(int x, int y) const;


private:
    void updateDisplayText();
    Tunnelman* m_tunnelman;
    std::vector<BaseForEverything*> m_everything;
    Earth* m_earth[64][60];
    int m_ticks;
    int m_barrelsLeft;

    int m_ticksSinceLastProtester;
};

#endif // STUDENTWORLD_H_


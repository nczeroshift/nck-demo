
#ifndef DS_SCENE_MAP_H
#define DS_SCENE_MAP_H

#include "../dsStage.h"
#include "nckEffect.h"

class BezNode {
public:
    BezNode(const Math::Vec3 & l, const Math::Vec3 & c, const Math::Vec3 & r);

    Math::Vec3 left;
    Math::Vec3 center;
    Math::Vec3 right;

    static float getDistance(const BezNode & a, const BezNode & b);
    static Math::Vec3 getValue(const BezNode & a, const BezNode & b, float t);
};

class BezCurve {
public:
    BezCurve();
    void Set(std::vector<BezNode> n);
    Math::Vec3 Get(float alpha);
    std::vector<BezNode> nodes;
    std::vector<float> distance;
    float totalDistance;
};

class dsSceneMap : public DS::Stage, public Scene::Processor {
public:
    dsSceneMap(DS::Data * data);
    virtual ~dsSceneMap();
    void Load();
    void Render(int64_t start, int64_t end, int64_t time);
    void RenderFBO(int64_t start, int64_t end, int64_t time);

    static Math::Vec2 getUser1Position() { return user1Position; }
    static Math::Vec2 getUser2Position() { return user2Position; }
    static Math::Vec2 getUser3Position() { return user3Position; }
    static Math::Vec2 getTPPosition() { return planePosition; }

private:
    
    Scene::Texture * HandleTexture(Scene::Texture * tex);
    void HandleFinish(BXON::Map * map, Scene::Compound * compound);
    void updateStuff(int64_t start, int64_t end, int64_t time);
    Scene::Object * HandleObject(Scene::Object * obj);

    std::vector<Scene::Object*> citiesObj;

    Math::Mat44 projectionMatrix;
    Math::Mat44 viewMatrix;
    DS::Compound * map;
    DS::Compound * plane;
    DS::Compound * guiElements;

    Graph::Program * depth;
    Graph::Program * mapProg;
    Graph::Program * basic;
 
    Graph::Texture2D * blur1Tex;
    Graph::RTManager * blur1RT;
    Graph::Texture2D * blur2Tex;
    Graph::RTManager * blur2RT;

    Scene::Effect_DirectionalBlur * blurEffect;
    Scene::Effect_Invert * invert;

    BezCurve curve;

    static Math::Vec2 user1Position;
    static Math::Vec2 user2Position;
    static Math::Vec2 user3Position;
    static Math::Vec2 planePosition;
};

#endif
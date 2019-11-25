
#ifndef DS_TEST_SCENE_H
#define DS_TEST_SCENE_H

#include "../dsStage.h"

class dsTestScene : public DS::Stage {
public:
    dsTestScene(DS::Data * data);
    virtual ~dsTestScene();
    void Load();
    void Render(int64_t start, int64_t end, int64_t time);
    void RenderFBO(int64_t start, int64_t end, int64_t time);
    void SetColors(const Math::Color4ub & start, const Math::Color4ub & end, bool quadratic = false);
protected:
    DS::Compound * scene;
    Graph::RTManager * manager;
    Graph::Texture2D * tex_direct;
    Graph::Texture2D * tex_position;
    Graph::Texture2D * tex_normal;
    Graph::Program * transform;
};

#endif
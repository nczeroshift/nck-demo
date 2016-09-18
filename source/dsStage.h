
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#ifndef DS_SCENE_H
#define DS_SCENE_H

#include "dsData.h"

_DS_BEGIN

class Stage
{
public:
    Stage(Data * data) { m_Data = data; };
    virtual ~Stage() {};
    virtual void Load() = 0;
    virtual void Render(int64_t start, int64_t end, int64_t time) = 0;
protected:
    Data * m_Data;
};

_DS_END

#endif
#include "dsSceneFloatPopup_ScrollCode.h"
#include "../dsUtils.h"

dsSceneFloatPopup_SC::dsSceneFloatPopup_SC(DS::Data * data) : dsSceneFloatPopup(data) {
    codeTex = NULL;
    codeProg = NULL;
}

dsSceneFloatPopup_SC::~dsSceneFloatPopup_SC() {

}

void dsSceneFloatPopup_SC::Load() {
    if((randomOff = Math::RandomValue(0,2))>1.0)
        codeTex = m_Data->LoadTexture("texture://tex2d_src1.png"); // load to/from change
    else 
        codeTex = m_Data->LoadTexture("texture://tex2d_src2.png"); // load to/from change
    randomOff *= 1e6;

    codeTex->SetFilter(Graph::FILTER_MAGNIFICATION, Graph::FILTER_NEAREST);
    codeTex->SetFilter(Graph::FILTER_MINIFICATION, Graph::FILTER_NEAREST);

    codeProg = m_Data->LoadProgram("shader://codescroll.cpp");
}

void dsSceneFloatPopup_SC::Render(int64_t start, int64_t end, int64_t time) {

    Graph::Device * dev = m_Data->GetGraphicsDevice();
    Gui::ShapeRenderer * shp = m_Data->GetShapeRenderer();

    prepare(start, end, time);
    
    Math::Vec2 pos = Math::LinearInterpolation(showPosA, showPosB, alpha);

    dev->Disable(Graph::STATE_DEPTH_TEST);

    dev->PushMatrix();
    dev->Translate(pos.GetX(), pos.GetY(), 0);

    codeProg->Enable();
    codeProg->SetVariable1f("time", (time - start + randomOff) / 1e6);
    codeProg->SetVariable4f("size", width, height, alpha*height, 0);

    dev->Color(255, 255, 255, 255);
    codeTex->Enable();
    DS::RenderSquare(dev, width, alpha*height, false);
    codeTex->Disable();
    codeProg->Disable();

    drawBorders();

    /*

    dev->MatrixMode(Graph::MATRIX_PROJECTION);
    dev->Identity();
    float rel = m_Data->GetHeight() / m_Data->GetWidth();
    dev->Ortho2D(1920, 1920 * rel, 0, 1);

    dev->MatrixMode(Graph::MATRIX_VIEW);
    dev->Identity();

    dev->MatrixMode(Graph::MATRIX_MODEL);
    dev->Identity();

    float showAlpha = Math::Clamp(0,1,(time - start)/showDuration);
    float hideAlpha = 1.0;// Math::Clamp(0, 1, (end - time) / hideDuration);

    float alpha = showAlpha * hideAlpha;

    Math::Vec2 pos = Math::LinearInterpolation(showPosA, showPosB, alpha);

    dev->Disable(Graph::STATE_DEPTH_TEST);

    dev->PushMatrix();
    dev->Translate(pos.GetX(), pos.GetY(), 0);

    codeProg->Enable();
    codeProg->SetVariable1f("time",(time-start+ randomOff)/1e6);
    codeProg->SetVariable4f("size",width,height, alpha*height,0);

    dev->Color(255, 255, 255, 255);
    codeTex->Enable();
    DS::RenderSquare(dev, width, alpha*height, false);
    codeTex->Disable();
    codeProg->Disable();

    dev->Color(250, 250, 250, 240);
    dev->PushMatrix();
    dev->Translate(0, -8, 0);
    
    DS::RenderSquare(dev, width, 8, false);
   
    dev->PopMatrix();
    DS::RenderSquare(dev, 40, 16, false);
    DS::RenderSquare(dev, 8, alpha*height, false);

    dev->PopMatrix();*/
}

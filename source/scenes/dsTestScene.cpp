
#include "dsTestScene.h"
#include "nckMaterialToProgram.h"

void generateShaderFile(DS::Data * data, Scene::MaterialToProgram * mtp, Scene::Material * mat) {
	std::string genFilename = "shader://generated/" + mat->GetName() + ".cpp";
	if (!Core::FileReader::Exists(genFilename)) {
		std::string shaderSrc = mtp->GenerateToString(mat);
		Core::FileWriter * fw = Core::FileWriter::Open(genFilename);
		fw->Write((uint8_t*)shaderSrc.c_str(), shaderSrc.size());
		SafeDelete(fw);
	}
	Graph::Program * p = data->LoadProgram(genFilename);
	mat->SetProgram(p);
}

dsTestScene::dsTestScene(DS::Data * data) : DS::Stage(data) {
	scene = NULL;
}

dsTestScene::~dsTestScene() {
   
}

void dsTestScene::Load() {
    Graph::Device * const dev = m_Data->GetGraphicsDevice();

	scene = GetData()->LoadCompound("model://lisboa.bxon");

	Scene::MaterialToProgram mtp(dev);
	
	std::vector<Scene::Material*> materials;
	scene->Get()->GetAllMaterials(&materials);
	for (int i = 0; i < materials.size(); i++) {
		Scene::Material * m = materials[i];
		m->SetProgram(mtp.Generate(m));
	}

	

	/*	
	scene = GetData()->LoadCompound("model://seta.bxon");
	transform = GetData()->LoadProgram("shader://bezier.cpp");
	*/
}


void dsTestScene::RenderFBO(int64_t start, int64_t end, int64_t time) {
    Graph::Device * const dev = m_Data->GetGraphicsDevice();
    const float width = m_Data->GetWidth();
    const float height = m_Data->GetHeight();

}

void dsTestScene::Render(int64_t start, int64_t end, int64_t time) {
    Graph::Device * const dev = m_Data->GetGraphicsDevice();
    Gui::ShapeRenderer * const shp = m_Data->GetShapeRenderer();

    const float aspect = m_Data->GetAspect();
    const float width = m_Data->GetWidth();
    const float height = m_Data->GetHeight();
    const float t = (time - start) / 1.0e6;
    float alpha = t / ((end-start) / 1.0e6);
  
	Scene::Camera * cam = scene->Get()->GetCamera("Camera");
	cam->SetAspect(width / height);
	
	dev->Enable(Graph::STATE_DEPTH_TEST);

	dev->MatrixMode(Graph::MATRIX_PROJECTION);
	dev->Identity();
	cam->Enable(Graph::MATRIX_PROJECTION);

	dev->MatrixMode(Graph::MATRIX_VIEW);
	dev->Identity();
	cam->Enable(Graph::MATRIX_VIEW);

	dev->MatrixMode(Graph::MATRIX_MODEL);
	dev->Identity();

	dev->Color(255, 255, 255);
	
	std::vector<Scene::Object*> lampObjs;
	scene->Get()->GetObjectsWithLayer(&lampObjs, Scene::DATABLOCK_LAMP);

	Scene::LampUniforms lUniforms;
	Scene::Lamp::GenerateUniforms(lampObjs, cam->GetMatrix(), &lUniforms);

	std::vector<Scene::Material*> mats;
	scene->Get()->GetAllMaterials(&mats);

	for (int i = 0; i < mats.size(); i++) {
		Scene::Material * mat = mats[i];
		lUniforms.Bind(mat->GetProgram());
	}


	scene->Get()->Render();

	/*Scene::Curve * cv = scene->Get()->GetCurve("BezierCurve");

	std::vector< std::vector<Scene::BezierNode> > nodes = cv->GetSplines();
	for (int i = 0; i < nodes.size(); i++) {
		std::vector<Scene::BezierNode> curveNodes = nodes[i];
		for (int j = 1; j<curveNodes.size(); j++) {
			Scene::BezierNode a = curveNodes[j - 1];
			Scene::BezierNode b = curveNodes[j];

			Math::Vec3 p1 = a.GetCenter();
			Math::Vec3 p2 = a.GetRight();
			Math::Vec3 p3 = b.GetLeft();
			Math::Vec3 p4 = b.GetCenter();

			transform->SetVariable4f("a_c", p1.GetX(), p1.GetY(), p1.GetZ(), 1.0);
			transform->SetVariable4f("a_n", p2.GetX(), p2.GetY(), p2.GetZ(), 1.0);

			transform->SetVariable4f("b_p", p3.GetX(), p3.GetY(), p3.GetZ(), 1.0);
			transform->SetVariable4f("b_c", p4.GetX(), p4.GetY(), p4.GetZ(), 1.0);
		}
	}

	transform->Enable();
	scene->Get()->Render();
	transform->Disable();

	dev->Color(255, 0, 0);

	cv->Render();
	*/
}

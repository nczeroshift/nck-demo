
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#include "dsServer.h"
#include "nckUtils.h"

#include "nckUriCodec.h"
#include "../codecs/JSON.h"

_DS_BEGIN

Server::Server(Data * data) {
    m_Server = NULL;
    m_Data = data;
    opMutex = Core::CreateMutex();
    imageCopyFlag = true;
   
    freeRunFlag = false;
    imageCopyBuffer = NULL;
    imageCompressBuffer = NULL;
}

Server::~Server() {
    SafeDelete(m_Server);
    SafeDelete(imageCopyBuffer);
    SafeArrayDelete(imageCompressBuffer);
    SafeDelete(opMutex);
}

void Server::Start() {
    m_Server = NULL;
    try {
        m_Server = Network::HttpServer::Create(8080);
    }
    catch (const Core::Exception & ex) {
        ex.PrintStackTrace();
        try {
            m_Server = Network::HttpServer::Create(0); // Find an available port
        }
        catch (const Core::Exception & ex2) {
            ex.PrintStackTrace();
            return;
        }
    }
    m_Server->SetCallback(this);
}

bool Server::AllowRequest(Network::HttpRequest * request, Network::HttpResponse * response) {
    return true;
}

bool Server::HandleRequest(Network::HttpRequest * request, Network::HttpResponse * response) {

    if (request->GetMethod() == "GET")
    {
        if (request->HasParameter("screenshot")) {
            imageCopyFlag = false;
            freeRunFlag = true;  // Desligar o sleep e atualizar o shader.
            Core::Chronometer * chron = Core::CreateChronometer();
            chron->Start();
          
            while (!imageCopyFlag && chron->GetElapsedTime() < 1.0e6) {
                Core::Thread::Wait(1);
            }
     
            if (imageCopyFlag == true) {
                opMutex->Lock();
                size_t size = imageCopyBuffer->Save(Core::IMAGE_TYPE_JPEG, 99, &imageCompressBuffer);
                response->GetBuffer()->Push(imageCompressBuffer, size);
                response->SetHeader("Content-Type", "image/jpeg");
                response->SetStatusCode(200);
                opMutex->Unlock();
            }
            else
                response->SetStatusCode(500);
          
            SafeDelete(chron);
        }
        else if (request->HasParameter("resources")) {
            std::vector<std::string> progStr;
            std::vector<std::string> compStr;
            std::vector<std::string> textStr;

            m_Data->GetProgramKeys(&progStr);
            m_Data->GetTextureKeys(&textStr);
            m_Data->GetCompoundsKeys(&compStr);


            JSONArray arrTex(textStr.size()), arrComp(compStr.size()), arrProg(progStr.size());
            for (int i = 0; i < textStr.size(); i++) arrTex[i] = new JSONValue(textStr[i]);
            for (int i = 0; i < compStr.size(); i++) arrComp[i] = new JSONValue(compStr[i]);
            for (int i = 0; i < progStr.size(); i++) arrProg[i] = new JSONValue(progStr[i]);

            JSONObject respObj;
            JSONValue * objTex = new JSONValue(arrTex), *objComp = new JSONValue(arrComp), *objProg = new JSONValue(arrProg);
            respObj.insert(std::pair<std::string, JSONValue*>("programs", objProg));
            respObj.insert(std::pair<std::string, JSONValue*>("compounds", objComp));
            respObj.insert(std::pair<std::string, JSONValue*>("textures", objTex));

            std::string tmp = JSONValue(respObj).Stringify();
            response->GetBuffer()->Push(tmp);
            response->SetHeader("Content-Type", "application/json");
            response->SetStatusCode(200);
        }
        else if (request->HasParameter("shader_src")) {
            std::string url = request->GetParameter("shader_src");
            freeRunFlag = true; // Desligar o sleep e atualizar o shader.

            if (Core::FileReader::Exists(url)) {
                Core::FileReader * fr = Core::FileReader::Open(url);

                char * buffer = new char[fr->Length()];
                fr->Read(buffer, fr->Length());

                response->GetBuffer()->Push(buffer, fr->Length());

                response->SetHeader("Content-Type", "plain/text");

                SafeArrayDelete(buffer);

                SafeDelete(fr);
                response->SetStatusCode(200);
            }
            else
                response->SetStatusCode(500);
        }



    }
    else if (request->GetMethod() == "POST")
    {
        if (request->HasParameter("update_shader")) {
            std::string shaderPath = request->GetParameter("update_shader");
            std::string content;
            request->GetBuffer()->ToString(&content);

            if (Core::FileReader::Exists(shaderPath)) {
                Core::FileWriter * fw = Core::FileWriter::Open(shaderPath);
                const char * str = content.c_str();
                fw->Write((uint8_t*)str, content.size());
                SafeDelete(fw);
                reloadContents = true;
            }
        }
    }

    return true;
}

void Server::UnsupportedRequest(Network::HttpRequest * request)
{

}


bool Server::FreeRun() {
    bool state = freeRunFlag;
    freeRunFlag = false;
    return state;
}

bool Server::ReloadContents() {
    bool state = reloadContents;
    reloadContents = false;
    return state;
}

void Server::FetchImage(Graph::Device * dev, int width, int height) {
    if (imageCopyFlag)
        return;

    width = width / 16;
    height = height /16;
    width *= 16;
    height *= 16;

    opMutex->Lock();
    if (imageCopyBuffer == NULL || imageCopyBuffer->GetWidth() != width|| imageCopyBuffer->GetHeight()!= height) {
        SafeDelete(imageCopyBuffer);
        SafeArrayDelete(imageCompressBuffer);
     
        imageCopyBuffer = Core::Image::Create(width, height, Core::PIXEL_FORMAT_RGB_8B);
        imageCompressBuffer = new uint8_t[width * height * 3];
    }

    dev->Capture(0, 0, width, height, Graph::FORMAT_RGB_8B, imageCopyBuffer->GetData());
    opMutex->Unlock();
    imageCopyFlag = true;
}

_DS_END

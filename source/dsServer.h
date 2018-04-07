
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#ifndef DS_SERVER_H
#define DS_SERVER_H

#include "dsData.h"
#include "nckHttpServer.h"

_DS_BEGIN

//class ServerRequestHandler;

class Server : public Network::HttpCallbackHandler {
public:
    Server(Data * main);
    ~Server();

    void Start();

    bool FreeRun();
    bool ReloadContents();

    void FetchImage(Graph::Device * dev, int width, int height);


private:

    bool AllowRequest(Network::HttpRequest * request, Network::HttpResponse * response);
    bool HandleRequest(Network::HttpRequest * request, Network::HttpResponse * response);
    void UnsupportedRequest(Network::HttpRequest * request);

    bool reloadContents;
    bool freeRunFlag;
    bool imageCopyFlag;
    Core::Image * imageCopyBuffer;
    uint8_t * imageCompressBuffer;
    Core::Mutex * opMutex;
    Network::HttpServer * m_Server;
    Data * m_Data;
};

_DS_END

#endif
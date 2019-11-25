
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#ifndef DS_SERVER_H
#define DS_SERVER_H

#include "dsData.h"
#include "nckHttpServer.h"
#include <map>

_DS_BEGIN

#define SERVER_OP_RELOAD_SHADER "reloadShader"

class Server;

class ServerOperation {
public:
	ServerOperation(Server * server, const std::string type, const std::string & object) {
		m_Server = server;
		m_Object = object;
		m_Type = type;
		m_Finished = false;
	}
	void SetError(const std::string & error) { m_Error = error; }

	std::string GetObject() { return m_Object; }
	std::string GetError() { return m_Error; }
	std::string GetType() { return m_Type; }

	bool IsFinished() { return m_Finished; }
	void Finish() { m_Finished = true; }
	void WaitToFinish();

private:
	Server * m_Server;
	bool m_Finished;
	std::string m_Type;
	std::string m_Object;
	std::string m_Error;
};

class Server : public Network::HttpCallbackHandler {
public:
    Server(Data * main);
    ~Server();

    void Start();

    bool FreeRun();
    bool ReloadContents();

	void DispatchOperation(ServerOperation * op);
	bool FetchOperation(const std::string & type, ServerOperation ** operation);
	void RemoveOperation(ServerOperation * op);

    void FetchImage(Graph::Device * dev, int width, int height);

private:

    bool AllowRequest(Network::HttpRequest * request, Network::HttpResponse * response);
    bool HandleRequest(Network::HttpRequest * request, Network::HttpResponse * response);
    void UnsupportedRequest(Network::HttpRequest * request);

	Core::Mutex * dispatchOpMutex;
	std::map < std::string, std::list<ServerOperation*>> dispatchOps;

	std::string reloadResult;
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
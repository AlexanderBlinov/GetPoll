#include <fastcgi2/component.h>
#include <fastcgi2/component_factory.h>
#include <fastcgi2/handler.h>
#include <fastcgi2/request.h>

#include <sstream>

#include "WebService.h"


class RequestHandler : virtual public fastcgi::Component, virtual public fastcgi::Handler {
private:
    WebService* service;

public:
    RequestHandler(fastcgi::ComponentContext *context) : fastcgi::Component(context) {
        service = new WebService();
    }

    virtual ~RequestHandler() {}

public:
    virtual void onLoad() {}

    virtual void onUnload() {}

    virtual void handleRequest(fastcgi::Request *request, fastcgi::HandlerContext *context) {
        service->web_service_process_request(request);
    }
};

FCGIDAEMON_REGISTER_FACTORIES_BEGIN()
FCGIDAEMON_ADD_DEFAULT_FACTORY("getpoll_factory", RequestHandler)
FCGIDAEMON_REGISTER_FACTORIES_END()

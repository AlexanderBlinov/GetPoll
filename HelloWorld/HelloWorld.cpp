#include <fastcgi2/component.h>
#include <fastcgi2/component_factory.h>
#include <fastcgi2/handler.h>
#include <fastcgi2/request.h>

#include <iostream>

class HelloWorld : virtual public fastcgi::Component, virtual public fastcgi::Handler {

public:
        HelloWorld(fastcgi::ComponentContext *context) :
                fastcgi::Component(context) {
        }
        virtual ~HelloWorld() {
        }

public:
        virtual void onLoad() {
        }
        virtual void onUnload() {
        }
        virtual void handleRequest(fastcgi::Request *request, fastcgi::HandlerContext *context) {
                request->setHeader("HelloWorld-Header", "Hello, World!");
        }

};

FCGIDAEMON_REGISTER_FACTORIES_BEGIN()
FCGIDAEMON_ADD_DEFAULT_FACTORY("helloworld_factory", HelloWorld)
FCGIDAEMON_REGISTER_FACTORIES_END()

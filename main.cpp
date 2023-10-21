#include <cstdlib>
#include <drogon/WebSocketController.h>
#include <drogon/drogon.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <string>
#include <unordered_map>

#define VERSION 2

#if VERSION == 1
#include "v1/all.hpp"
using v1::clearUserColors;
#elif VERSION == 2
#include "v2/all.hpp"
using v2::clearUserColors;
#else
#error "Unsupported VERSION value. Only 1 and 2 are allowed."
#endif

int main(void) {
  LOG_INFO << "Using version " << VERSION;
  drogon::app().addListener("0.0.0.0", 5555);
  drogon::app().createRedisClient("127.0.0.1", 6379);
  // drogon::app().registerBeginningAdvice(clearUserColors);
  drogon::app().registerPreRoutingAdvice(
      [](const drogon::HttpRequestPtr &req, drogon::AdviceCallback &&acb,
         drogon::AdviceChainCallback &&accb) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->addHeader("Access-Control-Allow-Origin", "*");
        resp->addHeader("Access-Control-Allow-Methods", "*");
        resp->addHeader("Access-Control-Allow-Headers", "*");

        if (req->getMethod() == drogon::Options) {
          resp->setStatusCode(drogon::k200OK);

          acb(resp);
          return;
        }

        accb();
      });

  drogon::app().run();

  return 0;
}

#include "v2/all.hpp"

int main(void) {
  drogon::app()
      .setLogLevel(trantor::Logger::kInfo)
      .addListener("0.0.0.0", 5555)
      .setThreadNum(0) // 0 means max
      .createRedisClient("127.0.0.1", 6379)
      .run();

  return 0;
}

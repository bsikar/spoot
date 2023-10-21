#pragma once

#include <drogon/HttpController.h>
#include <drogon/WebSocketController.h>
#include <drogon/drogon.h>
#include <string>

namespace v1 {

void clearUserColors(void);
std::string getClientIdFromEnv(void);
std::string getClientSecretFromEnv(void);

class ColorWebSocket : public drogon::WebSocketController<ColorWebSocket> {
public:
  void handleNewMessage(const drogon::WebSocketConnectionPtr &ws_conn_ptr,
                        std::string &&message,
                        const drogon::WebSocketMessageType &type) override;

  void handleNewConnection(
      const drogon::HttpRequestPtr &req,
      const drogon::WebSocketConnectionPtr &ws_conn_ptr) override;

  void handleConnectionClosed(
      const drogon::WebSocketConnectionPtr &ws_conn_ptr) override;

  WS_PATH_LIST_BEGIN
  WS_PATH_ADD("/buttons");
  WS_PATH_LIST_END

private:
  std::unordered_map<drogon::WebSocketConnectionPtr, std::string> user_names;
};

class CheckNameController : public drogon::HttpController<CheckNameController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(CheckNameController::checkName, "/check_name", drogon::Get);
  METHOD_LIST_END

  void
  checkName(const drogon::HttpRequestPtr &req,
            std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

class SpotifyAuthController
    : public drogon::HttpController<SpotifyAuthController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SpotifyAuthController::redirectToSpotifyAuth, "/login",
                drogon::Get);
  METHOD_LIST_END

  void redirectToSpotifyAuth(
      const drogon::HttpRequestPtr &req,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

class SpotifyHostController
    : public drogon::HttpController<SpotifyHostController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SpotifyHostController::showSpotifyData, "/host", drogon::Get);
  ADD_METHOD_TO(SpotifyHostController::showTopTracks, "/top_tracks",
                drogon::Get);
  METHOD_LIST_END

  void showSpotifyData(
      const drogon::HttpRequestPtr &req,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
  void showTopTracks(
      const drogon::HttpRequestPtr &req,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

} // namespace v1

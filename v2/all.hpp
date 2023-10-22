#pragma once

#include "../v1/all.hpp"
#include <drogon/HttpController.h>
#include <drogon/WebSocketController.h>
#include <drogon/drogon.h>
#include <optional>
#include <string>

namespace v2 {
using namespace v1;

class SpotifyHostController
    : public drogon::HttpController<SpotifyHostController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SpotifyHostController::getSpotCreds, "/get_creds", drogon::Get);
  ADD_METHOD_TO(SpotifyHostController::topArtists, "/top_artists", drogon::Get);
  METHOD_LIST_END

  void
  getSpotCreds(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);

  void
  topArtists(const drogon::HttpRequestPtr &req,
             std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
  std::optional<drogon::HttpRequestPtr>
  makeRequest(const drogon::HttpRequestPtr &req,
              std::function<void(const drogon::HttpResponsePtr &)> callback,
              const std::string &path);
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
} // namespace v2

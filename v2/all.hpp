#pragma once

#include <drogon/HttpController.h>
#include <drogon/WebSocketController.h>
#include <drogon/drogon.h>
#include <optional>
#include <string>

namespace v2 {

class SpotifyHostController
    : public drogon::HttpController<SpotifyHostController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SpotifyHostController::getSpotCreds, "/get_creds/{1}",
                drogon::Get, drogon::Options);
  ADD_METHOD_TO(SpotifyHostController::topArtists, "/top_artists", drogon::Get,
                drogon::Options);
  METHOD_LIST_END

  void
  getSpotCreds(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback,
               const std::string &difficulty);

  void
  topArtists(const drogon::HttpRequestPtr &req,
             std::function<void(const drogon::HttpResponsePtr &)> &&callback);

private:
  std::string access_token;
  std::string difficulty_level;
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
                drogon::Get, drogon::Options);
  METHOD_LIST_END

  void redirectToSpotifyAuth(
      const drogon::HttpRequestPtr &req,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

class CorsFilter : public drogon::HttpFilter<CorsFilter> {
public:
  virtual void doFilter(const drogon::HttpRequestPtr &req,
                        drogon::FilterCallback &&fcb,
                        drogon::FilterChainCallback &&fccb) override;
};

} // namespace v2

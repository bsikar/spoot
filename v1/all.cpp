#include "all.hpp"
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <unordered_map>

namespace v1 {

void clearUserColors(void) {
  try {
    auto redis_client = drogon::app().getRedisClient();

    if (!redis_client) {
      LOG_ERROR << "Redis client is null.";
      return;
    }

    redis_client->execCommandAsync([](const drogon::nosql::RedisResult &) {},
                                   [](const drogon::nosql::RedisException &ex) {
                                     LOG_ERROR << "Error clearing user_colors: "
                                               << ex.what();
                                   },
                                   "DEL user_colors");
  } catch (const std::exception &e) {
    LOG_ERROR << "Exception caught in clearUserColors: " << e.what();
  } catch (...) {
    LOG_ERROR << "Unknown exception caught in clearUserColors.";
  }
}

std::string getClientIdFromEnv() {
  std::ifstream file(".env");
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream line_stream(line);
    std::string key;
    if (std::getline(line_stream, key, '=')) {
      std::string value;
      if (key == "CLIENT_ID" && std::getline(line_stream, value)) {
        return value;
      }
    }
  }
  return "";
}

std::string getClientSecretFromEnv() {
  std::ifstream file(".env");
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream line_stream(line);
    std::string key;
    if (std::getline(line_stream, key, '=')) {
      std::string value;
      if (key == "CLIENT_SECRET" && std::getline(line_stream, value)) {
        return value;
      }
    }
  }
  return "";
}

void ColorWebSocket::handleNewMessage(
    const drogon::WebSocketConnectionPtr &ws_conn_ptr, std::string &&message,
    const drogon::WebSocketMessageType &type) {
  if (type != drogon::WebSocketMessageType::Text) {
    return;
  }

  auto it = user_names.find(ws_conn_ptr);
  if (it == user_names.end()) {
    return;
  }

  auto user_name = it->second;
  auto redis_client = drogon::app().getRedisClient();
  redis_client->execCommandAsync(
      [](const drogon::nosql::RedisResult &) {},
      [](const drogon::nosql::RedisException &ex) { LOG_ERROR << ex.what(); },
      "HSET user_colors %s %s", user_name.c_str(), message.c_str());
}

void ColorWebSocket::handleNewConnection(
    const drogon::HttpRequestPtr &req,
    const drogon::WebSocketConnectionPtr &ws_conn_ptr) {
  auto user_name = req->getParameter("name");
  if (!user_name.empty()) {
    user_names[ws_conn_ptr] = user_name;
  }
}

void ColorWebSocket::handleConnectionClosed(
    const drogon::WebSocketConnectionPtr &ws_conn_ptr) {
  auto it = user_names.find(ws_conn_ptr);
  if (it == user_names.end()) {
    return;
  }

  auto user_name = it->second;

  auto redis_client = drogon::app().getRedisClient();
  redis_client->execCommandAsync(
      [](const drogon::nosql::RedisResult &) {},
      [](const drogon::nosql::RedisException &ex) { LOG_ERROR << ex.what(); },
      "HDEL user_colors %s", user_name.c_str());

  user_names.erase(ws_conn_ptr);
}

void CheckNameController::checkName(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto name = req->getParameter("name");
  auto redis_client = drogon::app().getRedisClient();
  redis_client->execCommandAsync(
      [callback, redis_client, name](const drogon::nosql::RedisResult &result) {
        if (result.asString() == "1") {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(drogon::k404NotFound);
          callback(resp);
        } else {
          redis_client->execCommandAsync(
              [callback](const drogon::nosql::RedisResult &) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k200OK);
                callback(resp);
              },
              [callback](const drogon::nosql::RedisException &ex) {
                LOG_ERROR << ex.what();
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k500InternalServerError);
                callback(resp);
              },
              "HSET user_colors %s %s", name.c_str(), "");
        }
      },
      [callback](const drogon::nosql::RedisException &ex) {
        LOG_ERROR << ex.what();

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
      },
      "HEXISTS user_colors %s", name.c_str());
}

void SpotifyAuthController::redirectToSpotifyAuth(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  std::string client_id = getClientIdFromEnv();
  if (client_id.empty()) {
    std::cerr << "no .env CLIENT_ID= found" << std::endl;
    return;
  }

  std::string redirect_uri = "http://localhost:3000/play";
  std::string scopes = "user-top-read";

  std::string spotify_auth_url =
      "https://accounts.spotify.com/authorize?response_type=code&client_id=" +
      client_id + "&scope=" + scopes + "&redirect_uri=" + redirect_uri;

  auto resp = drogon::HttpResponse::newRedirectionResponse(spotify_auth_url);
  callback(resp);
}

void SpotifyHostController::showSpotifyData(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto code = req->getParameter("code");

  if (code.empty()) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k400BadRequest);
    resp->setBody(R"({"error": "Code parameter is missing"})");
    callback(resp);
    return;
  }

  auto client =
      drogon::HttpClient::newHttpClient("https://accounts.spotify.com");

  auto req2 = drogon::HttpRequest::newHttpRequest();
  req2->setMethod(drogon::Post);
  req2->setPath("/api/token");

  req2->setContentTypeCode(drogon::CT_APPLICATION_X_FORM);

  std::ostringstream bodyStream;
  bodyStream << "grant_type=authorization_code";
  bodyStream << "&redirect_uri="
             << "http://localhost:3000/play";
  bodyStream << "&code=" << code;

  req2->setBody(bodyStream.str());

  // Note: Basic Auth setup is a bit different in C++ compared to JS.
  // You'd typically base64 encode the "client_id:client_secret" string and
  // put that into your Authorization header:
  std::string authHeader =
      "Basic " + drogon::utils::base64Encode(getClientIdFromEnv() + ":" +
                                             getClientSecretFromEnv());
  req2->addHeader("Authorization", authHeader);

  client->sendRequest(req2,
                      [callback](drogon::ReqResult result,
                                 const drogon::HttpResponsePtr &response) {
                        if (result == drogon::ReqResult::Ok) {
                          auto resp = drogon::HttpResponse::newHttpResponse();
                          resp->setBody(std::string(response->getBody()));
                          callback(resp);
                        } else {
                          auto resp = drogon::HttpResponse::newHttpResponse();
                          resp->setStatusCode(drogon::k500InternalServerError);
                          callback(resp);
                        }
                      });
}
void SpotifyHostController::showTopTracks(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto access_token = req->getParameter("access_token");

  if (access_token.empty()) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k400BadRequest);
    resp->setBody(R"({"error": "Access token is required"})");
    callback(resp);
    return;
  }

  auto client = drogon::HttpClient::newHttpClient("https://api.spotify.com");
  auto spotify_req = drogon::HttpRequest::newHttpRequest();
  spotify_req->setMethod(drogon::Get);
  spotify_req->setPath("/v1/me/top/tracks?limit=15&time_range=short_term");
  spotify_req->addHeader("Authorization", "Bearer " + access_token);

  client->sendRequest(
      spotify_req, [callback](drogon::ReqResult result,
                              const drogon::HttpResponsePtr &response) {
        if (result == drogon::ReqResult::Ok) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          // Parse the body of the Spotify response into a Json::Value object
          Json::Reader reader;
          Json::Value json_data;
          if (reader.parse(std::string(response->getBody()), json_data)) {
            // Extract the artist names
            std::vector<std::string> artists;
            for (const auto &item : json_data["items"]) {
              for (const auto &artist : item["artists"]) {
                auto artist_name = artist["name"].asString();
                bool doesnt_contain = std::find(artists.begin(), artists.end(),
                                                artist_name) == artists.end();
                if (doesnt_contain) {
                  artists.push_back(artist_name);
                  std::cout << artist_name << std::endl;
                }
              }
            }

            // Convert the artists vector into a string
            std::string body;
            for (const auto &artist : artists) {
              body += "<p>" + artist + "</p>";
            }

            resp->setBody(body);
          } else {
            resp->setBody(R"({"error": "Failed to parse Spotify response"})");
          }

          callback(resp);
        } else {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(drogon::k500InternalServerError);
          resp->setBody(R"({"error": "Failed to fetch top tracks"})");
          callback(resp);
        }
      });
}

} // namespace v1

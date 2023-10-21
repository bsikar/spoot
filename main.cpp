#include <drogon/WebSocketController.h>
#include <drogon/drogon.h>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <sstream>
#include <string>
#include <unordered_map>

namespace v1 {

void clear_user_colors(void) {
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
    LOG_ERROR << "Exception caught in clear_user_colors: " << e.what();
  } catch (...) {
    LOG_ERROR << "Unknown exception caught in clear_user_colors.";
  }
}

std::string get_client_id_from_env() {
  std::ifstream file("../.env");
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream lineStream(line);
    std::string key;
    if (std::getline(lineStream, key, '=')) {
      std::string value;
      if (key == "CLIENT_ID" && std::getline(lineStream, value)) {
        return value;
      }
    }
  }
  return "";
}

std::string get_client_secret_from_env() {
  std::ifstream file("../.env");
  std::string line;
  while (std::getline(file, line)) {
    std::istringstream lineStream(line);
    std::string key;
    if (std::getline(lineStream, key, '=')) {
      std::string value;
      if (key == "CLIENT_SECRET" && std::getline(lineStream, value)) {
        return value;
      }
    }
  }
  return "";
}

class color_web_socket : public drogon::WebSocketController<color_web_socket> {
public:
  void handleNewMessage(const drogon::WebSocketConnectionPtr &ws_conn_ptr,
                        std::string &&message,
                        const drogon::WebSocketMessageType &type) override {
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

  void handleNewConnection(
      const drogon::HttpRequestPtr &req,
      const drogon::WebSocketConnectionPtr &ws_conn_ptr) override {
    auto user_name = req->getParameter("name");
    if (!user_name.empty()) {
      user_names[ws_conn_ptr] = user_name;
    }
  }

  void handleConnectionClosed(
      const drogon::WebSocketConnectionPtr &ws_conn_ptr) override {
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

  WS_PATH_LIST_BEGIN
  WS_PATH_ADD("/buttons");
  WS_PATH_LIST_END

private:
  std::unordered_map<drogon::WebSocketConnectionPtr, std::string> user_names;
};

class check_name_controller
    : public drogon::HttpController<check_name_controller> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(check_name_controller::check_name, "/check_name", drogon::Get);
  METHOD_LIST_END

  void
  check_name(const drogon::HttpRequestPtr &req,
             std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    auto name = req->getParameter("name");
    auto redis_client = drogon::app().getRedisClient();
    redis_client->execCommandAsync(
        [callback, redis_client,
         name](const drogon::nosql::RedisResult &result) {
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
};

class SpotifyAuthController
    : public drogon::HttpController<SpotifyAuthController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SpotifyAuthController::redirect_to_spotify_auth, "/login",
                drogon::Get);
  METHOD_LIST_END

  void redirect_to_spotify_auth(
      const drogon::HttpRequestPtr &req,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    std::string client_id = get_client_id_from_env();
    if (client_id.empty()) {
      std::cerr << "no .env CLIENT_ID= found" << std::endl;
      return;
    }

    std::string redirect_uri = "http://localhost:5555/host";
    std::string scopes = "user-read-private user-top-read";

    std::string spotify_auth_url =
        "https://accounts.spotify.com/authorize?response_type=code&client_id=" +
        client_id + "&scope=" + scopes + "&redirect_uri=" + redirect_uri;

    auto resp = drogon::HttpResponse::newRedirectionResponse(spotify_auth_url);
    callback(resp);
  }
};

class SpotifyHostController
    : public drogon::HttpController<SpotifyHostController> {
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(SpotifyHostController::show_spotify_data, "/host", drogon::Get);
  ADD_METHOD_TO(SpotifyHostController::show_top_tracks, "/top_tracks",
                drogon::Get);
  METHOD_LIST_END

  void show_spotify_data(
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
               << "http://localhost:5555/host";
    bodyStream << "&code=" << code;

    req2->setBody(bodyStream.str());

    // Note: Basic Auth setup is a bit different in C++ compared to JS.
    // You'd typically base64 encode the "client_id:client_secret" string and
    // put that into your Authorization header:
    std::string authHeader =
        "Basic " + drogon::utils::base64Encode(get_client_id_from_env() + ":" +
                                               get_client_secret_from_env());
    req2->addHeader("Authorization", authHeader);

    client->sendRequest(
        req2, [callback](drogon::ReqResult result,
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
  void show_top_tracks(
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
                  bool doesnt_contain =
                      std::find(artists.begin(), artists.end(), artist_name) ==
                      artists.end();
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
};

} // namespace v1

int main(void) {
  drogon::app().addListener("0.0.0.0", 5555);
  drogon::app().createRedisClient("127.0.0.1", 6379);
  drogon::app().registerBeginningAdvice(v1::clear_user_colors);
  drogon::app().run();

  return 0;
}

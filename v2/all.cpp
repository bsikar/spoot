#include "all.hpp"
#include <fstream>

using drogon::HttpClient;
using drogon::HttpRequestPtr;
using drogon::HttpResponsePtr;

const char *SPOT_API_URI = "https://api.spotify.com";
const char *SPOT_USER_URI = "https://accounts.spotify.com";
const char *SPOT_REDIRECT_URI1 = "http://localhost:5555/get_creds";
const char *SPOT_REDIRECT_URI2 = "http://localhost:3000/play";

#define SPOT_USER_FULL_REDIRECT_URI(client_id, scopes, difficulty)             \
  std::string(SPOT_USER_URI) + "/authorize?response_type=code" +               \
      "&client_id=" + client_id + "&scope=" + scopes +                         \
      "&redirect_uri=" + SPOT_REDIRECT_URI1 + "/" + difficulty

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

namespace v2 {

std::optional<HttpRequestPtr> SpotifyHostController::makeRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> callback,
    const std::string &path) {
  LOG_TRACE << "Entering makeRequest";

  if (access_token.empty()) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k400BadRequest);
    std::string error_message = R"({"error": "Access token is required"})";
    LOG_ERROR << error_message;
    resp->setBody(error_message);
    callback(resp);
    return std::nullopt;
  }

  auto client = HttpClient::newHttpClient(SPOT_API_URI);
  auto spotify_req = drogon::HttpRequest::newHttpRequest();
  spotify_req->setMethod(drogon::Get);
  spotify_req->setPath(path);
  spotify_req->addHeader("Authorization", "Bearer " + access_token);
  return spotify_req;
}

void SpotifyHostController::topArtists(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback) {
  LOG_TRACE << "Entering topArtists";
  auto client = HttpClient::newHttpClient(SPOT_API_URI);
  auto copied_callback = callback; // Make a copy of the callback
  auto diff = req->getParameter("difficulty");
  if (!diff.empty() && diff != "undefined") {
    difficulty_level = diff;
  }

  std::string time_range = "long_term";
  int limit = 50;
  if (difficulty_level == "easy") {
    time_range = "short_term";
    limit = 10;
  } else if (difficulty_level == "medium") {
    time_range = "medium_term";
    limit = 30;
  }

  std::ostringstream oss;
  oss << "/v1/me/top/artists?"
      << "time_range=" << time_range << "&limit=" << limit << "&offset=0";
  LOG_INFO << oss.str();

  auto spotify_req = makeRequest(req, copied_callback, oss.str());
  if (!spotify_req) {
    // callback handled in makeRequest
    return;
  }
  client->sendRequest(
      *spotify_req, [copied_callback](drogon::ReqResult result,
                                      const HttpResponsePtr &response) mutable {
        auto resp = drogon::HttpResponse::newHttpResponse();

        if (result == drogon::ReqResult::Ok) {
          Json::CharReaderBuilder reader_builder;
          Json::Value json_data;
          std::string errs;

          std::istringstream response_stream(std::string(response->getBody()));

          if (Json::parseFromStream(reader_builder, response_stream, &json_data,
                                    &errs)) {
            Json::Value json_data_array;
            for (const auto &item : json_data["items"]) {
              Json::Value json_object;
              json_object["name"] = item["name"];
              json_object["popularity"] = item["popularity"];
              json_object["image"] = item["images"][0]["url"];
              json_data_array.append(json_object);
            }

            // Setting body with the contents in the form of json
            Json::StreamWriterBuilder writer_builder;
            LOG_INFO << Json::writeString(writer_builder, json_data_array);
            resp->setBody(Json::writeString(writer_builder, json_data_array));
          } else {
            std::string error_message =
                R"({"error": "Failed to parse Spotify response"})";
            LOG_ERROR << error_message;
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody(error_message);
          }
        } else {
          std::string error_message =
              R"({"error": "Failed to fetch top tracks"})";
          LOG_ERROR << error_message;
          resp->setStatusCode(drogon::k500InternalServerError);
          resp->setBody(error_message);
        }

        // XXX:
        // I DONT KNOW WHY, BUT THIS IS THE ONLY WAY ITS WORKING :<<<<
        // i tried to set it globally, but no dice :/
        resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
        copied_callback(resp);
      });
}

void SpotifyHostController::getSpotCreds(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
    const std::string &difficulty) {
  difficulty_level = difficulty;
  LOG_TRACE << "Entering getSpotCreds";
  LOG_INFO << difficulty_level;

  auto code = req->getParameter("code");
  if (code.empty()) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k400BadRequest);

    std::string error_message = R"({"error": "Code parameter is missing"})";
    LOG_ERROR << error_message;
    resp->setBody(error_message);
    callback(resp);
    return;
  }

  auto client = drogon::HttpClient::newHttpClient(SPOT_USER_URI);
  auto token_request = drogon::HttpRequest::newHttpRequest();
  token_request->setMethod(drogon::Post);
  token_request->setPath("/api/token");
  token_request->setContentTypeCode(drogon::CT_APPLICATION_X_FORM);

  std::ostringstream body_stream;
  body_stream << "grant_type=authorization_code";
  body_stream << "&code=" << code;
  body_stream << "&redirect_uri=" << SPOT_REDIRECT_URI1 << "/"
              << difficulty_level;
  token_request->setBody(body_stream.str());

  std::string auth_header =
      "Basic " + drogon::utils::base64Encode(getClientIdFromEnv() + ":" +
                                             getClientSecretFromEnv());
  token_request->addHeader("Authorization", auth_header);
  client->sendRequest(
      token_request, [this, callback](drogon::ReqResult result,
                                      const drogon::HttpResponsePtr &response) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        if (result == drogon::ReqResult::Ok) {
          Json::Reader reader;
          Json::Value json_data;

          if (reader.parse(std::string(response->getBody()), json_data)) {
            this->access_token = json_data["access_token"].asString();
            auto redirect_resp = drogon::HttpResponse::newRedirectionResponse(
                SPOT_REDIRECT_URI2);
            redirect_resp->addHeader("Location", SPOT_REDIRECT_URI2);
            callback(redirect_resp);
            return;
          } else {
            std::string error_message =
                R"({"error": "Failed to parse Spotify response"})";
            LOG_ERROR << error_message;
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody(error_message);
          }
        } else {
          resp->setStatusCode(drogon::k500InternalServerError);
          std::string error_message =
              R"({"error": "Failed to get access token from Spotify"})";
          LOG_ERROR << error_message;
          resp->setBody(error_message);
        }

        callback(resp);
      });
}

void SpotifyAuthController::redirectToSpotifyAuth(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  LOG_TRACE << "Entering redirectToSpotifyAuth";
  std::string client_id = getClientIdFromEnv();
  if (client_id.empty()) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    std::string error_message =
        R"({"error": "Failed to parse .env file, no CLIENT_ID= found"})";
    LOG_ERROR << error_message;
    resp->setBody(error_message);
    callback(resp);
    return;
  }

  auto diff = req->getParameter("difficulty");
  LOG_INFO << diff;
  std::string scopes = "user-top-read";
  std::string spotify_auth_url =
      SPOT_USER_FULL_REDIRECT_URI(client_id, scopes, diff);
  LOG_INFO << spotify_auth_url;
  auto resp = drogon::HttpResponse::newRedirectionResponse(spotify_auth_url);
  callback(resp);
}

void CorsFilter::doFilter(const drogon::HttpRequestPtr &req,
                          drogon::FilterCallback &&fcb,
                          drogon::FilterChainCallback &&fccb) {
  auto resp = drogon::HttpResponse::newHttpResponse();

  // Add necessary headers
  resp->addHeader("Access-Control-Allow-Origin", "http://localhost:3000");
  resp->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  resp->addHeader("Access-Control-Allow-Headers", "Content-Type");

  // Handle preflight request
  if (req->getMethod() == drogon::Options) {
    fcb(resp);
    return;
  }

  fccb();
}

} // namespace v2

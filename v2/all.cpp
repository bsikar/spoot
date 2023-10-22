#include "all.hpp"

using drogon::HttpClient;
using drogon::HttpRequestPtr;
using drogon::HttpResponsePtr;

const char *SPOT_API_URI = "https://api.spotify.com";
const char *SPOT_USER_URI = "https://accounts.spotify.com";
const char *SPOT_REDIRECT_URI = "http://localhost:3000/play";

#define SPOT_USER_FULL_REDIRECT_URI(client_id, scopes)                         \
  std::string(SPOT_USER_URI) + "/authorize?response_type=code" +               \
      "&client_id=" + client_id + "&scope=" + scopes +                         \
      "&redirect_uri=" + SPOT_REDIRECT_URI

namespace v2 {

std::optional<HttpRequestPtr> SpotifyHostController::makeRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> callback,
    const std::string &path) {
  const auto access_token = req->getParameter("access_token");

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
  auto client = HttpClient::newHttpClient(SPOT_API_URI);
  auto copied_callback = callback; // Make a copy of the callback

  auto spotify_req = makeRequest(req, copied_callback,
                                 "/v1/me/top/artists?"
                                 "time_range=medium_term"
                                 "&limit=15" // hard coded max
                                 "&offset=0");
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

        copied_callback(resp);
      });
}

void SpotifyHostController::getSpotCreds(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {

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
  body_stream << "&redirect_uri=" << SPOT_REDIRECT_URI;
  body_stream << "&code=" << code;

  token_request->setBody(body_stream.str());

  std::string auth_header =
      "Basic " + drogon::utils::base64Encode(getClientIdFromEnv() + ":" +
                                             getClientSecretFromEnv());
  token_request->addHeader("Authorization", auth_header);

  client->sendRequest(
      token_request, [callback](drogon::ReqResult result,
                                const drogon::HttpResponsePtr &response) {
        if (result == drogon::ReqResult::Ok) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setBody(std::string(response->getBody()));
          callback(resp);
        } else {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(drogon::k500InternalServerError);
          std::string error_message =
              R"({"error": "Failed to get access token from Spotify"})";
          LOG_ERROR << error_message;
          resp->setBody(error_message);
          callback(resp);
        }
      });
}

void SpotifyAuthController::redirectToSpotifyAuth(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
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

  std::string scopes = "user-top-read";
  std::string spotify_auth_url = SPOT_USER_FULL_REDIRECT_URI(client_id, scopes);
  auto resp = drogon::HttpResponse::newRedirectionResponse(spotify_auth_url);
  callback(resp);
}

} // namespace v2


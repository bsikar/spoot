#include "all.hpp"

using drogon::HttpClient;
using drogon::HttpRequestPtr;
using drogon::HttpResponsePtr;

const char *SPOT_API_URI = "https://api.spotify.com";

namespace v2 {

std::optional<HttpRequestPtr> SpotifyHostController::makeRequest(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> callback,
    const std::string &path) {
  const auto access_token = req->getParameter("access_token");

  if (access_token.empty()) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k400BadRequest);
    LOG_ERROR << R"({"error": "Access token is required"})";
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
  LOG_INFO << "Entering topArtist in V2";
  auto client = HttpClient::newHttpClient(SPOT_API_URI);
  auto copied_callback = callback; // Make a copy of the callback

  auto spotify_req = makeRequest(
      req, copied_callback,
      "/v1/me/top/artists?time_range=medium_term&limit=15&offset=0");

  if (!spotify_req) {
    // callback handled in makeRequest
    return;
  }
  LOG_INFO << "Still safe";
  client->sendRequest(
      *spotify_req, [copied_callback](drogon::ReqResult result,
                                      const HttpResponsePtr &response) mutable {
        auto resp = drogon::HttpResponse::newHttpResponse();
        LOG_INFO << "I entered";
        if (result == drogon::ReqResult::Ok) {
          Json::Reader reader;
          Json::Value json_data;
          LOG_INFO << "Parse time";
          if (reader.parse(std::string(response->getBody()), json_data)) {
            LOG_INFO << "Parse if statment";
            std::vector<std::tuple<std::string, std::string, std::string>> data;
            LOG_FATAL << json_data.toStyledString();
            for (const auto &item : json_data["items"]) {
              auto image = item["images"][0]["url"].asString();
              auto name = item["name"].asString();
              auto pop = item["popularity"].asString();
              std::cout << image << name << pop << std::endl;
              data.push_back({name, pop, image});
            }

            std::string body;
            for (const auto &[i, n, p] : data) {
              body += "<p>" + i + " " + n + " " + p + "</p>";
            }
            resp->setBody(body);
          } else {
            LOG_ERROR << R"({"error": "Failed to parse Spotify response"})";
          }
        } else {
          resp->setStatusCode(drogon::k500InternalServerError);
          LOG_ERROR << R"({"error": "Failed to fetch top tracks"})";
        }
        LOG_INFO << "Im about to leave";
        copied_callback(resp);
        LOG_INFO << "Bye";
      });
  LOG_INFO << "Exiting function";
}

} // namespace v2

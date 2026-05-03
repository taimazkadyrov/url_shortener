#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "server.h"

using PathParams = std::map<std::string, std::string>;
using RouteHandler = std::function<HttpResponse(const HttpRequest&, const PathParams&)>;
struct Route {
    std::string method;
    std::string pattern;
    std::vector<std::string> parts;
    RouteHandler handler;
};

class Router {
public:
    Router() = default;
    void get(const std::string& pattern, RouteHandler handler);
    void post(const std::string& pattern, RouteHandler handler);
    HttpResponse handle(const HttpRequest& request);
private:
    std::vector<Route> routes_;
    void add_route(const std::string& method,
                  const std::string& pattern,
                  RouteHandler handler);

    std::vector<std::string> split_path(const std::string& path);
    bool match_route(const Route& route,
                    const std::string& path,
                    PathParams& params);
};
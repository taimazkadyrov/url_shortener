#include "router.h"
#include <sstream>
void Router::get(const std::string& pattern, RouteHandler handler) {
    add_route("GET", pattern, handler);
}

void Router::post(const std::string& pattern, RouteHandler handler) {
    add_route("POST", pattern, handler);
}

HttpResponse Router::handle(const HttpRequest& request) {
    PathParams params;
    for (const auto& route : routes_) {
        if (route.method != request.method) {
            continue;
        }
        if (match_route(route, request.path, params)) {
            return route.handler(request, params);
        }
    }
    return HttpResponse::notFound("маршрут не найден: " + request.path);
}

void Router::add_route(const std::string& method,
                      const std::string& pattern,
                      RouteHandler handler)
{
    Route route;
    route.method = method;
    route.pattern = pattern;
    route.parts = split_path(pattern);
    route.handler = handler;
    routes_.push_back(route);
}

std::vector<std::string> Router::split_path(const std::string& path) {
    std::vector<std::string> parts;
    std::istringstream stream(path);
    std::string part;
    while (std::getline(stream, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    return parts;
}

bool Router::match_route(const Route& route,
                        const std::string& path,
                        PathParams& params)
{
    std::vector<std::string> path_parts = split_path(path);

    if (route.parts.size() != path_parts.size()) {
        if (route.pattern == "/") {
            return true;
        }
        return false;
    }
    params.clear();

    for (size_t i = 0; i < route.parts.size(); i++) {
        const std::string& route_part = route.parts[i];
        const std::string& path_part = path_parts[i];
        if (route_part[0] == ':') {
            std::string param_name = route_part.substr(1); // убираем ":"
            params[param_name] = path_part;
        } else if (route_part != path_part) {
            params.clear();
            return false;
        }
    }

    return true;
}
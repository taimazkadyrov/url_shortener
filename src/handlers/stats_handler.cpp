#include "stats_handler.h"
#include <iostream>

StatsHandler::StatsHandler(std::shared_ptr<RedisClient> redis)
    : redis_(redis)
{
}

HttpResponse StatsHandler::handle(const HttpRequest& request,
                                  const PathParams& params)
{
    try {
        auto it = params.find("code");
        if (it == params.end()) {
            return HttpResponse::bad_request("код не указан");
        }

        std::string code = it->second;
        auto url = redis_->get("url:" + code);
        if (!url.has_value()) {
            return HttpResponse::not_found("ссылка не найдена: " + code);
        }

        long long clicks = redis_->get_counter("clicks:" + code);
        std::string body = build_json(code, url.value(), clicks);
        return HttpResponse::ok(body);

    } catch (const std::exception& e) {
        std::cerr << "StatsHandler ошибка: " << e.what() << std::endl;
        return HttpResponse::internal_error();
    }
}

std::string StatsHandler::build_json(const std::string& code,
                                    const std::string& url,
                                    long long clicks)
{
    return "{\n"
           "  \"code\": \""         + code              + "\",\n"
           "  \"original_url\": \"" + url               + "\",\n"
           "  \"clicks\": "         + std::to_string(clicks) + "\n"
           "}";
}
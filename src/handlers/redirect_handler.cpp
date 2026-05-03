#include "redirect_handler.h"
#include <iostream>

RedirectHandler::RedirectHandler(std::shared_ptr<RedisClient> redis)
    : redis_(redis)
{
}

HttpResponse RedirectHandler::handle(const HttpRequest& request,
                                     const PathParams& params)
{
    try {
        auto it = params.find("code");
        if (it == params.end()) {
            return HttpResponse::badRequest("код не указан");
        }
        std::string code = it->second;
        auto url = redis_->get("url:" + code);
        if (!url.has_value()) {
            return HttpResponse::notFound("cсылка не найдена: " + code);
        }
        redis_->incr("clicks:" + code);
        return HttpResponse::redirect(url.value());

    } catch (const std::exception& e) {
        std::cerr << "RedirectHandler ошибка: " << e.what() << std::endl;
        return HttpResponse::internalError();
    }
}
#include "shorten_handler.h"
#include <stdexcept>
#include <iostream>

ShortenHandler::ShortenHandler(std::shared_ptr<RedisClient> redis)
    : redis_(redis)
{
}
HttpResponse ShortenHandler::handle(const HttpRequest& request,
                                    const PathParams& params)
{
    try {
        std::string url = extract_url(request.body);
        if (url.empty()) {
            return HttpResponse::badRequest("поле url обязательно");
        }
        if (!is_valid_url(url)) {
            return HttpResponse::badRequest("некорректный url: " + url);
        }
        std::string code = generate_unique_code();
        redis_->set("url:" + code, url);
        redis_->set("clicks:" + code, "0");
        std::string short_url = "http://localhost:8080/" + code;
        std::string response_body = "{\"short_url\": \"" + short_url + "\"}";

        return HttpResponse::ok(response_body);
    } catch (const std::exception& e) {
        std::cerr << "ShortenHandler ошибка: " << e.what() << std::endl;
        return HttpResponse::internalError();
    }
}

std::string ShortenHandler::extract_url(const std::string& body) {
    std::string key = "\"url\"";
    auto key_pos = body.find(key);
    if (key_pos == std::string::npos) {
        return "";
    }
    auto colon_pos = body.find(':', key_pos);
    if (colon_pos == std::string::npos) {
        return "";
    }
    auto quote_start = body.find('"', colon_pos);
    if (quote_start == std::string::npos) {
        return "";
    }
    auto quote_end = body.find('"', quote_start + 1);
    if (quote_end == std::string::npos) {
        return "";
    }
    return body.substr(quote_start + 1, quote_end - quote_start - 1);
}

bool ShortenHandler::is_valid_url(const std::string& url) {
    if (url.substr(0, 7) == "http://" ||
        url.substr(0, 8) == "https://") {
        return true;
    }
    return false;
}

std::string ShortenHandler::generate_unique_code() {
    std::string code;
    int attempts = 0;
    const int max_attempts = 10;
    do {
        if (attempts++ >= max_attempts) {
            throw std::runtime_error("не удалось сгенерировать уникальный код");
        }
        code = CodeGenerator::generate(6);
    } while (redis_->exists("url:" + code));

    return code;
}
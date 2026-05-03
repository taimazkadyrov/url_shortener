#pragma once
#include <string>
#include <memory>
#include "../server/server.h"
#include "../server/router.h"
#include "../storage/redis_client.h"
#include "../generators/code_generator.h"

class ShortenHandler {
public:
    explicit ShortenHandler(std::shared_ptr<RedisClient> redis);
    HttpResponse handle(const HttpRequest& request, const PathParams& params);

private:
    std::shared_ptr<RedisClient> redis_;
    std::string extract_url(const std::string& body);
    bool is_valid_url(const std::string& url);
    std::string generate_unique_code();
};
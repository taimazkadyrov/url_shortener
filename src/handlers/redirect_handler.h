#pragma once
#include <string>
#include <memory>
#include "../server/server.h"
#include "../server/router.h"
#include "../storage/redis_client.h"

class RedirectHandler {
public:
    explicit RedirectHandler(std::shared_ptr<RedisClient> redis);
    HttpResponse handle(const HttpRequest& request, const PathParams& params);

private:
    std::shared_ptr<RedisClient> redis_;
};
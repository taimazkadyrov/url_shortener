#pragma once
#include <string>
#include <memory>
#include "../server/server.h"
#include "../server/router.h"
#include "../storage/redis_client.h"

class StatsHandler {
public:
    explicit StatsHandler(std::shared_ptr<RedisClient> redis);
    HttpResponse handle(const HttpRequest& request, const PathParams& params);

private:
    std::shared_ptr<RedisClient> redis_;
    std::string build_json(const std::string& code,
                          const std::string& url,
                          long long clicks);
};
#include <iostream>
#include <memory>

#include "server/server.h"
#include "server/router.h"
#include "storage/redis_client.h"
#include "handlers/shorten_handler.h"
#include "handlers/redirect_handler.h"
#include "handlers/stats_handler.h"

int main() {
    try {
        std::cout << "подключение к Redis..." << std::endl;
        auto redis = std::make_shared<RedisClient>("127.0.0.1", 6379);
        std::cout << "redis подключён!" << std::endl;

        auto shorten_handler  = std::make_shared<ShortenHandler>(redis);
        auto redirect_handler = std::make_shared<RedirectHandler>(redis);
        auto stats_handler    = std::make_shared<StatsHandler>(redis);

        Router router;
        router.post("/shorten", [&](const HttpRequest& req, const PathParams& params) {
            return shorten_handler->handle(req, params);
        });
        router.get("/stats/:code", [&](const HttpRequest& req, const PathParams& params) {
            return stats_handler->handle(req, params);
        });
        router.get("/:code", [&](const HttpRequest& req, const PathParams& params) {
            return redirect_handler->handle(req, params);
        });

        Server server("127.0.0.1", 8080);
        server.get("/", [&](const HttpRequest& req) {
            return router.handle(req);
        });
        server.post("/", [&](const HttpRequest& req) {
            return router.handle(req);
        });

        std::cout << "сервер запущен на http://127.0.0.1:8080" << std::endl;
        std::cout << "для остановки нажмите Ctrl+C" << std::endl;
        server.run();

    } catch (const std::exception& e) {
        std::cerr << "ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#pragma once
#include <string>
#include <optional>
#include <stdexcept>
#include <hiredis/hiredis.h>

class RedisClient {
public:
    RedisClient(const std::string& host, int port);
    ~RedisClient();
    RedisClient(const RedisClient&) = delete;
    RedisClient& operator=(const RedisClient&) = delete;

    void set(const std::string& key, const std::string& value);
    void setex(const std::string& key, const std::string& value, int ttl_seconds);
    std::optional<std::string> get(const std::string& key);
    bool exists(const std::string& key);
    void del(const std::string& key);
    long long incr(const std::string& key);
    long long get_counter(const std::string& key);

private:
    redisContext* context_;
    redisReply* execute_command(const char* format, ...);
    void free_reply(redisReply* reply);
};

#include "redis_client.h"
#include <stdexcept>
#include <cstdarg>

RedisClient::RedisClient(const std::string& host, int port) {
    context_ = redisConnect(host.c_str(), port);

    if (context_ == nullptr) {
        throw std::runtime_error("не удалось выделить память для Redis контекста");
    }

    if (context_->err) {
        std::string error = "ошибка подключения к Redis: " + std::string(context_->errstr);
        redisFree(context_);
        throw std::runtime_error(error);
    }
}

RedisClient::~RedisClient() {
    if (context_) {
        redisFree(context_);
    }
}

// --- Приватные методы ---

redisReply* RedisClient::execute_command(const char* format, ...) {
    va_list args;
    va_start(args, format);
    redisReply* reply = (redisReply*)redisvCommand(context_, format, args);
    va_end(args);

    if (reply == nullptr) {
        throw std::runtime_error("redis команда не выполнена: нет соединения");
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        std::string error = "redis вернул ошибку: " + std::string(reply->str);
        freeReplyObject(reply);
        throw std::runtime_error(error);
    }

    return reply;
}

void RedisClient::free_reply(redisReply* reply) {
    if (reply) {
        freeReplyObject(reply);
    }
}

// --- Публичные методы ---

void RedisClient::set(const std::string& key, const std::string& value) {
    redisReply* reply = execute_command("SET %s %s", key.c_str(), value.c_str());
    free_reply(reply);
}

void RedisClient::setex(const std::string& key, const std::string& value, int ttl_seconds) {
    redisReply* reply = execute_command(
        "SETEX %s %d %s",
        key.c_str(),
        ttl_seconds,
        value.c_str()
    );
    free_reply(reply);
}

std::optional<std::string> RedisClient::get(const std::string& key) {
    redisReply* reply = execute_command("GET %s", key.c_str());

    if (reply->type == REDIS_REPLY_NIL) {
        free_reply(reply);
        return std::nullopt;
    }
    std::string value(reply->str, reply->len);
    free_reply(reply);
    return value;
}

bool RedisClient::exists(const std::string& key) {
    redisReply* reply = execute_command("EXISTS %s", key.c_str());
    bool result = (reply->integer == 1);
    free_reply(reply);
    return result;
}

void RedisClient::del(const std::string& key) {
    redisReply* reply = execute_command("DEL %s", key.c_str());
    free_reply(reply);
}

long long RedisClient::incr(const std::string& key) {
    redisReply* reply = execute_command("INCR %s", key.c_str());
    long long value = reply->integer;
    free_reply(reply);
    return value;
}

long long RedisClient::get_counter(const std::string& key) {
    auto value = get(key);
    if (!value.has_value()) {
        return 0;
    }
    return std::stoll(value.value());
}
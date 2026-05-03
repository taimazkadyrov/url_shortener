#include "server.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

HttpResponse HttpResponse::ok(const std::string& body) {
    HttpResponse res;
    res.status_code = 200;
    res.body = body;
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse HttpResponse::redirect(const std::string& url) {
    HttpResponse res;
    res.status_code = 302;
    res.headers["Location"] = url;
    return res;
}

HttpResponse HttpResponse::not_found(const std::string& message) {
    HttpResponse res;
    res.status_code = 404;
    res.body = "{\"error\": \"" + message + "\"}";
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse HttpResponse::bad_request(const std::string& message) {
    HttpResponse res;
    res.status_code = 400;
    res.body = "{\"error\": \"" + message + "\"}";
    res.headers["Content-Type"] = "application/json";
    return res;
}

HttpResponse HttpResponse::internal_error(const std::string& message) {
    HttpResponse res;
    res.status_code = 500;
    res.body = "{\"error\": \"" + message + "\"}";
    res.headers["Content-Type"] = "application/json";
    return res;
}

Server::Server(const std::string& host, int port)
    : host_(host)
    , port_(port)
    , server_fd_(-1)
    , is_running_(false)
{
}

Server::~Server() {
    stop();
}

void Server::get(const std::string& path, Handler handler) {
    routes_["GET " + path] = handler;
}

void Server::post(const std::string& path, Handler handler) {
    routes_["POST " + path] = handler;
}

void Server::run() {
    init_socket();
    is_running_ = true;

    std::cout << "сервер запущен на " << host_ << ":" << port_ << std::endl;

    while (is_running_) {
        accept_connection();
    }
}

void Server::stop() {
    is_running_ = false;
    if (server_fd_ != -1) {
        close(server_fd_);
        server_fd_ = -1;
    }
}

void Server::init_socket() {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        throw std::runtime_error("не удалось создать сокет");
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    inet_pton(AF_INET, host_.c_str(), &address.sin_addr);

    if (bind(server_fd_, (sockaddr*)&address, sizeof(address)) < 0) {
        throw std::runtime_error("не удалось привязать сокет к адресу");
    }

    if (listen(server_fd_, 10) < 0) {
        throw std::runtime_error("не удалось начать прослушивание");
    }
}

void Server::accept_connection() {
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd_, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        if (is_running_) {
            std::cerr << "ошибка принятия соединения" << std::endl;
        }
        return;
    }
    try {
        HttpRequest request = parse_request(client_fd);
        HttpResponse response = dispatch(request);
        send_response(client_fd, response);
    } catch (const std::exception& e) {
        std::cerr << "ошибка обработки запроса: " << e.what() << std::endl;
        send_response(client_fd, HttpResponse::internal_error());
    }
    close(client_fd);
}

HttpRequest Server::parse_request(int client_fd) {
    char buffer[4096] = {};
    read(client_fd, buffer, sizeof(buffer) - 1);
    std::istringstream stream(buffer);
    HttpRequest request;
    std::string http_version;
    stream >> request.method >> request.path >> http_version;
    std::string line;
    std::getline(stream, line);
    while (std::getline(stream, line) && line != "\r") {
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2);
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            request.headers[key] = value;
        }
    }

    if (request.method == "POST") {
        std::ostringstream body_stream;
        body_stream << stream.rdbuf();
        request.body = body_stream.str();
        auto start = request.body.find_first_not_of("\r\n");
        if (start != std::string::npos) {
            request.body = request.body.substr(start);
        }
    }

    return request;
}

void Server::send_response(int client_fd, const HttpResponse& response) {
    std::map<int, std::string> status_texts = {
        {200, "OK"},
        {302, "Found"},
        {400, "Bad Request"},
        {404, "Not Found"},
        {500, "Internal Server Error"}
    };

    std::string status_text = status_texts.count(response.status_code)
        ? status_texts[response.status_code]
        : "Unknown";
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.status_code << " " << status_text << "\r\n";
    for (const auto& [key, value] : response.headers) {
        oss << key << ": " << value << "\r\n";
    }
    oss << "Content-Length: " << response.body.size() << "\r\n";
    oss << "\r\n";
    oss << response.body;
    std::string raw = oss.str();
    write(client_fd, raw.c_str(), raw.size());
}

HttpResponse Server::dispatch(const HttpRequest& request) {
    std::string key = request.method + " " + request.path;
    if (routes_.count(key)) {
        return routes_[key](request);
    }
    for (const auto& [route, handler] : routes_) {
        if (route == request.method + " /") {
            return handler(request);
        }
    }

    return HttpResponse::not_found("маршрут не найден: " + request.path);
}
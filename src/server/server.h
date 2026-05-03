#pragma once
#include <string>
#include <memory>
#include <functional>
#include <map>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
};

struct HttpResponse {
    int status_code;
    std::string body;
    std::map<std::string, std::string> headers;

    static HttpResponse ok(const std::string& body);
    static HttpResponse redirect(const std::string& url);
    static HttpResponse not_found(const std::string& message = "Not found");
    static HttpResponse bad_request(const std::string& message);
    static HttpResponse internal_error(const std::string& message = "Internal server error");
};
using Handler = std::function<HttpResponse(const HttpRequest&)>;

class Server {
public:
    Server(const std::string& host, int port);
    ~Server();
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    void run();
    void stop();

private:
    std::string host_;
    int port_;
    int server_fd_;
    bool is_running_;

    std::map<std::string, Handler> routes_;
    void accept_connection();
    HttpRequest parse_request(int client_fd);
    void send_response(int client_fd, const HttpResponse& response);
    HttpResponse dispatch(const HttpRequest& request);
    void init_socket();
};
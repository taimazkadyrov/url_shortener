# URL Shortener

Сервис сокращения ссылок, написанный на C++ без использования фреймворков.

## Стек
- C++17
- Redis (hiredis)
- HTTP (сырые сокеты)

## API
- `POST /shorten` — создать короткую ссылку
- `GET /:code` — редирект на оригинальный URL
- `GET /stats/:code` — статистика переходов

## Сборка (Linux)
```bash
sudo apt install libhiredis-dev
cmake -B build
cmake --build build
```

## Запуск
```bash
redis-server &
./build/url_shortener
```
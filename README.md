### Torrent Client Prototype

Этот проект представляет собой реализацию BitTorrent-клиента на C++ с поддержкой основных функций протокола.

## Функциональность

### Парсинг .torrent файлов
- Загрузка метаданных
- Вычисление infoHash
- Поддержка формата bencode

### Взаимодействие с трекером
- HTTP-запросы с использованием библиотеки cpr
- Обработка ответов в формате bencode

### Сетевое взаимодействие
- TCP-соединение с пирами
- Реализация handshake-протокола
- Обмен сообщениями (bitfield, interested, unchoke и др.)

### Скачивание файлов
- Загрузка частей файла (pieces)
- Проверка целостности данных
- Многопоточная обработка
- Сохранение на диск

## Функциональность

- CMake ≥ 3.10
- libcurl
- OpenSSL
- Компилятор с поддержкой C++17

## Основные компоненты

- bencode - парсер формата bencode
- torrent_file - работа с .torrent файлами
- torrent_tracker - взаимодействие с трекером
- peer_connect - соединение с пирами
- piece_storage - управление частями файла
- piece - часть файла
- tcp_connect - TCP-соединения

## Установка зависимостей

- Как установить зависимости на Ubuntu 20.04:
```
$ sudo apt-get install cmake libcurl4-openssl-dev libssl-dev
```
- Как установить зависимости на Mac OS через пакетный менеджер `brew` (https://brew.sh/):
```
$ brew install openssl cmake
```

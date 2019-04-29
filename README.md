# Tan90-Proxy
NAT acorss proxy server - NAT穿透代理服务器

## Dependency  
* glib2
* libuv

## Build  

|Input              | Description                                                                           |
| :-                | :-                                                                                    |
|**`make`**         |   build *libtan90common.a* , *tan90-server* and *tan90-client* by **release** mode    |
|**`make debug`**   |   build *libtan90common.a* , *tan90-server* and *tan90-client* by **debug** mode      |
|**`make clean`**   |   remove all *.o* and *libtan90common.a*                                              |
|**`make commom`**  |   build *libtan90common.a* by **release** mode                                        |
|**`make server`**  |   build *tan90-server* by **release** mode                                            |
|**`make client`**  |   build *tan90-client* by **release** mode                                            |

> On Windows , suggest using `Msys2` to build.

## Usage
Edit `server.ini` for `tan90-server`
```ini
# server.ini
[Config Name]
true_client_ip = 0.0.0.0
true_client_port = 80
proxy_client_ip = 0.0.0.0
proxy_client_port = 8102
```
* `Config Name` : unimportant value but cannot repeat in a file
* `true_client_ip` : bind to this IP for listening to true client
  * `0.0.0.0` : all owned IP , suggest using this value
  * `127.0.0.1` : can only be connected by local application
* `true_client_port` : bind to this port for listening to true client
* `proxy_client_ip` : bind to this IP for listening to proxy client
  * `0.0.0.0` : all owned IP , suggest using this value
  * `127.0.0.1` : can only be connected by local application
* `proxy_client_port` : bind to this port for listening to proxy client

Edit `client.ini` for `tan90-client`
```ini
# client.ini
[Config Name]
proxy_server_ip = 233.233.233.233
proxy_server_port = 8102
true_server_ip = 0.0.0.0
true_server_port = 80
```
* `Config Name` : unimportant value but cannot repeat in a file
* `proxy_server_ip` : IP of proxy server
* `proxy_server_port` : port of proxy server
* `true_server_ip` : IP of true server
* `true_server_port` : port of treu server

For Example : [Proxy Minecraft Server](https://github.com/hubenchang0515/Tan90-Proxy/issues/2)  

## Demo
![Image](./image/pic01.png)

```ini
# server.ini
[Proxy of Nginx]
true_client_ip = 0.0.0.0
true_client_port = 80
proxy_client_ip = 0.0.0.0
proxy_client_port = 8102

[Proxy of Minecraft]
true_client_ip = 0.0.0.0
true_client_port = 25565
proxy_client_ip = 0.0.0.0
proxy_client_port = 8103
```

```ini
# client.ini
[Proxy of Nginx]
proxy_server_ip = 233.233.233.233
proxy_server_port = 8102
true_server_ip = 0.0.0.0
true_server_port = 80

[Proxy of Minecraft]
proxy_server_ip = 233.233.233.233
proxy_server_port = 8103
true_server_ip = 0.0.0.0
true_server_port = 25565
```


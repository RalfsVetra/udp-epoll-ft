# Simple UDP file transfer using epoll

A simple implementation of file transfer using UDP and epoll. This project is for learning purposes only and is
not production ready. Currently only tested on Debian 13.

## Build
```bash
make -j
```

## Usage
```bash
./build/udp_epoll_ft server 5000
```

Run in server mode and listen on port 5000.

```bash
./build/udp_epoll_ft client 5000 "../file_to_upload.pdf"
```

Run in client mode and listen on the same port used by the server 5000. The final argument should be the path to the file you want to upload to server.

## License
This project is licensed under the MIT License - see the [LICENSE](/LICENSE) file for details.
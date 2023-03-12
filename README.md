# Configurable args
## Priveleged Ports
If you want to bind your server to priveleged ports (< 1024), then you should run the `httpserver` executable with root priveleges by, for instance, using `sudo` keyword (if you're on Ubuntu, Debian, etc.), as follows:
```
sudo ./run_server --server_directory <dir-name> --port <port-num>
```


# Status Codes Breakdown
## 200: OK
- File getting processed successfully.
- No content length header issue listed below, possibly due to the combination of read and write IO function calls.

## 400: Bad Request
- Not properly [formatted](https://kinsta.com/knowledgebase/what-is-an-http-request/) HTTP request, for example, `GET /XXX HTTP/1.1`.
- Not a file.
    - Content length header is not sent, as suggested by [this stackoverflow post](https://stackoverflow.com/questions/1759956/curl-error-18-transfer-closed-with-outstanding-read-data-remaining), in case of writing an error message to the client fd and closing the connection.
- Not an html fille (no conent length header for the same reason as above).
    - NOTE: This takes precedence for non-html files than the 404 (Not Found) status code. Even if you request a non-existent txt file (for example), you'll get a 400 status code, not 404.

## 403: Forbidden
- If the user tries to access outside server-hosting directory, e.g.
```
curl -v --path-as-is http://127.0.0.1:80/../server_dir/cute_capybara.html
```

## 404: Not Found
- An html file that does not exist.

## 500: File Server
- Error reading the html file (either caused by error in reading or operning file for reading).



# TODO

## cloud client

- fk yeah, `Windows` only
- run as system service
- monitoring
	- monitor a local directory (path can be config-ed)
	- back up to `cloud server`
-  separate command-line client
	- check old files
	- restore old files

1. select directory
1. when connected, sync files. (client update server)
1. periodical sync
1. configurable sync interval (config file)
1. restore: enter file path, select timestamp

## cloud server

- Linux only
- maintains backup files
- Also, backup past version

### meta data

- file path
- time diff (between server and client, timezone)


### Note

- server: lock meta data (single access)
- client: file lock @@
- file ID = (path, timestamp)
- On top of HTTP...


http://www.gnu.org/software/cgicc/doc/test_8cpp.html
http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
http://stackoverflow.com/questions/14551194/how-are-parameters-sent-in-an-http-post-request
http://stackoverflow.com/questions/16958448/what-is-http-multipart-request

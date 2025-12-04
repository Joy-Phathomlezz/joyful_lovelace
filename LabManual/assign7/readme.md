start server on h1

# Build and run
- Compile the UDP calc server (math library required for sin):

```sh
gcc udp_srv.c -o udp_srv -lm
./udp_srv 12345
```

- Compile the client:

```sh
gcc clnt.c -o clnt
./clnt 10.0.0.1 12345 1.0 0.5
```

Notes:
- The server computes currently only sin(x) - sin(y).

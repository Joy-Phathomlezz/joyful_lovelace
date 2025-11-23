start server on h1 
gcc server.c -o server
./server 12345

steart client on h2 
gcc clnt.c -o clnt
./clnt 10.0.0.1 12345 1.0 0.5


currently only does sin(x) - sin(y)
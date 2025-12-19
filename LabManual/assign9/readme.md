# CN Lab - Assignment 9

## Objective
File upload and download using TCP.

## Exercise
Write a client server socket program in TCP for uploading and downloading files between two different hosts. Also calculate the transfer time in both the cases.

![Screenshot](assign9.png)

## Implementation

### Server ([file_server.c])
- TCP server that handles file upload and download requests
- Calculates and displays transfer time for each operation
- Supports both sending and receiving files

### Client ([file_client.c])
- TCP client that can upload or download files
- Calculates and displays transfer time
- Uses Mininet IP (10.0.0.1)

## Compilation
```bash
make
```

## Usage
1. Start server:
   ```bash
   ./file_server
   ```

2. Download a file from server:
   ```bash
   ./file_client download server_file.txt
   ```

3. Upload a file to server:
   ```bash
   ./file_client upload client_file.txt
   ```

## Steps to Test in Mininet
1. Create two directories for server and client
2. Run xterm on mininet hosts
3. Place a file in the server directory
4. Place a file in the client directory
5. First, client downloads the server file
6. Second, client uploads its file to the server
7. Transfer times are displayed on both sides

## Learning Outcomes
1. Use of TCP for file transfer.

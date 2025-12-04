# Setup 


Two Mininet hosts

    Sender: h1 (10.0.0.1)

    Receiver: h2 (10.0.0.2)

Two C files 

 send_raw_tcp.c  

 recv_raw_tcp.c
 
"All commands below run in mininet host terminals (xterm)"

## start your mininet topology



```bash
xterm h1 h2
```

## compile them and run as usual c files

```python
gcc send_raw_tcp.c -o send_raw_tcp
gcc recv_raw_tcp.c -o recv_raw_tcp
```

## Note


```bash
!!! on h1:
sudo ./<your_raw_tcp_sender> 10.0.0.1 10.0.0.2 4000 5000 
```

```bash
sudo ./<your_raw_tcp_receiver> 
```



###### © Joy-Phathomlezz/joyful_lovelace 
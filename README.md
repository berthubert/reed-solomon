Compiling
---------
Make sure you have `libfec-dev` or equivalent installed. If your OS does not
have this as package, check out out [KA9Q's
site](http://www.ka9q.net/code/fec/)

Also make sure you have `cmake`.

Then:

```
mkdir build; cd build 
cmake ..
make
```

Then try: 

```
./rscmd --help
```

This command will allow you to encode and decode messages using all
Reed-Solomon configurations, as long as the symbol size is 8 bits. 

A sample run:

```
$ ./rscmd "bert hubert"
nroots: 32
prim: 11
fcr: 121
poly: 1 + x + x^2 + x^7 + x^8
polyval: 0x187
(N,K) = (255,223)
The 32 parity bytes for "bert hubert": ddaa8fd46499675b712186a0da1dd8738bee3df7f25b49c2c0e84e4a8fe8137b
```

This runs the command with the most bog standard Reed-Solomon settings, and
creates 32 parity bytes for the string "bert hubert". 

To test if it worked, let us corrupt this string a bit (or arguably fix it):

```
$ ./rscmd "nerd hubert" ddaa8fd46499675b712186a0da1dd8738bee3df7f25b49c2c0e84e4a8fe8137b
nroots: 32
prim: 11
fcr: 121
poly: 1 + x + x^2 + x^7 + x^8
polyval: 0x187
(N,K) = (255,223)
Fixed 2 corruptions in positions: 0 3
Recovered: bert hubert
```

Exercising
----------
If you are unsure about your chosen parameters, try `--exercise somestring`.
This will do 10000 encode/decode cycles, where each time the message is
mangled as much as possible while staying within what should be recoverable.

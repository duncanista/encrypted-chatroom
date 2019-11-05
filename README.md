# Encrypted Multi-client Chatroom in C

Chatroom encrypted with a simple XOR.

## Run

```console
gcc server.c -o server.out
```
```console
./server.out
```

## Cipher

I strongly recommend not using this XOR cipher, since it's pretty simple. Although, a way to ensure a bit of security would be modifying the code so it asks one time for the key, so you can communicate with other users.
```C
char message[] = "Incoming message";

char key[] = "CIPHER"; // === {'C','I','P','H','E','R'};
for(int i = 0; i < strlen(message); i++) {
    message[i] = message[i] ^ key[i % (sizeof(key)/sizeof(char))];
}
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate, following the standard.

## License
[MIT](https://choosealicense.com/licenses/mit/)

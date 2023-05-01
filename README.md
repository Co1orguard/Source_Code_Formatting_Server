
## About

This project is part of the Tennessee Technological University CSC-2770 Assignment 4. It is written in C++. The purpose of this program is to be a server that can handle requests from a netcat client. The requests are in the form of

```
ASTYLE
style=java
SIZE=78

#include <stdio.h>
int main() {
       printf("Hello world");
return 0;
}
```

and the server will respond with something like:


```
OK
SIZE=75

#include <stdio.h>
int main() {
       printf("Hello world");
       return 0;
}
```


## Usage

You can use the following Linux command to compile and run the program:

- `make` 
- `make run`

This will start the server listening on localhost:8007

You can then start a netcat on localhost:8007 and type in your request

- `netcat localhost 8007`


## Requirements 

You will need the following installed in order for `compile.sh` to run properly:
- gcc compiler 

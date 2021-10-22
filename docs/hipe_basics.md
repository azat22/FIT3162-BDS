# Hipe API 
This documentation is for the Hipe API, including common functions, use-cases and examples. 

The Hipe API is designed to be used with C and C++ programs. The API offers powerful functionality. The design of the API is compact and general rather than user-friendly, so you may find that there is a lot of boilerplate code that needs to be used for achieving simple tasks. We hope that this guide makes it easier to understand how to use the Hipe API.

## First steps
In every C/C++ file in your program that makes any calls to the Hipe API, you must include the Hipe header file. 

```
#include <hipe.h>
```
This is assuming that Hipe is installed and configured correctly on your system. Please [go here](start_here.md) to see how to install Hipe if you haven't already. 

When compiling your application, pass the following argument to your compiler/linker so that the Hipe library is linked to your program:

```
-lhipe
```

In Hipe, each application is granted a single frame by the Hipe server when it connects to it. A frame is a rectangular area of the screen that the application can use to interact with the user. 

### hipe_open_session()

This function is used to create a new Hipe session. It sends a request to the display server to create and request a frame. It returns a hipe_session value if successful. If the function fails, it returns 0, and the error message is printed to stderror. 

```
hipe_session hipe_open_session(const char* host_key, const char* socket_path, const char* key_path, const char* client_name)
```

The arguments passed to the function are as follows:

- host_key (optional) is a unique key-string that may be supplied by a particular Hipe environment (such as a framing manager). In most cases, you should pass 0 (a null value, not the string "0") and let libhipe obtain the key automatically from environment variables and hostkey files.
- socket_path (optional) is the location of the Hipe display server socket file. In most cases you should pass 0 and let libhipe deduce the socket path automatically from environment variables and fallback paths.
- key_path (optional) is the location of a file containing a valid host-key for connection to a Hipe environment. In most cases you should pass 0 and let libhipe deduce this information automatically from environment variables and fallback paths.
- client_name is a short name that identifies the application process. It is quite reasonable to use argv[0] here. This short name also serves as a temporary title placeholder until you set the frame title properly.

Here is a typical use-case for this function:

```'
hipe_session session;
session = hipe_open_session(0, 0, 0, "To-do list");
if(!session) exit(1);
```

In this case, request a new hipe session, a frame called 'To-do list'. We have not used any of the optional arguments in this case, it's just a simple frame request. 

Notice that we check the return value, if the variable session is 0, it means that the frame creation failed, and so we exit the program. 

### hipe_close_session()

This function closes the hipe_session passed to it and frees any associated resources. The application frame is destroyed. 

```
short hipe_close_session(hipe_session session)
```

It returns a 0 on success. 

Typical use-case:

```
hipe_close_session(session);  // the passed session is closed
```

### hipe_instruction_init()

This is a very important function in Hipe. Any hipe_instruction type variable must be initialised before use. The reasoning is that since code is written in C, the internal pointers of the variable may not be null, it may point to some garbage value that will break the program if you use it to call Hipe API functions. 

```
void hipe_instruction_init(hipe_instruction* obj)
```

Must be called on any new hipe_instruction instance (excluding shallow copies of an existing instance).

Must also be called on a hipe_instruction object whose values have been populated manually by the user (e.g. by assigning as arguments string pointers that are not owned by the hipe_instruction object itself) and is now going to be re-used by a Hipe API function.

Typical usage:

```
hipe_instruction event;
hipe_instruction_init(&event);
```
In the above example, we'd like to create a hipe_instruction variable called event, to use to capture events from the keyboard (for example). After creating it, we initialise it by passing its address to hipe_instruction_init(), so that it is safe for use. 



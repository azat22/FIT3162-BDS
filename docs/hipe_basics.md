# Hipe API 

This documentation file expands upon the documentation for Hipe written by Daniel Kos, found [here](http://hipe.tech/?l1=documentation). All rights belong to Daniel Kos under the license associated with Hipe. 

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


### hipe_instruction_clear()

This functoin frees memory allocated within the instruction, and makes the hipe_instruction variable ready for re-use. 

```
void hipe_instruction_clear(hipe_instruction* obj)
```

Do not use this function to clear a hipe_instruction for which you have allocated values yourself. Instead, de-allocate the argument strings in a way consistent with how you allocated them, then call hipe_instruction_init() to re-initialise the values into a clean state.


### hipe_instruction_copy()

Deeply copies values from one hipe_instruction instance to another, allocating memory for storage of values in the destination instance.

```
void hipe_instruction_copy(hipe_instruction* dest, hipe_instruction* src);
```

- dest is the hipe_instruction instance into which values will be be copied. dest should be in a clean state, as hipe_instruction_copy() does not do any initialisation or de-allocation of any existing data in dest.
- src is the hipe_instruction instance values are to be copied from.
When done with the instruction values in dest, you should free the memory associated with its internal variables using hipe_instruction_clear().

### hipe_next_instruction()

Gets the next instruction from the display server, with an optional blocking wait.

```
short hipe_next_instruction(hipe_session session, hipe_instruction* instruction_ret, short blocking)
```

Returns 1 if an instruction has been received, 0 if no instruction is available at the present moment and blocking was not requested, or -1 if disconnection from the server has occurred.

- session is the session handle that was obtained when hipe_open_session() was called.
- The received instruction is copied into *instruction_ret. instruction_ret must have been correctly initialised but does not need to have been cleared before calling this function. It will be cleared and automatically for your convenience, then repopulated with the next received instruction (if available).

Sample usage:

```
hipe_instruction event;
hipe_instruction_init(&event);
    
/* Main loop of the app. We wait for any events that are triggered by user actions, 
and call the event-handler functions accordingly */

do {
    // Get the next instruction
    hipe_next_instruction(session, &event, 1);
    
    switch(event.requestor) 
    {
        // Based on the event requestor values that we defined, call the appropriate event-handler
        // switch cases go here to handle events
    }
} while(event.opcode != HIPE_OP_FRAME_CLOSE); //repeat until window closed.
```

Here you can see how we use hipe_next_instruction() to fetch the next event from the display server. We are continously asking the server for new events. This bit of code is typical use for an event-driven application, where the event loop continuously runs, checking for new events (for example, user clicks, keyboard events, etc.). 

Notice how at the end of do..while loop we check if the received event's op_code is a frame close. This would indicate that the user has closed the frame, intending to shut down the program. Therefore, we can break from the loop and handle any termination conditions to gracefully exit the program. 

### hipe_await_instruction

A variation of the hipe_next_instruction function that waits for a specific type of instruction. There is a blocking wait until an instruction with the specified opcode arrives. When this happens, that instruction will jump the queue and be returned first.

```
short hipe_await_instruction(hipe_session session, hipe_instruction* instruction_ret, short opcode)
```

Returns 1 if an instruction of type opcode has been received or -1 if disconnection from the server has occurred.

- session is the session handle that was obtained when hipe_open_session() was called.
- The received instruction is copied into *instruction_ret. instruction_ret must have been correctly initialised but does not need to have been cleared before calling this function. It will be cleared and automatically for your convenience, then repopulated with the next received instruction.
- opcode is the instruction code to wait for.

This function should only be used in circumstances where you have sent the server a request with a guaranteed response, and have not called hipe_next_instruction() since then. Otherwise your application may hang waiting for an instruction that may never arrive (or worse, might have already arrived but been buffered in the received-instructions queue by hipe_next_instruction(), which is not checked by hipe_await_instruction()).

Sample usage:

```
void newListEntryInput(hipe_session session) 
{
    /* Create a hipe instruction to 'catch' the instruction sent back by the server
    This instruction will contain the input from the user in the dialog input box */
    hipe_instruction listenForInput;
    hipe_instruction_init(&listenForInput); // As always, hipe instructtions need to be initalised before use
    
    if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) 
    {
        // do our event happened, do something
....
}
```

In the above code snippet, we have a function that listens for input from the user when a text dialog box is opened. We can see how we used hipe_await_instruction to listen for a specific op_code, in this case it's HIPE_OP_DIALOG_RETURN, and we will execute some actions when this op_code is returned from the server. 

Notice that in this case we are guaranteed to receive the HIPE_OP_DIALOG_RETURN from the server, since we have already opened a dialog box. The user will either have to close the dialog box (without inputting anything), or enter some text and click 'OK' on the dialog box. In either case, we will get a return value from the server, and we can handle if effectively. 

However, this being a blocking function call, you must carefully consider the logic of your program to ensure that the program doesn't hang while waiting on a reply from the server that doesn't arrive. 

### hipe_send_instruction()
Transmits an instruction to the display server.

Returns 0 on success or -1 if there is no connection to the display server (or the connection has failed).

- session is the session handle that was obtained when hipe_open_session() was called.
- instruction is the Hipe instruction to transmit to the display server.

This is a basic function to send an instruction to the display server. 

Sample usage:

```
hipe_instruction openDialogBox;
hipe_instruction_init(&openDialogBox);
openDialogBox.opcode = HIPE_OP_DIALOG;
openDialogBox.arg[0] = 'Title of dialog box';
openDialogBox.arg[1] = 'Text of dialog box';
hipe_instruction_send(session, openDialogBox);
```

Here, we have created an instruction called openDialogBox, and we assigned it the opcode to open a dialog box. We have populated the required arguments (title and text) of dialog box, and sent the instruction to the server. 


### hipe_send()

As you can see above, the hipe_instruction_send() is quite inconvenient to use. We have to manually intitialise a hipe_instruction variable, assign it the op_code, and assign the required arguments, and only then can we send it to the server. 

We might make hundreds of such requests to the server to achieve the result that we want. This would make it difficult to do it manually, and we might end up with thousands of lines of what basically amounts to boilerplate code. 

To handle exactly this problem, we have a convenience function called hipe_send(). 

```
int hipe_send(hipe_session session, char opcode, uint64_t requestor, hipe_loc location, int n_args[, const char* arg, ...])
```

This function calls hipe_send_instruction internally and returns the return value from that call.

- session is the session handle that was obtained when hipe_open_session() was called.
- opcode, requestor, location and arg, ... are parts of the Hipe instruction to transmit. The strings arg[] need to be null-terminated, and their lengths are determined automatically. This convenience function should only be used when the arguments are simple text strings (due to the null-termination requirement).
- nargs is the number of arguments specified in the function call. Zero or more arguments may be specified at the end of the argument list.
- If an argument is explicitly specified but not used at a particular position, pass 0 (a null pointer) instead of a blank string.

This is a particularly useful function that you will be using a lot throughout your program. It makes it convenient to send instructions to the hipe_server without having to clutter your code with hipe_instruction variables being initialised and assigned, particularly when there are so many requests that need to be sent. 

Here is a simple usecase for this function. We want to add a CSS style rule for the HTML tag named 'body'. Of course, this is simple the body of the document. 

We call hipe_send() and pass to it our session, the op_code which in this case is HIPE_OP_ADD_STYLE, the first 0 is the requestor value which we don't need since this is a one-time use function and we don't receive anything back in return. The second 0 is the hipe_loc of where we want to add this style rule. The body is location 0 (by default). The '2' represents the number of arguments, with the first argument being the tag of the element, and the second element being the style rule that we want to apply. 

```
hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "body", "background-color: #32a885;");
```

You can see how useful this function actually is, since without it, the above instruction may need 6-7 lines of code, which has now been compressed to just 1 line of code. 

This helps keep the code neat and tidy, and makes it easier to read as well. As much as possible, we recommend using hipe_send() to reduce the number of repetitive lines of code in your program. 

Here is another example usage:

```
hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h1", "main-page-title");
hipe_loc main_page_title_loc = getLoc("main-page-title");
hipe_send(session, HIPE_OP_SET_TEXT, 0, main_page_title_loc, 1, "TO-DOIST");
// Apply some styling to the title
hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "text-align", "center"); 
```

Here, we add an h1 tag to the body and give it the ID 'main-page-title'. We then get the hipe_location of the ID using a utility function (not part of Hipe). 

Then, we use hipe_send() to set the text as well as add a CSS styling to the h1 tag that we appended. Notice that we used the hipe_location of the element in the hipe_send() call. This is very important, as when we have more elements in the DOM tree, we need to keep track of locations where we want to append new elements, and where we want to append style rules, for example. 

Regarding the number of arguments to hipe_send(), as you can see it varies based on the op_code of the instruction. You can check the OP_CODE guide to learn more about each function's arguments, what they represent, and how to handle them. 
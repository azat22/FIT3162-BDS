# To-doist - Organise your life!
## A Linux GUI created using Hipe 
### Or: How I Learned to Stop Worrying and Love the DOM
---
![Todoist](../img/todoist_homepage.png "To-doist")


This is a complete guide to making **To-doist** from scratch. You can consider this a primer on Hipe, teaching you the basics of using Hipe, and getting you started.

Checklist to complete before starting this guide:
- [x] Have a working Ubuntu Linux distro OS (recommended version 20.04)
- [x] Hipe is currently installed and configured by following [this](start_here.md) guide.
- [x] Had a nice cup of coffee and ready to get started (*this one is optional, but highly recommended*).

To get started, we need to discuss how a Hipe-based application works. 

## Understanding flow of control in Hipe
GUIs are generally event-driven applications. According to Wikipedia, [event-driven programming](https://en.wikipedia.org/wiki/Event-driven_programming) is a programming paradigm in which the flow of the program is determined by events such as user actions (mouse clicks, key presses), sensor outputs, or message passing from other programs or threads.

The basic idea is to have a main program loop that runs continuously, and *listens* for events, such as a button or a key being pressed. We create functions called *event-handlers* that are designed to handle the event, i.e. change the program state, I/O, visuals, etc. in response to the input. 

Now, it might be possible to create Hipe applications that have no events, but generally speaking, user-interactivity is made possible through the appropriate use of such events and event-handlers. 

**To-doist** is a to-do list program that allows the user to enter, edit and delete entries using buttons and text input. Looking at the screenshot, we can see there are three types of buttons:

- Add note button
- Edit note button
- Delete note button

Clicking any button must call the appropriate event-handler for that button, to handle the input.

Therefore, a high-level conceptual overview of making To-doist might be something like:

1. Create and place all initial elements (Title, Add note button, etc.)
2. Run main program loop, listens for input (*event*):
    - When Add note button is pressed, open dialog box to allow user to enter text
    - When Edit note button is pressed, open dialog box to allow user to edit text
    - When Delete note button is pressed, delete the corresponding list entry
3. If program exit value is called, exit the program. 

The above steps seem fairly simple. Of course, as always, the devil is in the details, where we ourselves struggled the most. Hopefully, with this guide in hand, you'll find it much easier. 

## Let's get started

To begin with, we'll create a file and call it 
```
todo.c
```
Open the file in your favourite code editor, and we can get started (We recommend Visual Studio Code). 

Like any good C file, ours start with declaring the header files that contain the functions that we want to use in this file. For our purposes, we need the following:

```
#include <hipe.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
```

Notice the 'hipe.h' file, which includes all the hipe functions that we'll be using. The others are fairly standard C library files. 

Next, we'll define the following constants:

```
#define NEW_LIST_ENTRY_EVENT 1
#define NEW_LIST_DELETE_EVENT 2
#define NEW_LIST_EDIT_EVENT 3
```

These are used as the 'requestor' codes passed on to Hipe, to differentiate which event is taking place. This will be discussed in the later parts where relevant, but for now, just trust us and put these in the file.

We define 





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
#define EXPORT_TO_FILE_EVENT 4
#define LOAD_FROM_FILE_EVENT 5
```

These are used as the 'requestor' codes passed on to Hipe, to differentiate which event is taking place. This will be discussed in the later parts where relevant, but for now, just trust us and put these in the file.

Next, we define a constant value to store the maximum number of entries we will save to file. We have set an arbitrary limit of 1000, but you can change it depending on how many entries you want to save, and if your system can handle it (most likely, it can handle a much larger number of entries). 

```
#define MAX_NUM_ENTRIES_IN_FILE 1000
```

We are done with the constants. Now, moving on to the global variables, we define some variables to keep track of program state. We will use these for various purposes throughout the program. Let's declare them as follows:

```
int counter; 
hipe_session session;
char* listEntries[MAX_NUM_ENTRIES_IN_FILE]; load from file
bool loaded_already;
```

- *Counter*: this is an int value that stores the *next* list entry index. Basically, if we have entered 4 entries into our list, this will store 5. We will use it to keep track of how many entries we've added, and as an index variable for the list entries array. 

- *session*: This is a 'hipe_session' type variable, it is the current session that we initialise and use for the application. This variable is very useful, and is required for every command that we send to the hipe server. Making it a global value means we have access to it in all the functions without having to pass it. 

- *listEntries*: This is an array that we declare to be of size MAX_NUM_ENTRIES_IN_FILE. It is a string array, with each entry storing one user entry. We will use it to store all user entries, as well as when we write and load from the file.

- *loaded_already*: A boolean value (true/false) to indicate if user has already loaded something from file. We don't want the user to keep loading the same entries from the same file and keep overwriting it, so we keep track of it using a boolean.

Next, we'll do some intitialisation, and declare some useful auxiliary functions. Initialisation is important to make sure that our program starts with the correct initial values for our variables. Auxiliary functions make our life easier by allowing us to do some commonly repeated tasks using a simple function call. 

Initialise the global variables to default values. 

```
void init()
{
    counter = 1;
    loaded_already = false;
    clearEntriesList(); // Calling auxiliary function to initialise the array
}
```

All elements in the array are set to empty. Note that "\0" is the termination character for a string. So we initialise all positions to be null initially. 
```
void clearEntriesList() 
{
    for(int i = 0; i < MAX_NUM_ENTRIES_IN_FILE; i++) 
    {
        listEntries[i] = "\0";
    }
}
```

Utility function to concatenate two strings and return the result
```
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
```

This is a useful utility function for hipe. Each element in a DOM layout has a 'location', in hipe, this location is of type 'hipe_loc', and we can use this location to interact and manipulate the elements. This utility function will return the hipe location of the element by its ID. (We must assign a unique ID to the element when creating it, in order to be able to find it).
```
hipe_loc getLoc(char* id) 
{
    hipe_send(session, HIPE_OP_GET_BY_ID, 0, 0, 1, id);
    hipe_instruction instruction;
    hipe_instruction_init(&instruction);
    hipe_await_instruction(session, &instruction, HIPE_OP_LOCATION_RETURN);
    return instruction.location;
}
```

Note that in the above, we have seen how we can send hipe instructions to the server. We highly recommend you read the hipe instructions page of the documentation to learn more about the format used. 

hipe_send() is a utility function that makes it easy to send a hipe instruction to the server. It creates a hipe_instruction, initialises it, and transmits it to the server with the OP_CODE and the arguments. It's a very useful function that we'll use throughout the program. We recommend you read the docs page on hipe_send to learn more about its formatting and usage. 

Here is another simple function that we'll use to display dialog box to the screen. A simple dialog box has just a title and a dialogText, which the function will receive as arguments. 
Then, it sends a HIPE_OP_DIALOG instruction to the hipe server.
```
void displaySimpleDialog(char* title, char* dialogText) 
{
    hipe_send(session, HIPE_OP_DIALOG, 0,0, 2, title, dialogText);
}
```

# main()

Now, in the code file for Todoist, we first define all the functions that we use, and finally we define the main function at the end. This is standard practice as you must define your functions before you use them. 

But for this tutorial, it'll make it easier if we start from the main function, so you can actually see how and where we will call different functions. Once we're done with main(), we will define all the other required functions. This should make it easier to follow the program flow, as the execution of the program beings in main(). 

Let's define the main function as follows:

```
int main(int argc, char** argv)
{
    // ADD REST OF THE CODE HERE 
    return 0;
}
```

The first step is to call our initialisation function we defined above. To do this, we simply do:

```
init();
```

Next, we request a top-level application frame from the Hipe server. Our application will exist within this top level frame, and occupy the entire screenspace within the frame. We do:

```
session = hipe_open_session(argc>1 ? argv[1] : 0, 0, 0, "To-do list");
if(!session) exit(1);
```
Note that if for whatever reason, a valid session is not received, we will exit the program. We have made the title of the window "To-do list", feel free to change it to whatever you wish. 

We now have a hipe session that we can send commands to. The first step is to configure our application layout. This is the base layout of the application, where we can text, buttons, lines, images, etc. We will use a combination of Hipe instructions, HTML and CSS to do this. 

Let's add a background colour:
```
hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "body", "background-color: #32a885;");
```
If you're familiar with HTML/CSS, you can recornigse the 'body' tag, and the 'background-color' property. We simply send a command to the server, telling it to add a background color to the element with the ID 'body'. 


Let's add a title (header) and a subtitle to the app.
```
hipe_send(session, HIPE_OP_APPEND_TAG, 0, 0, 2, "h1", "main-page-title");
hipe_loc main_page_title_loc = getLoc("main-page-title");
hipe_send(session, HIPE_OP_SET_TEXT, 0, main_page_title_loc, 1, "TO-DOIST");
```

You can see that we append an 'h1' tag to the body (the argument 0 represents body, which has hipe_loc of 0). We give this tag the ID "main-page-title". Then, we get the location of the tag using the getLoc() utility function. We then set the text using another hipe_send, using the OP_CODE of HIPE_OP_SET_TEXT, and passing to it the location, number of arguments (= 1), and the text. 

Next, we'll apply some styling to the title.

```
hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "text-align", "center"); 
hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "font-family", "impact, sans-serif");
hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "margin-top", "0.5em");
hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "margin-bottom", "0em");
```

Let's add a subtitle, and add some styling to it.

```
hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h4", "main-page-subtitle");
hipe_loc main_page_subtitle_loc = getLoc("main-page-subtitle");
hipe_send(session, HIPE_OP_SET_TEXT, 0, main_page_subtitle_loc, 1, "Organise your life!");
hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_subtitle_loc, 2, "text-align", "center");
hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_subtitle_loc, 2, "font-style", "italic");
```

We have used CSS properties here, so if you don't recognise them, please search up the CSS property to learn its usage. 

The app will have buttons, so let's add some styling to these:

```
hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "button", "background-color: #e7e7e7; border-radius: 8px;"); 
```

Notice how we can add more than one property in the string argument passed to the function, they just need to be separated by ';'.

Now, let's add a button that allows the user to export their list to a file. 
```
hipe_send(session, HIPE_OP_APPEND_TAG, 0, 0, 2, "div", "exportButtonDiv");
hipe_loc exportButtonDiv = getLoc("exportButtonDiv");
hipe_send(session, HIPE_OP_APPEND_TAG, 0, exportButtonDiv, 2, "button", "export-to-file-button");
hipe_loc export_to_file_button = getLoc("export-to-file-button");
```
We have wrapped it in a DIV for convenience, as DIVs have more predictable behaviour with CSS. 

To the same DIV, we will also add a button that allows the user to import their list from file.

```
hipe_send(session, HIPE_OP_APPEND_TAG, 0, exportButtonDiv, 2, "button", "load-from-file-button");
hipe_loc load_from_file_button = getLoc("load-from-file-button");
hipe_send(session, HIPE_OP_APPEND_TEXT, 0, export_to_file_button, 1, "Export to file");
hipe_send(session, HIPE_OP_APPEND_TEXT, 0, load_from_file_button, 1, "Load from file");
hipe_send(session, HIPE_OP_SET_STYLE, 0, exportButtonDiv, 2, "position", "absolute");
hipe_send(session, HIPE_OP_SET_STYLE, 0, exportButtonDiv, 2, "right", "0");
hipe_send(session, HIPE_OP_SET_STYLE, 0, exportButtonDiv, 2, "margin-right", "1em");
hipe_send(session, HIPE_OP_SET_STYLE, 0, exportButtonDiv, 2, "margin-top", "0.5em");
```

Adding a horizontal line to act as a divider between the header section and body:

```
hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr"); 
```

Now, we add a button for adding a new entry, the user can click it, and it should open a text entry box. 

```
hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", "newListEntryDialogButtonDiv");
hipe_loc newListEntryDialogButtonDivLoc = getLoc("newListEntryDialogButtonDiv");
hipe_send(session, HIPE_OP_APPEND_TAG, 0, newListEntryDialogButtonDivLoc, 2, "button", "newListEntryDialogButton");
hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "#newListEntryDialogButtonDiv", "text-align:center"); 
hipe_loc newListEntryDialogButton = getLoc("newListEntryDialogButton");
hipe_send(session, HIPE_OP_APPEND_TEXT, 0,newListEntryDialogButton, 1, "Add new entry");
```

Now, here comes the important part with the buttons. We need to add event handlers for the buttons. Basically, we pass a value to hipe for each button we created, and this value is unique. It tells hipe to send this value back to the session whenever that button is clicked. This way, we can differentiate between the 'events', so we can tell which button is clicked. Accordingly, we can call the function that handles that specific event.

```
hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_ENTRY_EVENT, newListEntryDialogButton, 1, "click");
hipe_send(session, HIPE_OP_EVENT_REQUEST, EXPORT_TO_FILE_EVENT, export_to_file_button, 1, "click");
hipe_send(session, HIPE_OP_EVENT_REQUEST, LOAD_FROM_FILE_EVENT, load_from_file_button, 1, "click");
```

We're almost done with the main() function. Next, we create a hipe_instruction and intialise it. We'll use this instruction to listen for instructions from the server, and handle it accordingly. 

```
hipe_instruction event;
hipe_instruction_init(&event);
```

For the main loop, we will use a do...while loop, which continuously fetches instructions from the server. Based on the 'requestor' value, we will handle it using a switch statement. This requestor value is our predefined values for the buttons that passed on to the server. 

```
do
{
    hipe_next_instruction(session, &event, 1);
    switch(event.requestor) 
    {
        case NEW_LIST_ENTRY_EVENT:
        {
            newListEntryDialog(); 
            newListEntryInput(session);
            break;
        }   
        case NEW_LIST_DELETE_EVENT:
        {
            deleteListEntry(session);
            break;
        }
        case NEW_LIST_EDIT_EVENT:
        {
            editListEntry(session);
            break;
        }
        case EXPORT_TO_FILE_EVENT:
        {
            exportToFile();
            break;
        }
        case LOAD_FROM_FILE_EVENT:
        {
            loadFromFile(session);
            break;
        }
    }
} while(event.opcode != HIPE_OP_FRAME_CLOSE);
```

And that's it for the main function. That's all that we need there. Notice that we used several functions in the switch statement, and we have to define these now. The functions are:

- newListEntryDialog()
- newListEntryInput()
- deleteListEntry()
- editListEntry()
- exportToFile()
- loadFromFile()

The name of the function should clue you in on what it does. We'll tackle each one separetely. 

# newListEntryDialog()

This is a simple function that sends one Hipe instruction to the server. 

```
void newListEntryDialog() 
{
    hipe_send(session, HIPE_OP_DIALOG_INPUT, 0,0, 4, "New note", "Start writing below: ", "Write here");
}
```
This instruction tells Hipe to open a dialog box with a text input entry field, which allows the user to enter input that they want to add to the list. 

# newListEntryInput()

What happens to the user input from the dialog box in the function above? How do we get the input to process it?

The answer is that Hipe sends an instruction back to the session, called HIPE_OP_DIALOG_RETURN. This instruction will contain the input from the user inside it. We have to listen for this instruction, and catch it so we can get the data from it. 

Here is the entire code for the function:

```
/* Create a hipe instruction to 'catch' the instruction sent back by the server
    This instruction will contain the input from the user in the dialog input box */
    hipe_instruction listenForInput;
    hipe_instruction_init(&listenForInput); // As always, hipe instructtions need to be initalised before use
    
    if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) 
    {
        // We await until we get a hipe instruction that matches the required op_code, which is 
        // HIPE_OP_DIALOG_RETURN, meaning that the server has sent the value back to the app session
        if(listenForInput.arg[0] != '\0') {
            // If the content of the user entry is not empty, then we add it to the list
            // In case the content is empty, then the user has entered nothing into the text box, so we ignore
            // Creating a unique 'entryNumber' string for each list entry. This uses the global counter value that we have. 
            char entryNumber[50];
            sprintf(entryNumber, "%d", counter);
            char* uniqueEntryDivID = concat("entryDivID", entryNumber); // Now, the unique entry ID is something like entryDivID12, for example, if counter = 12.

            // We use hipe_send to append a new tag to the body, which is just a div, giving it the ID we entered.
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, 0, 2, "div", uniqueEntryDivID);
            hipe_loc entryDivLoc = getLoc(uniqueEntryDivID);    // Getting the location of this div so we can populate it
            
            // Adding an 'arrow' symbol to the div, as a stylistic representation of a list entry
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 1, 	"➼ ");
            
            // Create a paragraph tag, giving it a unique ID, and appending it to the div
            char* uniqueTextID = concat("textID", entryNumber);
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 2, "p", uniqueTextID);
            hipe_loc p_loc = getLoc(uniqueTextID);  // Get its location 

            // Center the paragraph tag using CSS, accessed via its hipe_location
            hipe_send(session, HIPE_OP_SET_STYLE, 0, p_loc, 2, "display", "inline");
            // Populate the p tag with the text received from the user input
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, p_loc, 1, listenForInput.arg[0]);
            listEntries[entryDivLoc] = listenForInput.arg[0];
            
            // Applying some CSS style rules to the div, giving it margins in all four directions to space it correctly.
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-top", "1em");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-left", "0.5em");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-right", "0.5em");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-bottom", "1em");

            // Create a unique ID for each delete button
            char* uniqueDeleteButtonID = concat("deleteButtonID", entryNumber);
            // Adding the delete button to the DIV
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 3, "button", uniqueDeleteButtonID, uniqueEntryDivID);
            hipe_loc deleteButton = getLoc(uniqueDeleteButtonID);
            // Add text and styling to the delete button
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, deleteButton, 1, "Delete entry"); 
            hipe_send(session, HIPE_OP_SET_STYLE, 0, deleteButton, 2, "font-family", "impact");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, deleteButton, 2, "float", "right");

            // Create a unique ID for each edit button
            char* uniqueEditButtonID = concat("editButtonID", entryNumber);
            // Adding the edit button to the DIV
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 3, "button", uniqueEditButtonID, uniqueEntryDivID);
            hipe_loc editButton = getLoc(uniqueEditButtonID);
            // Add text and styling to the edit button
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, editButton, 1, "Edit entry"); 
            hipe_send(session, HIPE_OP_SET_STYLE, 0, editButton, 2, "font-family", "impact");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, editButton, 2, "float", "right");

            // Add a horizontal line - acts as a separator between the entries
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 1, "hr");

            //requests events for these buttons (delete, edit)
            hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_DELETE_EVENT, deleteButton, 2, "click", uniqueEntryDivID);
            hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_EDIT_EVENT, editButton, 2, "click", uniqueEntryDivID);
            counter++; // increment the global counter since we have added an entry
        }
    }
```

Notice how we first create a hipe_instruction and intialise it before we can use it. As always, C may have some garbage in the memory locations, so its important to initialise variables to have the proper starting values, so we don't run into any memory errors later on. 

Next, we wait until we receive the HIPE_OP_DIALOG_RETURN instruction from the server, we do this by:
```
 if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) 
    {
      ...
```
The function hipe_await_instruction() will only return 1 if it receives an instruction from Hipe with the specified OP_CODE. Otherwise, it'll return 0.

So, once we receive the HIPE_OP_DIALOG_RETURN, it'll be saved to the hipe_instruction that we declared, which is listenForInput. Then, we'll extract the user input from the arguments of this instruction, and create the required DIVs and buttons, so we can add this entry to the list. 

```
if(listenForInput.arg[0] != '\0') {
```
The above line is to check if the user input is empty. If it is, then we don't want to do anything. If it's not empty, then there is some text there that we should add to the list as an entry. '\0' is the zero characeter in C, and also denotes an empty string (it's also the termination character for a string in C). 

We then create DIVs, add the text to it, and add two buttons, 'Edit entry' and 'Delete entry' buttons. It's done using the commands that we've already seen. Please refer to the Hipe documentation for any function that you're unsure about. 

At the end, we add listeners to the buttons, so Hipe can send back the appropriate event code whenever the button is clicked. 
```
//requests events for these buttons (delete, edit)
hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_DELETE_EVENT, deleteButton, 2, "click", uniqueEntryDivID);
hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_EDIT_EVENT, editButton, 2, "click", uniqueEntryDivID);
counter++; // increment the global counter since we have added an entry
```

# deleteListEntry()
This function is called whenever the user clicks the 'Delete' button on an entry. The code for the function is:

```
void deleteListEntry(hipe_session session) 
{
    hipe_instruction listenForDeleteEvent;
    hipe_instruction_init(&listenForDeleteEvent);
    if(hipe_await_instruction(session, &listenForDeleteEvent, HIPE_OP_EVENT) == 1) 
    {
        // Delete based on location, we know div location is 2 less than the button location
        hipe_loc deleteLocDiv = listenForDeleteEvent.location - 2;
        hipe_send(session, HIPE_OP_DELETE, 0, deleteLocDiv, 2, "button", "deleteNoteDiv");
        listEntries[deleteLocDiv] = "\0";
    }
}
```

As before, we create a hipe_instruction and listen for a hipe_event. We will delete using the location of the DIV for that entry, which contains the text of the entry, and also the edit and delete buttons. Notice that we calculate this location, by taking the location of the delete button, and subtracting 2 from it. In Hipe, all elements that are added have sequential integer locations, and since the delete button is the second item added to the entry DIV when we create it, it will have a location which is two greater than the parent DIV. 

The deletion is done using the HIPE_OP_DELETE instruction sent to the server. We simply need to provide it a location to delete. 
We also remove the entry from the listEntries array. 


# editListEntry()
This function is used to edit the text for a particular entry. It is called when the user clicks the 'Edit' button next to an entry. 

Here is the code for this function:

```
// Function to deal with the input from the edit button dialog box
void editListEntry(hipe_session session) 
{
    hipe_instruction listenForEditEvent;
    hipe_instruction_init(&listenForEditEvent);
    // Wait for button press, and store it in the hipe_instruction we created above
    if(hipe_await_instruction(session, &listenForEditEvent, HIPE_OP_EVENT) == 1) 
    {
        // Find the location of the div
        hipe_loc editLocDiv = listenForEditEvent.location - 2;
        // Get content of the div (the text that is currently in the list entry)
        hipe_send(session, HIPE_OP_GET_CONTENT, 0, editLocDiv, 0);
        // Await the reply from the server with the required content from the entry
        if(hipe_await_instruction(session, &listenForEditEvent, HIPE_OP_CONTENT_RETURN) == 1) 
        {
            // Call the auxiliary function to open a dialog box, passing to it the current list entry contents
            editListEntryDialog(session, listenForEditEvent.arg[0]);
        }
        hipe_instruction listenForInput;
        hipe_instruction_init(&listenForInput);
        // Await reply from dialog box
        if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) 
        {
            if(listenForInput.arg[0] != '\0') {
                // If there is some text, we update the list entry
                hipe_send(session, HIPE_OP_SET_TEXT, 0, editLocDiv, 1, listenForInput.arg[0]);
                listEntries[editLocDiv - 1] = listenForInput.arg[0];
            }
        }
    }
}
```
Notice that we send a HIPE_OP_GET_CONTENT instruction to the server. This instruction requests the contents of the specified hipe_loc to be sent back to the session. In this case, this content is the text for that entry. We listen for a HIPE_OP_CONTENT_RETURN to fetch the content of the DIV. Using the text we received, we call an auxiliary function editListEntryDialog(), passing this text over to it. 


# editListEntryDialog()



# exportToFile()




# loadFromFile()




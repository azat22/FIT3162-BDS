/*
TO-DOIST - A To-do list program implemented using Hipe. 
An FIT3162 Project - Semester 2, 2021.
Team 23
*/

#include <hipe.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

// Define event handling requestors for various events in the app
#define NEW_LIST_ENTRY_EVENT 1
#define NEW_LIST_DELETE_EVENT 2
#define NEW_LIST_EDIT_EVENT 3

// Defining global variables to be used in program
int counter = 1; // Used to append to div ID, giving each 'note' div a unique ID 
hipe_session session; // The primary hipe session that the program runs on

// Utility function to concatenate two strings and return the result
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

// Function to get the hipe_location of an element by its ID
// Returns the location as a hipe_loc value
hipe_loc getLoc(char* id) {
    hipe_send(session, HIPE_OP_GET_BY_ID, 0, 0, 1, id);
    hipe_instruction instruction;
    hipe_instruction_init(&instruction);
    hipe_await_instruction(session, &instruction, HIPE_OP_LOCATION_RETURN);
    return instruction.location;
}

// Create a new entry in the list by calling HIPE_OP_DIALOG_INPUT
void newListEntryDialog() {
    hipe_send(session, HIPE_OP_DIALOG_INPUT, 0,0, 4, "New note", "Start writing below: ", "Write here");
}

// Function to handle the input entered by the user when entering a new note
void newListEntryInput(hipe_session session) {
    /* Create a hipe instruction to 'catch' the instruction sent back by the server
    This instruction will contain the input from the user in the dialog input box */
    hipe_instruction listenForInput;
    hipe_instruction_init(&listenForInput); // As always, hipe instructtions need to be initalised before use
    
    if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) {
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
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 1, 	"??? ");
            
            // Create a paragraph tag, giving it a unique ID, and appending it to the div
            char* uniqueTextID = concat("textID", entryNumber);
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 2, "p", uniqueTextID);
            hipe_loc p_loc = getLoc(uniqueTextID);  // Get its location 

            // Center the paragraph tag using CSS, accessed via its hipe_location
            hipe_send(session, HIPE_OP_SET_STYLE, 0, p_loc, 2, "display", "inline");
            // Populate the p tag with the text received from the user input
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, p_loc, 1, listenForInput.arg[0]);
            
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
}

// Function to delete a list entry - called when delete button is pressed
void deleteListEntry(hipe_session session) {
    hipe_instruction listenForDeleteEvent;
    hipe_instruction_init(&listenForDeleteEvent);
    
    if(hipe_await_instruction(session, &listenForDeleteEvent, HIPE_OP_EVENT) == 1) {
        // Delete based on location, we know div location is 2 less than the button location
        hipe_loc deleteLocDiv = listenForDeleteEvent.location - 2;
        hipe_send(session, HIPE_OP_DELETE, 0, deleteLocDiv, 2, "button", "deleteNoteDiv");
    }
}

// This function is called when edit button is pressed - opens a dialog box, sending it the current text which is in the entry
void editListEntryDialog(hipe_session session, char* text) {
    hipe_send(session, HIPE_OP_DIALOG_INPUT, 0,0, 4, "Edit note", "Start writing below: ", text);
}

// Function to deal with the input from the edit button dialog box
void editListEntry(hipe_session session) {
    hipe_instruction listenForEditEvent;
    hipe_instruction_init(&listenForEditEvent);
    // Wait for button press, and store it in the hipe_instruction we created above
    if(hipe_await_instruction(session, &listenForEditEvent, HIPE_OP_EVENT) == 1) {
        // Find the location of the div
        hipe_loc editLocDiv = listenForEditEvent.location - 2;
        // Get content of the div (the text that is currently in the list entry)
        hipe_send(session, HIPE_OP_GET_CONTENT, 0, editLocDiv, 0);
        // Await the reply from the server with the required content from the entry
        if(hipe_await_instruction(session, &listenForEditEvent, HIPE_OP_CONTENT_RETURN) == 1) {
            // Call the auxiliary function to open a dialog box, passing to it the current list entry contents
            editListEntryDialog(session, listenForEditEvent.arg[0]);
        }
        hipe_instruction listenForInput;
        hipe_instruction_init(&listenForInput);
        // Await reply from dialog box
        if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) {
            if(listenForInput.arg[0] != '\0') {
                // If there is some text, we update the list entry
                hipe_send(session, HIPE_OP_SET_TEXT, 0, editLocDiv, 1, listenForInput.arg[0]);
            }
        }
    }
}

int main(int argc, char** argv)
{
    //Request a new top-level application frame from the Hipe server
    session = hipe_open_session(argc>1 ? argv[1] : 0, 0, 0, "To-do list");
    if(!session) exit(1);
    
    /* INTIAL SETUP - TITLE, BACKGROUND COLOUR, APPENDING BUTTONS, ETC. */
    // Change the background colour 
    hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "body", "background-color: #32a885;");
    // Add title to the app, showing the app name 
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h1", "main-page-title");
    hipe_loc main_page_title_loc = getLoc("main-page-title");
    hipe_send(session, HIPE_OP_SET_TEXT, 0, main_page_title_loc, 1, "TO-DOIST");
    // Apply some styling to the title
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "text-align", "center"); 
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "font-family", "impact, sans-serif");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "margin-top", "0.5em");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "margin-bottom", "0em");
    // Add subtitle and apply some styling
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h4", "main-page-subtitle");
    hipe_loc main_page_subtitle_loc = getLoc("main-page-subtitle");
    hipe_send(session, HIPE_OP_SET_TEXT, 0, main_page_subtitle_loc, 1, "Organise your life!");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_subtitle_loc, 2, "text-align", "center");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_subtitle_loc, 2, "font-style", "italic");
    // Add style rule for all buttons that appear in the app
    hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "button", "background-color: #e7e7e7; border-radius: 8px;"); 
    // Add a horizontal line, diving the app header from the body section
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr"); 
    // Add a div to hold the new list entry button
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", "newListEntryDialogButtonDiv");
    hipe_loc newListEntryDialogButtonDivLoc = getLoc("newListEntryDialogButtonDiv");
    // Add a button to the div and set its text
    hipe_send(session, HIPE_OP_APPEND_TAG, 0, newListEntryDialogButtonDivLoc, 2, "button", "newListEntryDialogButton");
    hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "#newListEntryDialogButtonDiv", "text-align:center"); 
    hipe_loc newListEntryDialogButton = getLoc("newListEntryDialogButton");
    hipe_send(session, HIPE_OP_APPEND_TEXT, 0,newListEntryDialogButton, 1, "Add new entry");

    //requests event for the button
    hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_ENTRY_EVENT, newListEntryDialogButton, 1, "click");
    
    hipe_instruction event;
    hipe_instruction_init(&event);
    
    /* Main loop of the app. We wait for any events that are triggered by user actions, 
    and call the event-handler functions accordingly */
    do {
        // Get the next instruction
        hipe_next_instruction(session, &event, 1);
        
        switch(event.requestor) {
            // Based on the event requestor values that we defined, call the appropriate event-handler
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

        }
    } while(event.opcode != HIPE_OP_FRAME_CLOSE); //repeat until window closed.
    
    return 0;
}


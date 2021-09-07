/* Test program to demonstrate dialog and related functionality
- Asking the framing manager (or Hipe if running at top level) to display a modal dialog on screen with multiple discrete options
- Requesting text input with list of suggested choices
- Selecting a FIFO resource to open.

*/


#include <hipe.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define NEW_LIST_ENTRY_EVENT 3
#define NEW_LIST_ENTRY_INPUT 4
#define NEW_LIST_DELETE_EVENT 5

int counter = 1;

hipe_session session;

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

hipe_loc getLoc(char* id) {
    hipe_send(session, HIPE_OP_GET_BY_ID, 0, 0, 1, id);
    hipe_instruction instruction;
    hipe_instruction_init(&instruction);
    hipe_await_instruction(session, &instruction, HIPE_OP_LOCATION_RETURN);
    //We have changed this, it was returning instruction.location
    return instruction.location;
}

void newListEntryDialog() {
    hipe_send(session, HIPE_OP_DIALOG_INPUT, 0,0, 4, "New note", "Start writing below: ", "Write here");
}

void newListEntryInput(hipe_session session) {
    hipe_instruction listenForInput;
    hipe_instruction_init(&listenForInput);
    
    if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) {
        if(listenForInput.arg[0] != '\0') {
            // If the content of the user entry is not empty, then we add it to the list
            // div
            char entryNumber[50];
            sprintf(entryNumber, "%d", counter);
            char* uniqueEntryDivID = concat("entryDivID", entryNumber);

            hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", uniqueEntryDivID);
            hipe_loc entryDivLoc = getLoc(uniqueEntryDivID);
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 1, "hr");
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 2, entryNumber);
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 2, ".  ");
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 2, listenForInput.arg[0]);

            // Create a unique ID for each button
            char* uniqueButtonID = concat("buttonID", entryNumber);
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 2, "button", uniqueButtonID, uniqueEntryDivID);
            hipe_loc deleteButton = getLoc(uniqueButtonID);
            //add text to the buttons
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, deleteButton, 1, "Delete entry");
            //requests events for these buttons (we designate requestor codes 1,2,3 to identify these quickly)
            hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_DELETE_EVENT, deleteButton, 2, "click", uniqueEntryDivID);

            counter++;
        }
    }
}

void deleteListEntry(hipe_session session) {
    hipe_instruction listenForDeleteEvent;
    hipe_instruction_init(&listenForDeleteEvent);
    
    if(hipe_await_instruction(session, &listenForDeleteEvent, HIPE_OP_EVENT) == 1) {
        hipe_loc deleteLocDiv = listenForDeleteEvent.location;

        hipe_send(session, HIPE_OP_APPEND_TAG, 0, 0, 1, "h1", "some_id");
        hipe_loc some_loc = getLoc("some_id");

        char temp[50];
        sprintf(temp, "%d", listenForDeleteEvent.arg[1]);

        hipe_send(session, HIPE_OP_APPEND_TEXT, 0, some_loc, 1, temp);
        
        

        hipe_send(session, HIPE_OP_DELETE, 0, deleteLocDiv, 2, "button", "deletedeleteButton");
        
    }
}

int main(int argc, char** argv)
{
    //Request a new top-level application frame from the Hipe server
    session = hipe_open_session(argc>1 ? argv[1] : 0,0,0,"To-do list");
    if(!session) exit(1);

    //Create and place buttons
    //hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr"); //horizontal line

    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h1", "main-page-title");
    hipe_loc main_page_title_loc = getLoc("main-page-title");
    
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr"); //horizontal line
    
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", "newListEntryDialogButtonDiv");
    hipe_loc newListEntryDialogButtonDivLoc = getLoc("newListEntryDialogButtonDiv");
    hipe_send(session, HIPE_OP_APPEND_TAG, 0, newListEntryDialogButtonDivLoc, 2, "button", "newListEntryDialogButton");


    // hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", "newListEntryDialogButtonDiv2");
    // hipe_loc newListEntryDialogButtonDivLoc2 = getLoc("newListEntryDialogButtonDiv2");
    // hipe_send(session, HIPE_OP_APPEND_TAG, 0,newListEntryDialogButtonDivLoc2, 2, "button", "newListEntryDeleteButton");
    
    hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "h1", "text-align:center"); 
    
    hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "#newListEntryDialogButtonDiv", "text-align:center"); 
    
    hipe_send(session, HIPE_OP_SET_TEXT, 0, main_page_title_loc, 1, "TO-DOIST");
    
    hipe_loc newListEntryDialogButton = getLoc("newListEntryDialogButton");

    //add text to the buttons
    hipe_send(session, HIPE_OP_APPEND_TEXT, 0,newListEntryDialogButton, 1, "Add new entry");

    //requests events for these buttons (we designate requestor codes 1,2,3 to identify these quickly)
    hipe_send(session, HIPE_OP_EVENT_REQUEST, NEW_LIST_ENTRY_EVENT, newListEntryDialogButton, 1, "click");
    
    hipe_instruction event;
    hipe_instruction_init(&event);
    
    do {
        hipe_next_instruction(session, &event, 1);
        
        switch(event.requestor) {
            case NEW_LIST_ENTRY_EVENT:   {
                newListEntryDialog(); 
                newListEntryInput(session);
                break;
            }

            case NEW_LIST_DELETE_EVENT:
                    {
                        //hipe_loc deleteLocDiv = getLoc("entryDiv");
                        //hipe_send(session, HIPE_OP_DELETE, 0, deleteLocDiv, 2, "button", "deletedeleteButton");
                        deleteListEntry(session);
                        // no idea how to find the location of the text itself???
                        // hipe_loc textLoc = getLoc("hr");
                        // hipe_send(session, HIPE_OP_DELETE, 0, textLoc, 2, "text", "deletetext");
                        break;
                    }

        }
    } while(event.opcode != HIPE_OP_FRAME_CLOSE); //repeat until window closed.


    return 0;
}

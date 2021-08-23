/* Test program to demonstrate dialog and related functionality
- Asking the framing manager (or Hipe if running at top level) to display a modal dialog on screen with multiple discrete options
- Requesting text input with list of suggested choices
- Selecting a FIFO resource to open.

*/


#include <hipe.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NEW_LIST_ENTRY_EVENT 3
#define NEW_LIST_ENTRY_INPUT 4

hipe_session session;

hipe_loc getLoc(char* id) {
    hipe_send(session, HIPE_OP_GET_BY_ID, 0, 0, 1, id);
    hipe_instruction instruction;
    hipe_instruction_init(&instruction);
    hipe_await_instruction(session, &instruction, HIPE_OP_LOCATION_RETURN);
    return instruction.location;
}

void newListEntryDialog() {
    hipe_send(session, HIPE_OP_DIALOG_INPUT, 0,0, 4, "New note", "Start writing below: ", "Write here");
}

void newListEntryInput(hipe_session session) {
    hipe_instruction listenForInput;
    hipe_instruction_init(&listenForInput);
    
    if(hipe_await_instruction(session, &listenForInput, HIPE_OP_DIALOG_RETURN) == 1) {
        hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr");
        hipe_send(session, HIPE_OP_APPEND_TEXT, 0,0, 2, listenForInput.arg[0]);
    }
}


int main(int argc, char** argv)
{
    //Request a new top-level application frame from the Hipe server
    session = hipe_open_session(argc>1 ? argv[1] : 0,0,0,"To-do list");
    if(!session) exit(1);

    //Create and place buttons
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr"); //horizontal line

    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h1", "main-page-title");
    hipe_loc main_page_title_loc = getLoc("main-page-title");
    
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr"); //horizontal line
    
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", "newListEntryDialogButtonDiv");
    hipe_loc newListEntryDialogButtonDivLoc = getLoc("newListEntryDialogButtonDiv");
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,newListEntryDialogButtonDivLoc, 2, "button", "newListEntryDialogButton");
    
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
        }
    } while(event.opcode != HIPE_OP_FRAME_CLOSE); //repeat until window closed.


    return 0;
}

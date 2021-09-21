/* 
To-do list program implemented using Hipe.
*/

#include <hipe.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define NEW_LIST_ENTRY_EVENT 1
#define NEW_LIST_DELETE_EVENT 2

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
            
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 1, "➼ ");
            // hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 2, ". ");
            
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, entryDivLoc, 2, listenForInput.arg[0]);
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-top", "1em");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-left", "0.5em");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-right", "0.5em");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, entryDivLoc, 2, "margin-bottom", "1em");

            // Create a unique ID for each button
            char* uniqueButtonID = concat("buttonID", entryNumber);
            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 3, "button", uniqueButtonID, uniqueEntryDivID);
            
            hipe_loc deleteButton = getLoc(uniqueButtonID);
            //add text to the buttons
            hipe_send(session, HIPE_OP_APPEND_TEXT, 0, deleteButton, 1, "Delete entry"); 
            hipe_send(session, HIPE_OP_SET_STYLE, 0, deleteButton, 2, "font-family", "impact");
            hipe_send(session, HIPE_OP_SET_STYLE, 0, deleteButton, 2, "float", "right");

            hipe_send(session, HIPE_OP_APPEND_TAG, 0, entryDivLoc, 1, "hr");

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
        hipe_loc deleteLocDiv = listenForDeleteEvent.location - 1;
        hipe_send(session, HIPE_OP_GET_ATTRIBUTE, 0, deleteLocDiv, 0);
        hipe_send(session, HIPE_OP_DELETE, 0, deleteLocDiv, 2, "button", "deletedeleteButton");

        hipe_instruction getNoteContent;
        hipe_instruction_init(&getNoteContent);
        hipe_send(session, HIPE_OP_GET_CONTENT, 0,0,0);


    }
}

int main(int argc, char** argv)
{
    //Request a new top-level application frame from the Hipe server
    session = hipe_open_session(argc>1 ? argv[1] : 0,0,0,"To-do list");
    if(!session) exit(1);

    hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "body", "background-color: #32a885;");

    //Create and place buttons
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h1", "main-page-title");
    hipe_loc main_page_title_loc = getLoc("main-page-title");
    
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "text-align", "center"); 
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "font-family", "impact, sans-serif");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "margin-top", "0.5em");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_title_loc, 2, "margin-bottom", "0em");
    
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h4", "main-page-subtitle");
    hipe_loc main_page_subtitle_loc = getLoc("main-page-subtitle");
    hipe_send(session, HIPE_OP_SET_TEXT, 0, main_page_subtitle_loc, 1, "Organise your life!");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_subtitle_loc, 2, "text-align", "center");
    hipe_send(session, HIPE_OP_SET_STYLE, 0, main_page_subtitle_loc, 2, "font-style", "italic");
    
    hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "button", "background-color: #e7e7e7; border-radius: 8px;"); 



    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 1, "hr"); //horizontal line
    
    hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", "newListEntryDialogButtonDiv");
    hipe_loc newListEntryDialogButtonDivLoc = getLoc("newListEntryDialogButtonDiv");
    hipe_send(session, HIPE_OP_APPEND_TAG, 0, newListEntryDialogButtonDivLoc, 2, "button", "newListEntryDialogButton");


    // hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "div", "newListEntryDialogButtonDiv2");
    // hipe_loc newListEntryDialogButtonDivLoc2 = getLoc("newListEntryDialogButtonDiv2");
    // hipe_send(session, HIPE_OP_APPEND_TAG, 0,newListEntryDialogButtonDivLoc2, 2, "button", "newListEntryDeleteButton");
    
    
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



# HTML elements and CSS styling using Hipe

One of the most common tasks we'll be doing using Hipe is creating HTML elements in our layout, and applying styling rules to it. Since Hipe has a DOM-based layout and rendering capability, we can, for the most part, use HTML and CSS instructions to create and layout elements in our application. 

This guide will teach you the basics of working with HTML elements in Hipe, and adding CSS style rules. 

Note that 'location' refers to the hipe_location of an element, which is an integer value that starts at 0 for the HTML body tag, and increments for any added element. 

## Auxiliary function to get an element's location by its ID

```
// Function to get the hipe_location of an element by its ID
// Returns the location as a hipe_loc value
hipe_loc getLoc(char* id) 
{
    hipe_send(session, HIPE_OP_GET_BY_ID, 0, 0, 1, id);
    hipe_instruction instruction;
    hipe_instruction_init(&instruction);
    hipe_await_instruction(session, &instruction, HIPE_OP_LOCATION_RETURN);
    return instruction.location;
}
```
The above is a useful auxiliary function to get an element's location by its ID. Note that we cannot force an element to have a specific location value, and the element's location value is determined automatically by Hipe.

Thus, it is also important to give all our element's meaningful ID values so we can get their location using this function. It also makes it easier for us when reading the code to identify what purpose a particular element serves. 

We normally use this command straight after appending a new element, as we generally need its location to modify it, such as adding CSS styles to it. 


## Appending a new tag 
We want to append a new HTML tag somewhere in the DOM tree. The first consideration is the parent location. Any HTML tag needs to be appended to an existing location, and it will be added as a child of that location (or element). 

By defauly, the 'body' tag has the location 0. Therefore any new tags that we want to append at the root level, can be appended to the body. 

Careful consideration will be needed as we want to have a logical structure for layout in the application. Appending all page elements to the body is a recipe for disaster, as we won't have the granular control over styling that we would otherwise have. 

Where possible, we recommend wrapping elements within DIVs, which is a generic container for elements in the DOM. DIVs are more predictable and have better styling properties compared to other elements. It's also how we can combine several elements that we want to be grouped together, such as buttons that we want to appear inline with each other. 

Here is a general append tag instruction:

```
hipe_send(session, HIPE_OP_APPEND_TAG, 0,0, 2, "h1", "main-page-title");
```

In this case, we are appending an h1 tag to the location 0, which is the body. 

```
hipe_send(session, HIPE_OP_APPEND_TAG, 0, 0, 2, "div", "exportButtonDiv");
hipe_loc exportButtonDiv = getLoc("exportButtonDiv");
hipe_send(session, HIPE_OP_APPEND_TAG, 0, exportButtonDiv, 2, "button", "export-to-file-button");
```
In this code snippet, we are appending a div to the body, and then getting the location of the div. Then, we are appending an HTML button element to the DIV. This is the more common and general case for appending tags, as mentioned before, we want to layout everything cleanly, and it makes sense to wrap related items within a DIV if we want to access them collectively, such as for applying CSS styling. 


## Adding CSS styling

There are several Hipe op_codes that are useful in adding CSS styling to elements. The two basic ones are:

- HIPE_OP_SET_STYLE: used to add one CSS rule to an element
- HIPE_OP_ADD_STYLE_RULE used to add one or more CSS rules to an element (or group of elements)

### HIPE_OP_SET_STYLE
This is a very simple instruction that can be used to specify one style rule to be applied to one element. 

- location: The tag element to be styled.
- arg[0]: The style property (e.g. "border").
- arg[1]: The new value of the style property (e.g. "1px solid")

Sample usage:

```
hipe_send(session, HIPE_OP_SET_STYLE, 0, exportButtonDiv, 2, "margin-left", "1em");
```

In the above example, we add a style rule to the element located at exportButtonDiv (this is the hipe_location of the element), and we add give it a 'margin-left' of '1em'. 

We can make more calls using this op_code if we want to add multiple styles to an element.


### HIPE_OP_ADD_STYLE_RULE

This op_code allows you to add multiple CSS rules to an element. 

- location: 0. Not used.
- arg[0]: The CSS descriptor of element(s) the rule will apply to (e.g. "div", "#element", etc.).
- arg[1]: A set of style properties, separated by semicolons (e.g. "border:0; width:100%;").


Sample usage:

```
hipe_send(session, HIPE_OP_ADD_STYLE_RULE, 0,0, 2, "button", "background-color: #e7e7e7; border-radius: 8px;"); 
```

In the above example, we have applied two different CSS style rules to the element 'button'. Notice that the hipe_location value is not used in this case, as this is a more general way of adding style rules across the entire document. 

The style rules are separated by semicolons ';', so we can write many different style rules in one line. 

There are some caveats to the usage of this instruction, the main one being that it's best to use the instruction before appending the element. 

From the offical Hipe documentation: 

"This instruction can be used any number of times before your program displays output, but are not guaranteed to be respected by the underlying WebKit engine once the first content element manipulations are performed. Therefore, style rules should generally be specified before anything else.

For styling individual elements after you have created them, use the HIPE_OP_SET_STYLE instruction instead.

Some combinations of CSS rules can produce undefined behaviour involving race conditions where application layouts may either render correctly or incorrectly each time the application runs. To avoid this, ensure that multiple layout elements are not defined purely relative to one another, particularly when using tables."

We can confirm that this instruction is more finnicky to use than HIPE_OP_SET_STYLE, so for adding styles to individual elements after creation, we recommend using HIPE_OP_SET_STYLE instead. 


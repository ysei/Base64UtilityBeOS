//
//  Base64Window.cpp
//  Base64Utility
//
//  Created by Josh Barrow on 3/14/12.
//  Copyright 2012 Jukaela Enterprises. All rights reserved.
//

#include <Alert.h>
#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <File.h>
#include <FilePanel.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Message.h>
#include <Path.h>
#include <String.h>
#include <TextView.h>
#include "Base64Window.h"
#include "base64.h"

const uint32 ENCODE_BUTTON_MESSAGE = 'enbm';
const uint32 DECODE_BUTTON_MESSAGE = 'dcbm';
const uint32 OPEN_FILE_MESSAGE = 'ofms';
const uint32 SAVE_FILE_MESSAGE = 'sfms';
const uint32 RESET_MESSAGE = 'rsms';
const uint32 QUIT_MESSAGE = 'qtms';
const uint32 ABOUT_MESSAGE = 'abms';

BMessage Base64Arch;

Base64Window::Base64Window(BRect frame) : BWindow (frame, "Base64 Utility", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
    // Instantiate the main BMenu
	BMenu *menu;
    
	menubar = new BMenuBar(Bounds(), "menubar");
	
    // Create the File menu
	menu = new BMenu("File");
	menu->AddItem(new BMenuItem("Reset", new BMessage(RESET_MESSAGE), 'R'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem("Quit", new BMessage(QUIT_MESSAGE), 'Q'));
	
    // Add the File menu to the menu bar
	menubar->AddItem(menu);
	
    // Create the help menu
	menu = new BMenu("Help");
	menu->AddItem(new BMenuItem("About...", new BMessage(ABOUT_MESSAGE)));
	
    // Add the help menu to the menu bar
	menubar->AddItem(menu);
	
    // Add the menubar to the main BWindow
	AddChild(menubar);
    
    // Create the main rect
	BRect mainRect(frame);
	mainRect.OffsetTo(0, 0);
    
    // Create the backgroudn view.  Simply a grey background that things can be added to
	BView *background = new BView(mainRect, NULL, B_FOLLOW_ALL_SIDES, 0);
    
	rgb_color color = ui_color(B_PANEL_BACKGROUND_COLOR);
    
    // No gradients!
	background->SetViewColor(color);
	background->SetLowColor(color);
    
	mainRect.InsetBy(5, 5);
    
    // Create the inner rects
	BRect innerRect(mainRect);
	innerRect.top += 15;
	innerRect.bottom -= 5;
	innerRect.right = innerRect.left + 185;
    
    // Create the BBox that will hold the encoding stuff
	BBox *encodeBox = new BBox(innerRect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM);
	encodeBox->SetLabel("Encode");
    
    // Add it to the background view
	background->AddChild(encodeBox);
    
    // Change the inner rect
	innerRect.left = innerRect.right + 5;
	innerRect.right = mainRect.right - 5;
	innerRect.bottom = mainRect.bottom - 5;
    
    // Create the Bbox that will hold the decoding stuff
	BBox *decodeBox = new BBox(innerRect, NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM);
	decodeBox->SetLabel("Decode");
    
    // Add it to the background view
	background->AddChild(decodeBox);
    
    // Add the background to the main BWindow
	AddChild(background);
    
    // Create the rects for the onscreen control.s
	BRect encodeButtonRect = BRect(10, 15, 175, 30);
	BRect encodeTextViewRect = BRect(10, 45, 175, 175);
	BRect decodeButtonRect = BRect(10, 150, 175, 30);
	BRect decodeTextViewRect = BRect(10, 15, 175, 145);
    
    // Instantiate the encode button and the decode button
	encodeButton = new BButton (encodeButtonRect, NULL, "Encode and Display...", new BMessage(ENCODE_BUTTON_MESSAGE));
	decodeButton = new BButton (decodeButtonRect, NULL, "Decode and Save...", new BMessage(DECODE_BUTTON_MESSAGE));
    
    // Instantiate the encode text view and decode text view
	encodeTextView = new BTextView(encodeTextViewRect, "encode_text_view", BRect(0, 0, 175, 175), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED);
	decodeTextView = new BTextView(decodeTextViewRect, "decode_text_view", BRect(0, 0, 175, 175), B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_PULSE_NEEDED);
    
    // Add all the controls to the appropriate boxes
	encodeBox->AddChild(encodeButton);
	encodeBox->AddChild(encodeTextView);
	decodeBox->AddChild(decodeButton);
	decodeBox->AddChild(decodeTextView);
    
    // Instantiate the open and save panels
	openPanel = new BFilePanel(B_OPEN_PANEL, &self, NULL, B_FILE_NODE, false, new BMessage(OPEN_FILE_MESSAGE), NULL);
	savePanel = new BFilePanel(B_SAVE_PANEL, &self, NULL, B_FILE_NODE, false, new BMessage(SAVE_FILE_MESSAGE), NULL);
    
    // Let BMessenger know that we're listening for messages
	BMessenger self(this);
};


bool Base64Window::QuitRequested()
{
	return true;
}

void Base64Window::MessageReceived(BMessage* message)
{
	BEntry entry;
	entry_ref ref;
	BPath path;
    
	switch(message->what) {
		case ENCODE_BUTTON_MESSAGE: {
			openPanel->Show();
            
			break;
		}
		case DECODE_BUTTON_MESSAGE: {
			savePanel->Show();
            
			break;
		}
		case OPEN_FILE_MESSAGE: {
			message->FindRef("refs", &ref);
            
			entry.SetTo(&ref);
			entry.GetPath(&path);
            
			encodeFile(path, entry);
            
			break;
		}
		case SAVE_FILE_MESSAGE: {
			decodeFile(message);
            
			break;
		}
		case RESET_MESSAGE: {
			encodeTextView->SelectAll();
			decodeTextView->SelectAll();
            
			encodeTextView->Delete();
			decodeTextView->Delete();
            
			break;
		}
		case QUIT_MESSAGE: {
			be_app->PostMessage(B_QUIT_REQUESTED);
            
			break;
		}
		case ABOUT_MESSAGE : {
			(new BAlert("About Base64 Utility", "Copyright 2012, Jukaela Enterprises. Visit us at http://www.jukaela.com.", "Close"))->Go();
            
			break;
		}
	}
}

void Base64Window::encodeFile(BPath pathName, BEntry entry)
{
	BFile file(pathName.Path(), B_READ_ONLY);
    
    off_t length;
    char *text;
    file.GetSize(&length);
    text = (char *) malloc(length);
	
	file.Read(text, length);
    
	std::string encoded = base64_encode(reinterpret_cast<const unsigned char *>(text), length);
    
	const char *textEncoded = encoded.c_str();
    
	text_run_array *currentSelection = new text_run_array;
    
	encodeTextView->SetText(textEncoded, currentSelection);
}

void Base64Window::decodeFile(BMessage *message)
{
 	entry_ref ref;
    status_t err;
    const char *name;
    
    BPath path;
    BEntry entry;
    FILE *file;
    
    if ((err=message->FindRef("directory", &ref)) != B_OK) {
        return;
    }
    
    if ((err=message->FindString("name", &name)) != B_OK) {
        return;
    }
    
    if ((err=entry.SetTo(&ref)) != B_OK) {
        return;
    }
    
    entry.GetPath(&path);
    path.Append(name);
    
    if (!(file = fopen(path.Path(), "w"))) {
        return;
    }
    
	string decodedString = base64_decode(decodeTextView->Text());
    
	err = fwrite(decodedString.c_str(), 1, decodeTextView->TextLength(), file);
    
    fclose(file);
}


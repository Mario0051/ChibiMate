# Chibi Viewer

A simple Windows application that displays animated GIFs with transparency, allowing characters to appear on your desktop.

## Features

- Transparent window with opaque GIF animation
- Multiple animation states (move, wait, sit, etc.)
- Automatic mode that switches between animations and moves around
- Manual mode to control animations yourself
- Pick up and drag the character with your mouse
- Import your own GIF animations from a folder

## How to Compile

1. Make sure you have a C++ compiler that supports C++11 or later (Visual Studio 2013 or newer recommended)
2. Open the project in Visual Studio or compile from command line:

```
cl ChibiViewer.cpp /EHsc /std:c++14 /link user32.lib gdi32.lib gdiplus.lib shlwapi.lib
```

## Controls

- **M**: Open/close the menu
- **A**: Toggle between Automatic and Manual mode
- **Spacebar**: In Manual mode, cycle through animations
- **Click and hold**: Pick up the character (displays "pick" animation)
- **Import button**: Select a folder with GIF animations
- **Quit button**: Close the application

## GIF Requirements

The application looks for specific GIF files in the imported folder:

- First GIF with "move" in its name becomes the movement animation
- First GIF with "wait" in its name becomes the idle animation
- First GIF with "sit" in its name becomes the sitting animation
- First GIF with "pick" in its name becomes the picking up animation
- All other GIFs are categorized as miscellaneous

## Limitations

- GIFs need to have a transparent background to look good
- For best results, all GIFs should be the same size
- "Move" animations should face right when moving right

## Technical Notes

This application uses:
- Windows API for window management
- GDI+ for GIF loading and rendering
- Windows Shell APIs for folder selection 
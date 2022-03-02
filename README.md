# tedit
tedit is a simple text editor created in win32

## Design
* It's look and feel is mostly the same as notepad
* Files are opened through menu or by simply dragging a file into the window
* Automatically detects if the opened file is binary or UTF-8
* Supports setting any font which is installed in your system

**To do**
* Add support for conversion from Windows CRLF to UNIX LF style.

As of now, if you open a UNIX type file, it will show all the lines on the same line in tedit.
This is because Windows and UNIX uses two different ways to denote a newline(tedit only supports the Windows version)
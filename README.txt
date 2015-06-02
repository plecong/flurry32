Flurry32 - Win32 Port of Flurry
-----------------------------------------------------------------------
This is a port of Calum Robinson's Flurry Screen Saver to Win32. The goal is to provide a Windows version of this screensaver for those of us without a Mac.


REQUIREMENTS
-----------------------------------------------------------------------
I haven't really tested this on a wide variety of platforms.  Try it and if it appears jittery, then you probably don't have what it takes.  It also require OpenGL, and I think this comes with most Windows installation lately, but I'm not too sure about Windows 95.

See http://www.microsoft.com/ for more details about OpenGL on Windows.


INSTALLATION
-----------------------------------------------------------------------
Copy the Flurry32.scr file into the correct directory according to your Windows OS version:

Windows 95:   C:\Windows
Windows 98:   C:\Windows\System or C:\Windows 
Windows 2000: C:\WINNT\System32 
Windows XP:   C:\Windows or C:\Windows\System32 


UNINSTALLATION
-----------------------------------------------------------------------
Delete the Flurry32.scr file.


REFERENCES
-----------------------------------------------------------------------
The original screensaver on Mac OS X is by Calum Robinson:
	http://homepage.mac.com/calumr/
The port of flurry to GNU/Linux xscreensaver is by Tobias Sargeant and can be found:
	http://www.jwz.org/xscreensaver/
OpenGL Win32 tutorial (it's good to be listed high in Google):
	http://www.nullterminator.net/opengl32.html
How to Scr: Writing an OpenGL Screensaver for Windows:
	http://www.cityintherain.com/howtoscr.html
Microsoft Developer Network:
	http://msdn.microsoft.com/default.aspx
	

RELEASE HISTORY
-----------------------------------------------------------------------
v0.1 - Initial Release based on xscreensaver code.  No preferences window so no changing of preferences.  Lotsa intertwined code between screensaver stuff and OpenGL stuff.
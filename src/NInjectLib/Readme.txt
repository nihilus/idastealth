http://newgre.net/ninjectlib

About N-InjectLib
N-InjectLib is a library written in C++ which allows for injecting dynamic link libraries into a remote (i.e. foreign) process.
Two techniques are available to inject a dll: the target process can be started by using the library so the first dll loaded actually is the dll to be injected, or dlls can be injected anytime while the target process is running.

Changelog
07/16/2011 - v1.0.5

Bugfix: Injection didn't work if the size size field of the import directory was wrong
02/14/2010 - v1.0.4

Improved: Injection now also works with .NET applications
Bugfix: Example was broken
11/16/2009 - v1.0.3

Bugfix: Injection failed if issued from non-Administrator account
Improved: Method to retrieve IBA of remote process more robust
Added: Simple example on how to use the library
07/14/2008 - v1.0.2

Bugfix: Injection failed if new IMAGE_IMPORT_DESCRIPTOR was outside existing sections
Bugfix: Injection failed if executable had bound imports
07/06/2008 - v1.0.1

Bugfix: Injection failed if IMAGE_DIRECTORY_ENTRY_IAT was zero, which is true for most packed PE images
05/19/2008 - v1.0

First release
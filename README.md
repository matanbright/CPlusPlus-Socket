# CPlusPlus-Socket

This is a UNIX socket class for C++.
It enables you to create and manage a UNIX socket in an object-oriented approach, as there is in most of the high-level programming languages.
It invokes the appropriate UNIX system calls behind the scenes when you call the object's methods, and it adds support for exception handling.

Disclaimer: Currently it only supports the INET and INET6 address-families, and the TCP and UDP protocols.

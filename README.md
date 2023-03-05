# tex-to-tls

A small, quick program to quickly export texture files to an appropriate Ninja TexList in the form of a .tls file! Made primarily for Enhanced Chao World but fully applicable for any other mod for Sonic Adventure DX and 2, and maybe other projects!

Drag and drop a texture file (.pak, .prs, .gvm, .pvmx, .xvm) onto the executable and select an export mode (exporting with names is rarely needed, so unless you *need* that select option 0), a .tls file will be created alongside the texture file which you can now import and #include into your project! Casting to void* is required unless you edit your njdef.h 'TEXN()' define to cast the string there.

.tls files were actually used by the devs! But this is only my best guess as to how they were actually formatted. I normally try to fully recreate how it was originally done before using it myself and usually long before I publish it publicly, but this is already a very useful tool on it's own, accurate or not!

Feel free to fork and make changes, I'd love to see some of the logic being done less crudely, but it works for now!

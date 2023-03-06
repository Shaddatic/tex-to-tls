# tex-to-tls

A small, non-flashy tool to quickly export texture files (.pak, .prs, .pvm, .gvm, .pvmx, .xvm) to an appropriate Ninja TexList in the form of a .tls file! Made primarily for Enhanced Chao World but fully applicable for any other mod for Sonic Adventure DX and 2, and maybe other projects!

Drag and drop a texture file onto the executable and select an export mode (Without names is recommended unless you *need* them), a .tls file will be created alongside the texture file which you can now import and #include into your project! Casting to void* is required unless you edit your njdef.h 'TEXN(NAME)' define to cast the string there.

Feel free to fork and make changes, I'd love to see some of the logic being done less crudely, but it works for now!

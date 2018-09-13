# carveJpg

Simple app for Linux searching for jpg files in selected file

As parameter it takes input filename (it can be device file for example of disk).

Founded images are saved in current directory.

It is hepfull to recover images from damaged file systems.

In case of damaged media 'ddrescue' program can be used.

I think this way to recover files it's called "file carving".

Application reads byte-by-byte input file looking for three characters with witch every picture starts and uses libjpeg to try read image.

The files that are only partially read by the library are also saved.

From experience, I concludes that some popular programs for similar purposes decorate less than in a simple way possible.

Implementation very suboptimal but short.

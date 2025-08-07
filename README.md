# HomeBrewBrowser
HBB is a completely homemade browser written in C based on all available online documentation.

# Building
HBB comes with a linux64 build in the builds folder, but if you want to build it for yourself, you will need libsdl2-dev, ggc, libgtk-4-dev, make, and build-essential.
example install command for debian-based distros like ubuntu (package names may vary):
```
sudo apt update
sudo apt install build-essential libgtk-4-dev libcurl4-openssl-dev gcc
```
Then, simply run:
```
make
```
in the cloned directory, and the build will appear in the builds folder.

# Current featureset
As of I0.0.1, the program has been rewritten in GTK (good to early on) due to its better system integration and ease of use. It now can render full webpages in raw.

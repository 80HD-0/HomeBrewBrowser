# HomeBrewBrowser
HBB is a completely homemade browser written in C based on all available online documentation.

# Building
HBB comes with a linux64 build in the builds folder, but if you want to build it for yourself, you will need libsdl2-dev, ggc, libsdl2-ttf-dev, libcurl4-openssl-dev, make, and build-essential.
example install command for debian-based distros like ubuntu (package names may vary):
```
sudo apt update
sudo apt install build-essential libsdl2-dev libsdl2-ttf-dev libcurl4-openssl-dev gcc
```
Then, simply run:
```
make
```
in the cloned directory, and the build will appear in the builds folder.

# Current featureset
As of I0.0.0, there is a url entry system that submits to a variable and shows a preview of that variable. That's it. I am implementing a content rendering system that will bring about I0.0.1.

# Pogotrack GUI

This repository hosts a minimalistic QT6 GUI for use in combination with [pogotrack](https://github.com/keivan-amini/pogotrack) and the [pogobot robots](https://github.com/nekonaute/pogobot).

The aim is to help experimentalists quickly identify the correct parameters for the tracking of pogobots robots.

# Install

Download directly the binary for your distribution in Releases.

# Requirements
## Ubuntu 22.04
```
sudo apt install libopencv-dev qt6-base-dev
```

## Ubuntu 24.04
```
sudo apt install libopencv-dev qt6-base-dev libglx-dev libgl1-mesa-dev
```


## MacOs 15+
(_not tested_)
```
brew install opencv
brew install qt
```

# Build requirements

* CMake >= 3.10
* QT >= 6.10
* OpenCv >= 4.12.0

# i3-pager

A fork of [i3-pager](https://github.com/duvholt/i3-pager) to remove the workspace ID from the panel and set workspaces' icons to the largest displayed container.

![image](https://user-images.githubusercontent.com/1637715/72685652-676b4e80-3aec-11ea-8173-4c20daaa584e.png)

## Installation

+ Install system dependencies (for Debian based distros):

```sh
sudo apt install -y libsigc++-2.0-dev libjsoncpp-dev extra-cmake-modules qml-module-qt-labs-qmlmodels make cmake libkf5plasma-dev qtbase5-dev qtdeclarative5-dev
```

+ Build and install i3-pager:

  ```bash
  git clone https://github.com/MaxIsMyName/i3-pager.git --recurse-submodules
  cd i3-pager
  cmake -DCMAKE_INSTALL_PREFIX=$(kf5-config --prefix) -DCMAKE_BUILD_TYPE=Release -B build
  make -j $(nproc) -C build
  sudo make -C build install
  ```

+ Add i3-pager plasmoid to either Plasma dock or Latte-dock.

## Features

+ Shows current mode
+ Shows workspaces
  + Allows filtering by the current screen
  + Show workspace icons using `Font Awesome`
  + (Fork) Display name not number
  + (Fork) For each workspace, add an icon for its largest container.


## Configuration

### Multiple screens

If you have multiple screens you need to either put i3-pager on both or you need to disable `Filter workspaces by current screen` in the plasmoid setting. Otherwise you are only going to see the workspaces on the current screen.

### Workspace name and icon

This fork was created as I wanted per screen workspace management (eg: mod+1 should take me to the first workspace on my current screen). To do this, I use a script which manipulates the workspace ID, and as a result I did not want to display the ID in the panel. 

I also wanted to have an icon in the panel for the largest container on any workspace, rather than renaming workspaces manually. I could probably have done it differently but this works for now.

Available icons are listed [here](https://fontawesome.com/icons?d=gallery&s=solid&m=free).

Select an icon and click `Copy Unicode Glyph`.

If you can't see any icons make sure `Font Awesome` is installed as `Font Awesome 5 Free Solid`. Try running `fc-match "Font Awesome 5 Free Solid"` and see if you get a match.

You can choose to hide workspace names in the plasmoid settings.

## Development

Build with debug mode:

```bash
cmake -DCMAKE_INSTALL_PREFIX=$(kf5-config --prefix) -DCMAKE_BUILD_TYPE=Debug -B build
make -C build
sudo make -C build install
```

This will install the plasmoid globally as I'm not aware of a way to access the plasmoid C++ plugin without globally installing it. To see changes restart Plasma:

```
killall plasmashell; kstart5 plasmashell
```

### Plasmoid

The `plasmoid` folder contains the code for the widget and settings.

To quickly prototype changes use plasmoidviewer:

```bash
plasmoidviewer --applet plasmoid
```

For better logging override QT's log format:

```
QT_MESSAGE_PATTERN="[%{type}] (%{function}:%{line}) - %{message}" plasmoidviewer --applet plasmoid
```

To see changes in plasmoidviewer has to be restarted, but there is no need to run make unless there are changes in the C++ plugin.

Simple terminal control for wxWidgets.

Installation
=====
Either copy the **include/wxterminal/terminalCtrl.h** and **src/terminalCtrl.cpp** files into your project, or install the package using CMake as follows:

```
git clone https://github.com/rwypior/wxWidgetsTerminalCtrl.git
cd wxWidgetsTerminalCtrl
mkdir build && cd build
cmake ..
sudo cmake --install .
```

Usage
=====
Once the **wxterminal/terminalCtrl.h** header is included, the widget may be used in the same way as **wxTextCtrl**.

Events
=====
Events emmited by the terminal widget may be handled using the following function signature:

```
void eventTerminalCommand(TerminalCommandEvent& event);
```

The widget provides the following events:

### terminalctrlEVT_COMMAND
The event is emmited when user pressed enter button while inputting a command in the terminal, and the terminal processed it's internal state. This event may be used to process the command by the client software.

### terminalctrlEVT_POST_COMMAND
The event is emitted AFTER the command processing is finished. This event may be used to alter the prompt, like change the prompt's indent, or the prompt character itself.

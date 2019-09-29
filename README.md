# Dark Mode Listener Windows

A small cli tool to listen to dark mode changes on Windows 10.

Serves the same function as [DarkModeListener](https://github.com/LinusU/DarkModeListener) for macOS.

## Installation

Simply open and build this in Visual Studio 2019. Community edition is fine.

## Usage

```sh
$ dark-mode-listener-windows.exe
light
dark
light
...
```

The program will start by printing a line with either `light` or `dark` depending on the current active mode. It will then print a new line whenever the value changes.
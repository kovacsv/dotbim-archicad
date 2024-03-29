# Archicad Dotbim Add-On

[![Build](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml/badge.svg)](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml)

Archicad Add-On to import and export [Dotbim (.bim)](https://dotbim.net) files.

## How to use?

You can download the latest version here:
- [Archicad 27 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimAddOn-27_Release_WIN.zip)
- [Archicad 26 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimAddOn-26_Release_WIN.zip)
- [Archicad 25 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimAddOn-25_Release_WIN.zip)
- [Archicad 27 (macOS)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimAddOn-27_Release_MAC.zip)
- [Archicad 26 (macOS)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimAddOn-26_Release_MAC.zip)
- [Archicad 25 (macOS)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimAddOn-25_Release_MAC.zip)

### Install

You have two options to install the Add-On

#### First option: Load in Add-On Manager

You can open Add-On Manager with the `Options > Add-On Manager` command. Open the "Edit List of Available Add-Ons" panel, click on the "Add..." button, and select the downloaded apx file.

#### Second option: Copy to the Add-Ons folder

Copy the downloaded apx file to the Add-Ons folder of your Archicad installation. Make sure to restart Archicad after the copy. On Windows the Add-Ons folder location is something like this:
```
C:\Program Files\GRAPHISOFT\ARCHICAD\Add-Ons
```

### Use

You can access import from:
- `File > Open > Open...`
- `File > Interoperability > Merge... > Merge from File...`

You can access export from:
- `File > Save As...`
- `File > Interoperability > Export Dotbim File...`

## How to build?

The build environment is set up for Windows development, but the code should compile on Mac as well.

### Prerequisites

You should install some prerequisites to build the Add-On:
- [Visual Studio](https://visualstudio.microsoft.com/downloads):
  - Archicad 24: Visual Studio 2017
  - Archicad 25: Visual Studio 2019
  - Archicad 26: Visual Studio 2019
- [CMake](https://cmake.org) for generating the project file (3.16+)
- [Python](https://www.python.org) for resource compilation (2.7+ or 3.8+)

### Generate the project and build

Run this command from the root of the repository:
```
python Tools\BuildAddOn.py --configFile config.json --acVersion 25 26 27
```

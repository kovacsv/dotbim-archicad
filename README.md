# Archicad Dotbim Add-On

[![Build](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml/badge.svg)](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml)

Archicad Add-On to import and export [Dotbim (.bim)](https://dotbim.net) files.

## How to use?

You can download the latest version here:
- [Archicad 27 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/Dotbim_AC27_Win.apx)
- [Archicad 26 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/Dotbim_AC26_Win.apx)
- [Archicad 25 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/Dotbim_AC25_Win.apx)
- [Archicad 24 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/Dotbim_AC24_Win.apx)

### Install

You have two options to install the Add-On

#### First option: Load in Add-On Manager

You can open Add-On Manager with the `Options > Add-On Manager` command. Open the "Edit List of Available Add-Ons" panel, click on the "Add..." button, and select the downloaded apx file.

#### Second option: Copy to the Add-Ons folder

Copy the downloaded apx file to the Add-Ons folder of your Archicad installation. Make sure to restart Archicad after the copy. On Windows the Add-Ons folder location is something like this:
```
C:\Program Files\GRAPHISOFT\ARCHICAD\Add-Ons`
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
- [Archicad API Development Kit](https://archicadapi.graphisoft.com) for the target version
- [CMake](https://cmake.org) for generating the project file (3.16+)
- [Python](https://www.python.org) for resource compilation (2.7+ or 3.8+)

### Generate the project and build

See the `Tools` folder for some preconfigured build scripts for each version. Running one of these scripts also generates the Visual Studio project, so after the first run you can continue to work in Visual Studio.
- Archicad 24: `Tools/build_ac24.bat`
- Archicad 25: `Tools/build_ac25.bat`
- Archicad 26: `Tools/build_ac26.bat`

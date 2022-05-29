# Archicad Dotbim Exporter

[![Build](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml/badge.svg)](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml)

Archicad Add-On to export the 3D model to [Dotbim (.bim)](https://dotbim.net) format.

## How to use?

You can download the latest version here:
- [Archicad 25 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimExporter_AC25.apx)
- [Archicad 24 (Windows)](https://github.com/kovacsv/dotbim-archicad/releases/latest/download/DotbimExporter_AC24.apx)

### Install

You have two options to install the Add-On

#### First option: Load the Add-On Manager

You can open Add-On Manager with the `Options > Add-On Manager` command. Open the "Edit List of Available Add-Ons" panel, click on the "Add..." button, and select the downloaded apx file.

#### Second option: Copy to the Add-Ons folder

Copy the downloaded apx file to the Add-Ons folder of your Archicad installation. Make sure to restart Archicad after the copy. On Windows the Add-Ons folder location is something like this:
```
C:\Program Files\GRAPHISOFT\ARCHICAD\Add-Ons`
```

### Use

The export functionality is available at two places:
- `File > Save As...`
- `File > Interoperability > Export Dotbim File...`

## How to build?

The build environment is set up for Windows development, but the code should compile on Mac as well.

### Prerequisites

You should install some prerequisites to build the Add-On:
- [Visual Studio](https://visualstudio.microsoft.com/downloads):
  - Archicad 24: Visual Studio 2017.
  - Archicad 25: Visual Studio 2019.
- [Archicad API Development Kit](https://archicadapi.graphisoft.com/) for the target version.
- [CMake](https://cmake.org) for generating the project file (3.16+).
- [Python](https://www.python.org) for resource compilation (2.7+ or 3.8+).

### Generate the project and build

See the `Tools` folder for some preconfigured build scripts for each version. Running one of these scripts also generates the Visual Studio project, so after the first run you can continue to work in Visual Studio.
- Archicad 24: `Tools/build_ac24.bat`.
- Archicad 25: `Tools/build_ac25.bat`.

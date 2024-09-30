# Archicad Dotbim Add-On

[![Build](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml/badge.svg)](https://github.com/kovacsv/dotbim-archicad/actions/workflows/build.yml)

Archicad Add-On to import and export [Dotbim (.bim)](https://dotbim.net) files.

## Add-On Usage

### Install

You can download the Add-On from the [Bimdots](https://bimdots.com/product/dotbim-in-out) store. Please read the [installation guide](https://bimdots.com/help-center/add-on-installation-guide) for more instructions.

### Use

You can access import from:
- `File > Open > Open...`
- `File > Interoperability > Merge... > Merge from File...`

You can access export from:
- `File > Save As...`
- `File > Interoperability > Export Dotbim File...`

## Add-On JSON interface

The Add-On registers commands to the Archicad JSON interface, so you can access export and import via an external application.

Export command example:
```json
{
    "command" : "API.ExecuteAddOnCommand",
    "parameters": {
        "addOnCommandId": {
            "commandNamespace": "Bimdots",
            "commandName": "ExportDotbimFile"
        },
        "addOnCommandParameters": {
            "filePath": "path/to/dotbim/file"
        }
    }
}
```

Import command example:
```json
{
    "command" : "API.ExecuteAddOnCommand",
    "parameters": {
        "addOnCommandId": {
            "commandNamespace": "Bimdots",
            "commandName": "ImportDotbimFile"
        },
        "addOnCommandParameters": {
            "filePath": "path/to/dotbim/file"
        }
    }
}
```

## Add-On Development

The build environment is set up for Windows development, but the code should compile on Mac as well.

### Prerequisites

You should install some prerequisites to build the Add-On:
- [Visual Studio](https://visualstudio.microsoft.com/downloads) (2019+)
- [CMake](https://cmake.org) for generating the project file (3.16+)
- [Python](https://www.python.org) for resource compilation (2.7+ or 3.8+)

### Generate the project and build

Run this command from the root of the repository:
```
python Tools\BuildAddOn.py --configFile config.json --acVersion 26 27 28
```

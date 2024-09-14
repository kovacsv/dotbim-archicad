#include <ACAPinc.h>

#include "APIEnvir.h"

#include "ResourceIds.hpp"

#include "DotbimImporter.hpp"
#include "DotbimExporter.hpp"
#include "DotbimCommands.hpp"
#include "PropertyHandler.hpp"
#include "ApiUtils.hpp"

#include <DGModule.hpp>

static const GSType FileTypeId = 1;

static GSErrCode ExportDotbimFromSaveAs (const API_IOParams* ioParams, Modeler::SightPtr sight)
{
    ModelerAPI::Model model;
    if (GetAPIModel (sight, &model) != NoError) {
        return Error;
    }
    return ExportDotbimFile (model, *ioParams->fileLoc);
}

static GSErrCode ExportDotbimFromMenu ()
{
    API_WindowInfo windowInfo = {};

    if (ACAPI_Window_GetCurrentWindow (&windowInfo) != NoError) {
        return Error;
    }

    if (windowInfo.typeID != APIWind_3DModelID) {
        DG::AlertResponse response = DG::WarningAlert (
            RSGetIndString (ID_ADDON_WARN3D_STRS, ID_ADDON_WARN3D_STR_WARNING_TITLE, ACAPI_GetOwnResModule ()),
            RSGetIndString (ID_ADDON_WARN3D_STRS, ID_ADDON_WARN3D_STR_OPEN_3D_QUESTION, ACAPI_GetOwnResModule ()),
            RSGetIndString (ID_ADDON_WARN3D_STRS, ID_ADDON_WARN3D_STR_OPEN_3D, ACAPI_GetOwnResModule ()),
            RSGetIndString (ID_ADDON_WARN3D_STRS, ID_ADDON_WARN3D_STR_CANCEL, ACAPI_GetOwnResModule ())
        );
        if (response == DG::AlertResponse::Cancel) {
            return NoError;
        }

        API_WindowInfo newWindowInfo = {};
        newWindowInfo.typeID = APIWind_3DModelID;
        if (ACAPI_Window_ChangeWindow (&newWindowInfo) != NoError) {
            return Error;
        }
    }

    DG::FileDialog saveFileDialog (DG::FileDialog::Type::Save);
    GS::UniString fileTypeString = RSGetIndString (ID_ADDON_STRS, ID_ADDON_STR_FORMAT_NAME, ACAPI_GetOwnResModule ());
    FTM::FileTypeManager fileTypeManager ("Dotbim");
    FTM::FileType fileType (fileTypeString.ToCStr (CC_UTF8), "bim", 0, 0, 0);
    FTM::TypeID fileTypeId = fileTypeManager.AddType (fileType);
    saveFileDialog.AddFilter (fileTypeId);

    IO::Location fileLoc = saveFileDialog.GetSelectedFolder ();
    fileLoc.AppendToLocal (IO::Name (GetProjectName ()));
    saveFileDialog.SelectFile (fileLoc);

    if (!saveFileDialog.Invoke ()) {
        return NoError;
    }

    const IO::Location& location = saveFileDialog.GetSelectedFile ();
    return ExportDotbimFileFrom3DWindow (location);
}

static GSErrCode DotbimFileTypeHandler (const API_IOParams* ioParams, Modeler::SightPtr sight)
{
    GSErrCode result = Error;
    if (ioParams->method == IO_OPEN) {
        GS::UniString materialNameTemplate = RSGetIndString (ID_ADDON_STRS, ID_ADDON_STR_MATERIAL_NAME, ACAPI_GetOwnResModule ());
        result = ImportDotbim (*ioParams->fileLoc, materialNameTemplate);
    } else if (ioParams->method == IO_MERGE) {
        GS::UniString commandString = RSGetIndString (ID_ADDON_STRS, ID_ADDON_STR_MERGE_COMMAND_NAME, ACAPI_GetOwnResModule ());
        result = ACAPI_CallUndoableCommand (commandString, [&]() -> GSErrCode {
            GS::UniString materialNameTemplate = RSGetIndString (ID_ADDON_STRS, ID_ADDON_STR_MATERIAL_NAME, ACAPI_GetOwnResModule ());
            return ImportDotbim (*ioParams->fileLoc, materialNameTemplate);
        });
    } else if (ioParams->method == IO_SAVEAS3D) {
        result = ExportDotbimFromSaveAs (ioParams, sight);
    }
    if (result != NoError) {
        if (ioParams->method == IO_OPEN || ioParams->method == IO_MERGE) {
            DG::ErrorAlert (
                RSGetIndString (ID_ADDON_WARN3D_STRS, ID_ADDON_WARN3D_STR_FAILED_TO_IMPORT, ACAPI_GetOwnResModule ()),
                "",
                RSGetIndString (ID_ADDON_WARN3D_STRS, ID_ADDON_WARN3D_STR_OK, ACAPI_GetOwnResModule ())
            );
        }
    }
    return result;
}

static GSErrCode MenuCommandHandler (const API_MenuParams* menuParams)
{
    switch (menuParams->menuItemRef.menuResID) {
        case ID_ADDON_MENU:
            switch (menuParams->menuItemRef.itemIndex) {
                case ID_ADDON_MENU_COMMAND:
                    ExportDotbimFromMenu ();
                    break;
            }
            break;
    }
    return NoError;
}

API_AddonType CheckEnvironment (API_EnvirParams* envir)
{
    RSGetIndString (&envir->addOnInfo.name, ID_ADDON_INFO, 1, ACAPI_GetOwnResModule ());
    RSGetIndString (&envir->addOnInfo.description, ID_ADDON_INFO, 2, ACAPI_GetOwnResModule ());

    return APIAddon_Preload;
}

GSErrCode RegisterInterface (void)
{
    GSErrCode err = NoError;

    err = ACAPI_MenuItem_RegisterMenu (ID_ADDON_MENU, 0, MenuCode_Interoperability, MenuFlag_Default);
    if (err != NoError) {
        return err;
    }

    err = ACAPI_AddOnIntegration_RegisterFileType (
        FileTypeId,
        'DBIM',
        'GSAC',
        "bim",
        ID_ADDON_ICON,
        ID_ADDON_STRS,
        ID_ADDON_STR_FORMAT_NAME,
        Open2DSupported | Merge2DSupported | SaveAs3DSupported
    );
    if (err != NoError) {
        return err;
    }

    return NoError;
}

GSErrCode Initialize (void)
{
    GSErrCode err = NoError;

    err = ACAPI_MenuItem_InstallMenuHandler (ID_ADDON_MENU, MenuCommandHandler);
    if (err != NoError) {
        return err;
    }

    err = ACAPI_AddOnIntegration_InstallFileTypeHandler3D (FileTypeId, DotbimFileTypeHandler);
    if (err != NoError) {
        return err;
    }

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (GS::NewOwned<ExportDotbimFileCommand> ());
    if (err != NoError) {
        return err;
    }

    err = ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler (GS::NewOwned<ImportDotbimFileCommand> ());
    if (err != NoError) {
        return err;
    }

    return NoError;
}

GSErrCode FreeData (void)
{
    return NoError;
}

#include "APIEnvir.h"
#include "ACAPinc.h"

#include "ResourceIds.hpp"
#include "DGModule.hpp"

#include "DotbimImporter.hpp"
#include "DotbimExporter.hpp"
#include "PropertyHandler.hpp"
#include "ApiUtils.hpp"

static const GSResID AddOnInfoID				= ID_ADDON_INFO;
	static const Int32 AddOnNameID				= 1;
	static const Int32 AddOnDescriptionID		= 2;

static const GSResID AddOnMenuID				= ID_ADDON_MENU;
	static const Int32 AddOnCommandID			= 1;

static const GSResID AddOnStrsID				= ID_ADDON_STRS;
	static const Int32 FormatNameID				= 1;
	static const Int32 MergeCommandNameID		= 2;
	static const Int32 MaterialNameID			= 3;

static const GSResID AddOnWarning3DStrsID		= ID_ADDON_WARN3D_STRS;
	static const Int32 Warning3DLargeTextID		= 1;
	static const Int32 Warning3DSmallTextID		= 2;
	static const Int32 Warning3DOpen3DTextID	= 3;
	static const Int32 Warning3DCancelTextID	= 4;
	static const Int32 FailedToImportTextID		= 5;
	static const Int32 OkTextID					= 6;

static const GSType FileTypeId					= 1;

static GSErrCode ExportDotbimFile (const ModelerAPI::Model& model, const IO::Location& location)
{
	IO::File output (location, IO::File::OnNotFound::Create);
	if (output.GetStatus () != NoError) {
		return Error;
	}

	output.Open (IO::File::OpenMode::WriteEmptyMode);
	if (output.GetStatus () != NoError) {
		return Error;
	}

	std::string dotbimContent = ExportDotbim (model);
	output.WriteBin (dotbimContent.c_str (), (USize) dotbimContent.length ());

	output.Close ();
	return NoError;
}

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

	if (ACAPI_Database (APIDb_GetCurrentWindowID, &windowInfo) != NoError) {
		return Error;
	}

	if (windowInfo.typeID != APIWind_3DModelID) {
		DG::AlertResponse response = DG::WarningAlert (
			RSGetIndString (AddOnWarning3DStrsID, Warning3DLargeTextID, ACAPI_GetOwnResModule ()),
			RSGetIndString (AddOnWarning3DStrsID, Warning3DSmallTextID, ACAPI_GetOwnResModule ()),
			RSGetIndString (AddOnWarning3DStrsID, Warning3DOpen3DTextID, ACAPI_GetOwnResModule ()),
			RSGetIndString (AddOnWarning3DStrsID, Warning3DCancelTextID, ACAPI_GetOwnResModule ())
		);
		if (response == DG::AlertResponse::Cancel) {
			return NoError;
		}

		API_WindowInfo newWindowInfo = {};
		newWindowInfo.typeID = APIWind_3DModelID;
		if (ACAPI_Automate (APIDo_ChangeWindowID, &newWindowInfo) != NoError) {
			return Error;
		}
	}

	ModelerAPI::Model model;
	void* currentSight = nullptr;
	if (ACAPI_3D_GetCurrentWindowSight (&currentSight) != NoError) {
		return Error;
	}

	Modeler::SightPtr sightPtr ((Modeler::Sight*) currentSight);
	Modeler::ConstModel3DPtr modelPtr (sightPtr->GetMainModelPtr ());
	if (GetAPIModel (modelPtr, &model) != NoError) {
		return Error;
	}

	DG::FileDialog saveFileDialog (DG::FileDialog::Type::Save);
	GS::UniString fileTypeString = RSGetIndString (AddOnStrsID, FormatNameID, ACAPI_GetOwnResModule ());
	FTM::FileTypeManager fileTypeManager (fileTypeString.ToCStr ());
	FTM::FileType fileType (nullptr, "bim", 0, 0, 0);
	FTM::TypeID fileTypeId = FTM::FileTypeManager::SearchForType (fileType);
	saveFileDialog.AddFilter (fileTypeId);

	IO::Location fileLoc = saveFileDialog.GetSelectedFolder ();
	fileLoc.AppendToLocal (IO::Name (GetProjectName ()));
	saveFileDialog.SelectFile (fileLoc);

	if (!saveFileDialog.Invoke ()) {
		return NoError;
	}

	const IO::Location& location = saveFileDialog.GetSelectedFile ();
	return ExportDotbimFile (model, location);
}

static GSErrCode DotbimFileTypeHandler (const API_IOParams* ioParams, Modeler::SightPtr sight)
{
	GSErrCode result = Error;
	if (ioParams->method == IO_OPEN) {
		GS::UniString materialNameTemplate = RSGetIndString (AddOnStrsID, MaterialNameID, ACAPI_GetOwnResModule ());
		result = ImportDotbim (*ioParams->fileLoc, materialNameTemplate);
	} else if (ioParams->method == IO_MERGE) {
		GS::UniString commandString = RSGetIndString (AddOnStrsID, MergeCommandNameID, ACAPI_GetOwnResModule ());
		result = ACAPI_CallUndoableCommand (commandString, [&] () -> GSErrCode {
			GS::UniString materialNameTemplate = RSGetIndString (AddOnStrsID, MaterialNameID, ACAPI_GetOwnResModule ());
			return ImportDotbim (*ioParams->fileLoc, materialNameTemplate);
		});
	} else if (ioParams->method == IO_SAVEAS3D) {
		result = ExportDotbimFromSaveAs (ioParams, sight);
	}
	if (result != NoError) {
		if (ioParams->method == IO_OPEN || ioParams->method == IO_MERGE) {
			DG::ErrorAlert (
				RSGetIndString (AddOnWarning3DStrsID, FailedToImportTextID, ACAPI_GetOwnResModule ()),
				"",
				RSGetIndString (AddOnWarning3DStrsID, OkTextID, ACAPI_GetOwnResModule ())
			);
		}
	}
	return result;
}

static GSErrCode MenuCommandHandler (const API_MenuParams *menuParams)
{
	switch (menuParams->menuItemRef.menuResID) {
		case AddOnMenuID:
			switch (menuParams->menuItemRef.itemIndex) {
				case AddOnCommandID:
					ExportDotbimFromMenu ();
					break;
			}
			break;
	}
	return NoError;
}

API_AddonType __ACDLL_CALL CheckEnvironment (API_EnvirParams* envir)
{
	RSGetIndString (&envir->addOnInfo.name, AddOnInfoID, AddOnNameID, ACAPI_GetOwnResModule ());
	RSGetIndString (&envir->addOnInfo.description, AddOnInfoID, AddOnDescriptionID, ACAPI_GetOwnResModule ());

	return APIAddon_Normal;
}

GSErrCode __ACDLL_CALL RegisterInterface (void)
{
	GSErrCode err = NoError;

	err = ACAPI_Register_Menu (AddOnMenuID, 0, MenuCode_Interoperability, MenuFlag_Default);
	if (err != NoError) {
		return err;
	}

	err = ACAPI_Register_FileType (
		FileTypeId,
		'DBIM',
		'GSAC',
		"bim",
		ID_ADDON_ICON,
		AddOnStrsID,
		FormatNameID,
		Open2DSupported | Merge2DSupported | SaveAs3DSupported
	);
	if (err != NoError) {
		return err;
	}

	return NoError;
}

GSErrCode __ACENV_CALL Initialize (void)
{
	GSErrCode err = NoError;

	err = ACAPI_Install_MenuHandler (AddOnMenuID, MenuCommandHandler);
	if (err != NoError) {
		return err;
	}

	err = ACAPI_Install_FileTypeHandler3D (FileTypeId, DotbimFileTypeHandler);
	if (err != NoError) {
		return err;
	}

	return NoError;
}

GSErrCode __ACENV_CALL FreeData (void)
{
	return NoError;
}

#include "PropertyHandler.hpp"
#include "ResourceIds.hpp"

#include "ACAPinc.h"

static const GSResID AddOnPropStrsID		= ID_ADDON_PROP_STRS;
	static const Int32 ElementInfoID		= 1;
	static const Int32 RenovationStatusID	= 2;
	static const Int32 ProjectNameID		= 3;
	static const Int32 GeneratedByID		= 4;
	static const Int32 UntitledID			= 5;

static void ProcessProperty (const GS::UniString& name, const GS::UniString& val, const PropertyProcessor& processor)
{
	std::string propertyName = name.ToCStr (0, MaxUSize, CC_UTF8).Get ();
	std::string propertyVal = val.ToCStr (0, MaxUSize, CC_UTF8).Get ();
	processor (propertyName, propertyVal);
}

GS::UniString GetProjectName ()
{
	GS::UniString projectName = RSGetIndString (AddOnPropStrsID, UntitledID, ACAPI_GetOwnResModule ());

	API_ProjectInfo projectInfo = {};
	BNZeroMemory (&projectInfo, sizeof (API_ProjectInfo));

	GSErrCode err = ACAPI_Environment (APIEnv_ProjectID, &projectInfo);
	if (err != NoError || projectInfo.untitled || projectInfo.projectName == nullptr) {
		return projectName;
	}

	return *projectInfo.projectName;
}

void EnumerateProjectProperties (const PropertyProcessor& processor)
{
	GS::UniString projectNameStr = RSGetIndString (AddOnPropStrsID, ProjectNameID, ACAPI_GetOwnResModule ());
	ProcessProperty (projectNameStr, GetProjectName (), processor);

	GS::UniString generatedByStr = RSGetIndString (AddOnPropStrsID, GeneratedByID, ACAPI_GetOwnResModule ());
	ProcessProperty (generatedByStr, "https://github.com/kovacsv/dotbim-archicad", processor);
}

void EnumerateElemProperties (const GS::Guid& elemGuid, const PropertyProcessor& processor)
{
	GSErrCode err = NoError;
	API_Guid apiElemGuid = GSGuid2APIGuid (elemGuid);

	API_ElementMemo	elementMemo;
	err = ACAPI_Element_GetMemo (apiElemGuid, &elementMemo, APIMemoMask_ElemInfoString);
	if (err == NoError && elementMemo.elemInfoString != nullptr) {
		GS::UniString elementIdStr = RSGetIndString (AddOnPropStrsID, ElementInfoID, ACAPI_GetOwnResModule ());
		ProcessProperty (elementIdStr, *elementMemo.elemInfoString, processor);
		ACAPI_DisposeElemMemoHdls (&elementMemo);
	}

	API_Element element;
	BNZeroMemory (&element, sizeof (API_Element));
	element.header.guid = apiElemGuid;
	err = ACAPI_Element_Get (&element);
	if (err == NoError) {
		GS::UniString renovationStatusName;
		err = ACAPI_Goodies (APIAny_GetRenovationStatusNameID, (void*) (GS::IntPtr) element.header.renovationStatus, &renovationStatusName);
		if (err == NoError) {
			GS::UniString renovationStatusStr = RSGetIndString (AddOnPropStrsID, RenovationStatusID, ACAPI_GetOwnResModule ());
			ProcessProperty (renovationStatusStr, renovationStatusName, processor);
		}
	}

#if defined (ServerMainVers_2500)
	GS::Array<API_PropertyDefinition> definitions;
	err = ACAPI_Element_GetPropertyDefinitions (apiElemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err == NoError) {
		for (const API_PropertyDefinition& definition : definitions) {
			API_Property property = {};
			err = ACAPI_Element_GetPropertyValue (apiElemGuid, definition.guid, property);
			if (err != NoError) {
				continue;
			}

			GS::UniString propertyValue;
			err = ACAPI_Property_GetPropertyValueString (property, &propertyValue);
			if (err != NoError) {
				continue;
			}

			API_PropertyGroup group = {};
			group.guid = property.definition.groupGuid;
			err = ACAPI_Property_GetPropertyGroup (group);
			if (err != NoError) {
				continue;
			}

			GS::UniString propertyFullName = property.definition.name + " (" + group.name + ")";
			ProcessProperty (propertyFullName, propertyValue, processor);
		}
	}
#endif
}

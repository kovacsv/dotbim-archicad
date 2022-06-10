#include "PropertyHandler.hpp"
#include "ResourceIds.hpp"

#include "ACAPinc.h"

static const GSResID AddOnPropStrsID	= ID_ADDON_PROP_STRS;
	static const Int32 ElementInfoID	= 1;
	static const Int32 ProjectNameID	= 2;
	static const Int32 GeneratedByID	= 3;
	static const Int32 UntitledID		= 4;

static void ProcessProperty (const GS::UniString& name, const GS::UniString& val, const PropertyProcessor& processor)
{
	std::string propertyName = name.ToCStr ().Get ();
	std::string propertyVal = val.ToCStr ().Get ();
	processor (propertyName, propertyVal);
}

GS::UniString GetProjectName ()
{
	GS::UniString projectName = RSGetIndString (AddOnPropStrsID, UntitledID, ACAPI_GetOwnResModule ());

	API_ProjectInfo projectInfo = {};
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

	// ID
	API_ElementMemo	elementMemo;
	err = ACAPI_Element_GetMemo (apiElemGuid, &elementMemo, APIMemoMask_ElemInfoString);
	if (err == NoError && elementMemo.elemInfoString != nullptr) {
		GS::UniString elementIdStr = RSGetIndString (AddOnPropStrsID, ElementInfoID, ACAPI_GetOwnResModule ());
		ProcessProperty (elementIdStr, *elementMemo.elemInfoString, processor);
		ACAPI_DisposeElemMemoHdls (&elementMemo);
	}

	// Categories
	GS::Array<API_ElemCategory> categoryList;
	err = ACAPI_Database (APIDb_GetElementCategoriesID, &categoryList);
	if (err == NoError) {
		for (const API_ElemCategory& category : categoryList) {
			API_ElemCategoryValue categoryValue;
			err = ACAPI_Element_GetCategoryValue (apiElemGuid, category, &categoryValue);
			if (err != NoError) {
				continue;
			}
			ProcessProperty (category.name, categoryValue.name, processor);
		}
	}

#if defined (ServerMainVers_2500)
	// User-Defined Properties
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

#pragma once

#include <ACAPinc.h>

#include <Sight.hpp>
#include <Model.hpp>
#include <Model3D/Model3D.hpp>

void SetAPIElementType (API_Element& element, API_ElemTypeID elemTypeId);
GS::UniString GetElemTypeName (const API_Elem_Head& elemHead);

GSErrCode GetAPIModel (Modeler::SightPtr sight, ModelerAPI::Model* model);
GSErrCode GetAPIModel (Modeler::ConstModel3DPtr modelPtr, ModelerAPI::Model* model);

#ifndef ServerMainVers_2700

#define ACAPI_MenuItem_RegisterMenu ACAPI_Register_Menu
#define ACAPI_MenuItem_InstallMenuHandler ACAPI_Install_MenuHandler

#define ACAPI_Sight_GetCurrentWindowSight ACAPI_3D_GetCurrentWindowSight

#define ACAPI_AddOnIntegration_RegisterFileType ACAPI_Register_FileType
#define ACAPI_AddOnIntegration_InstallFileTypeHandler3D ACAPI_Install_FileTypeHandler3D

#define ACAPI_AddOnAddOnCommunication_InstallAddOnCommandHandler ACAPI_Install_AddOnCommandHandler

#define ACAPI_Category_GetCategoryValue ACAPI_Element_GetCategoryValue
#define ACAPI_Category_GetCategoryValueDefault ACAPI_Element_GetCategoryValueDefault

#define ACAPI_Element_GetElemTypeName ACAPI_Goodies_GetElemTypeName

GSErrCode ACAPI_ProjectOperation_Project (API_ProjectInfo* projectInfo);

GSErrCode ACAPI_Window_ChangeWindow (API_WindowInfo* windowInfo);
GSErrCode ACAPI_Window_GetCurrentWindow (API_WindowInfo* windowInfo);

GSErrCode ACAPI_Category_GetElementCategories (GS::Array<API_ElemCategory>* categoryList);

GSErrCode ACAPI_HierarchicalEditing_GetHierarchicalElementOwner (API_Guid* elemApiGuid, API_HierarchicalOwnerType* hierarchicalOwnerType, API_HierarchicalElemType* hierarchicalElemType, API_Guid* ownerElemApiGuid);

#endif

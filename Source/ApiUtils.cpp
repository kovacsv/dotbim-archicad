#include "ApiUtils.hpp"

#if defined(ServerMainVers_2600)
#include <IAttributeReader.hpp>
#else
#include <AttributeReader.hpp>
#endif

#include <exp.h>

void SetAPIElementType (API_Element& element, API_ElemTypeID elemTypeId)
{
#ifdef ServerMainVers_2600
    element.header.type = API_ElemType (elemTypeId);
#else
    element.header.typeID = elemTypeId;
#endif
}

GS::UniString GetElemTypeName (const API_Elem_Head& elemHead)
{
    GS::UniString elemTypeName;
#if defined(ServerMainVers_2600)
    ACAPI_Element_GetElemTypeName (elemHead.type, elemTypeName);
#else
    ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) elemHead.typeID, &elemTypeName);
#endif
    return elemTypeName;
}

GSErrCode GetAPIModel (Modeler::SightPtr sight, ModelerAPI::Model* model)
{
#if defined(ServerMainVers_2600)
    GS::Owner<Modeler::IAttributeReader> attributeReader (ACAPI_Attribute_GetCurrentAttributeSetReader ());
    if (EXPGetModel (sight, model, attributeReader.Get ()) != NoError) {
        return Error;
    }
#else
    AttributeReader attributeReader;
    if (EXPGetModel (sight, model, &attributeReader) != NoError) {
        return Error;
    }
#endif
    return NoError;
}

GSErrCode GetAPIModel (Modeler::ConstModel3DPtr modelPtr, ModelerAPI::Model* model)
{
#if defined(ServerMainVers_2600)
    GS::Owner<Modeler::IAttributeReader> attributeReader (ACAPI_Attribute_GetCurrentAttributeSetReader ());
    if (EXPGetModel (modelPtr, model, attributeReader.Get ()) != NoError) {
        return Error;
    }
#else
    AttributeReader attributeReader;
    if (EXPGetModel (modelPtr, model, &attributeReader) != NoError) {
        return Error;
    }
#endif
    return NoError;
}

#ifndef ServerMainVers_2700

GSErrCode ACAPI_ProjectOperation_Project (API_ProjectInfo* projectInfo)
{
    return ACAPI_Environment (APIEnv_ProjectID, projectInfo);
}

GSErrCode ACAPI_Window_ChangeWindow (API_WindowInfo* windowInfo)
{
    return ACAPI_Automate (APIDo_ChangeWindowID, windowInfo);
}

GSErrCode ACAPI_Window_GetCurrentWindow (API_WindowInfo* windowInfo)
{
    return ACAPI_Database (APIDb_GetCurrentWindowID, windowInfo);
}

GSErrCode ACAPI_Category_GetElementCategories (GS::Array<API_ElemCategory>* categoryList)
{
    return ACAPI_Database (APIDb_GetElementCategoriesID, categoryList);
}

GSErrCode ACAPI_HierarchicalEditing_GetHierarchicalElementOwner (API_Guid* elemApiGuid, API_HierarchicalOwnerType* hierarchicalOwnerType, API_HierarchicalElemType* hierarchicalElemType, API_Guid* ownerElemApiGuid)
{
    return ACAPI_Goodies (APIAny_GetHierarchicalElementOwnerID, elemApiGuid, hierarchicalOwnerType, hierarchicalElemType, ownerElemApiGuid);
}

#endif

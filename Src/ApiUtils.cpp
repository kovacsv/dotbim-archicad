#include "ApiUtils.hpp"

#if defined(ServerMainVers_2600)
#include "IAttributeReader.hpp"
#else
#include "AttributeReader.hpp"
#endif

#include "exp.h"

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
	ACAPI_Goodies_GetElemTypeName (elemHead.type, elemTypeName);
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

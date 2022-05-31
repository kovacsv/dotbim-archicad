#include "ModelEnumerator.hpp"

#include "ACAPinc.h"
#include "Algorithms.hpp"

#include "ModelMeshBody.hpp"
#include "Polygon.hpp"
#include "ConvexPolygon.hpp"
#include "ModelMaterial.hpp"

static unsigned char ConvertApiColorComponent (double component)
{
	return (unsigned char) std::round (component * 255.0);
}

Color::Color (unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
	r (r),
	g (g),
	b (b),
	a (a)
{

}

bool Color::operator== (const Color& rhs) const
{
	return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

bool Color::operator!= (const Color& rhs) const
{
	return !operator== (rhs);
}

TriangleEnumerator::TriangleEnumerator ()
{

}

TriangleEnumerator::~TriangleEnumerator ()
{

}

ModelEnumerator::ModelEnumerator (const ModelerAPI::Model& model) :
	model (model),
	topLevelElements (),
	guidToElement (),
	elemHierarchy ()
{
	BuildHierarchy ();
}

USize ModelEnumerator::GetElementCount () const
{
	return topLevelElements.GetSize ();
}

const GS::Guid& ModelEnumerator::GetElementGuid (UIndex index) const
{
	return topLevelElements[index].first;
}

void ModelEnumerator::EnumerateElementGeometry (UIndex index, TriangleEnumerator& enumerator) const
{
	const GS::Guid& elementGuid = GetElementGuid (index);
	if (!guidToElement.ContainsKey (elementGuid)) {
		return;
	}

	Int32 vertexOffset = 0;
	
	const ModelerAPI::Element& element = guidToElement[elementGuid];
	vertexOffset = EnumerateElement (element, vertexOffset, enumerator);
	if (elemHierarchy.ContainsKey (elementGuid)) {
		for (const GS::Guid& subElementGuid : elemHierarchy[elementGuid]) {
			const ModelerAPI::Element& subElement = guidToElement[subElementGuid];
			vertexOffset = EnumerateElement (subElement, vertexOffset, enumerator);
		}
	}
}

void ModelEnumerator::BuildHierarchy ()
{
	for (Int32 elementIndex = 1; elementIndex <= model.GetElementCount (); ++elementIndex) {
		ModelerAPI::Element element;
		model.GetElement (elementIndex, &element);
		if (element.IsInvalid ()) {
			continue;
		}

		GS::Guid elemGuid = element.GetElemGuid ();
		API_Guid elemApiGuid = GSGuid2APIGuid (elemGuid);
		API_HierarchicalOwnerType ownerType = API_RootHierarchicalOwner;
		API_HierarchicalElemType elemType = API_UnknownElemType;
		API_Guid rootApiGuid = APINULLGuid;
		GSErrCode err = ACAPI_Goodies (APIAny_GetHierarchicalElementOwnerID, &elemApiGuid, &ownerType, &elemType, &rootApiGuid);
		if (err != NoError) {
			continue;
		}

		switch (elemType) {
			case API_SingleElem:
			case API_MainElemInMultipleElem:
				{
					topLevelElements.Push (GuidAndType (elemGuid, element.GetType ()));
				}
				break;
			case API_ChildElemInMultipleElem:
				{
					GS::Guid rootGuid = APIGuid2GSGuid (rootApiGuid);
					if (!elemHierarchy.ContainsKey (rootGuid)) {
						elemHierarchy.Add (rootGuid, {});
					}
					elemHierarchy[rootGuid].Push (elemGuid);
				}
				break;
			case API_UnknownElemType:
			default:
				break;
		}

		guidToElement.Add (element.GetElemGuid (), element);
	}

	GS::Sort (topLevelElements.Begin (), topLevelElements.End (), [&] (const GuidAndType& e1, const GuidAndType& e2) {
		return e1.second < e2.second;
	});
}

Int32 ModelEnumerator::EnumerateElement (const ModelerAPI::Element& element, Int32 vertexOffset, TriangleEnumerator& enumerator) const
{
	Int32 currentVertexOffset = vertexOffset;
	for (Int32 bodyIndex = 1; bodyIndex <= element.GetTessellatedBodyCount (); ++bodyIndex) {
		ModelerAPI::MeshBody body;
		element.GetTessellatedBody (bodyIndex, &body);

		for (Int32 vertexIndex = 1; vertexIndex <= body.GetVertexCount (); ++vertexIndex) {
			ModelerAPI::Vertex vertex;
			body.GetVertex (vertexIndex, &vertex, ModelerAPI::CoordinateSystem::World);
			enumerator.OnVertex (vertex);
		}

		for (Int32 polygonIndex = 1; polygonIndex <= body.GetPolygonCount (); ++polygonIndex) {
			ModelerAPI::Polygon polygon;
			body.GetPolygon (polygonIndex, &polygon);
			if (polygon.IsInvisible ()) {
				continue;
			}

			ModelerAPI::Material material;
			polygon.GetMaterial (&material);

			ModelerAPI::Color surfaceColor = material.GetSurfaceColor ();
			Color color (
				ConvertApiColorComponent (surfaceColor.red),
				ConvertApiColorComponent (surfaceColor.green),
				ConvertApiColorComponent (surfaceColor.blue),
				ConvertApiColorComponent (1.0 - material.GetTransparency ())
			);

			for (Int32 convexPolygonIndex = 1; convexPolygonIndex <= polygon.GetConvexPolygonCount (); ++convexPolygonIndex) {
				ModelerAPI::ConvexPolygon convexPolygon;
				polygon.GetConvexPolygon (convexPolygonIndex, &convexPolygon);

				Int32 convexPolygonVertexCount = convexPolygon.GetVertexCount ();
				Int32 triangleCount = convexPolygonVertexCount - 2;
				for (Int32 triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++) {
					Int32 vertexIndex1 = convexPolygon.GetVertexIndex (1);
					Int32 vertexIndex2 = convexPolygon.GetVertexIndex (triangleIndex + 2);
					Int32 vertexIndex3 = convexPolygon.GetVertexIndex (triangleIndex + 3);
					enumerator.OnTriangle (
						color,
						currentVertexOffset + vertexIndex1 - 1,
						currentVertexOffset + vertexIndex2 - 1,
						currentVertexOffset + vertexIndex3 - 1
					);
				}
			}
		}

		currentVertexOffset += body.GetVertexCount ();
	}

	return currentVertexOffset;
}

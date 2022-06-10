#include "DotbimImporter.hpp"
#include "Quaternion.hpp"

#include "ACAPinc.h"

#include "File.hpp"
#include "TRANMAT.h"

#include "rapidjson.h"
#include "document.h"

// Assuming that the transformation doesn't contain scaling
//static void ComposeMatrix (TRANMAT& tranmat, const Vector3D& translation, const Quaternion& rotation)
//{
//	double tx = translation.x;
//	double ty = translation.y;
//	double tz = translation.z;
//	double qx = rotation.qx;
//	double qy = rotation.qy;
//	double qz = rotation.qz;
//	double qw = rotation.qw;
//
//	double x2 = qx + qx;
//	double y2 = qy + qy;
//	double z2 = qz + qz;
//	double xx = qx * x2;
//	double xy = qx * y2;
//	double xz = qx * z2;
//	double yy = qy * y2;
//	double yz = qy * z2;
//	double zz = qz * z2;
//	double wx = qw * x2;
//	double wy = qw * y2;
//	double wz = qw * z2;
//
//	Geometry::Matrix34 matrix (
//		{ (1.0 - (yy + zz)), (xy + wz), (xz - wy) },
//		{ (xy - wz), (1.0 - (xx + zz)), (yz + wx) },
//		{ (xz + wy), (yz - wx), (1.0 - (xx + yy)) },
//		{ tx, ty, tz }
//	);
//
//	tranmat.SetMatrix (matrix);
//}

static void SetAPIElementType (API_Element& element, API_ElemTypeID elemTypeId)
{
#ifdef ServerMainVers_2600
	element.header.type = API_ElemType (elemTypeId);
#else
	element.header.typeID = elemTypeId;
#endif
}

static GSErrCode ImportDotbimElement (
	const rapidjson::Value& meshes,
	const GS::HashTable<Int32, rapidjson::SizeType>& meshIdToIndex,
	const rapidjson::Value& element)
{
	Int32 meshId = element["mesh_id"].GetInt ();
	if (!meshIdToIndex.ContainsKey (meshId)) {
		return Error;
	}

	rapidjson::SizeType meshIndex = meshIdToIndex[meshId];
	const rapidjson::Value& mesh = meshes[meshIndex];

	GSErrCode err = NoError;
	void* bodyData = nullptr;
	err = ACAPI_Body_Create (nullptr, nullptr, &bodyData);
	if (err != NoError) {
		return err;
	}

	GS::Array<UInt32> vertexIndices;
	const rapidjson::Value& coordinates = mesh["coordinates"];
	for (rapidjson::SizeType i = 0; i < coordinates.Size (); i += 3) {
		API_Coord3D position = {
			coordinates[i + 0].GetDouble (),
			coordinates[i + 1].GetDouble (),
			coordinates[i + 2].GetDouble ()
		};
		UInt32 vertexIndex = 0;
		ACAPI_Body_AddVertex (bodyData, position, vertexIndex);
		vertexIndices.Push (vertexIndex);
	}

	const rapidjson::Value& indices = mesh["indices"];
	for (rapidjson::SizeType i = 0; i < indices.Size (); i += 3) {
		UInt32 v0 = vertexIndices[indices[i + 0].GetInt ()];
		UInt32 v1 = vertexIndices[indices[i + 1].GetInt ()];
		UInt32 v2 = vertexIndices[indices[i + 2].GetInt ()];

		Int32 e0 = 0;
		Int32 e1 = 0;
		Int32 e2 = 0;
		ACAPI_Body_AddEdge (bodyData, v0, v1, e0);
		ACAPI_Body_AddEdge (bodyData, v1, v2, e1);
		ACAPI_Body_AddEdge (bodyData, v2, v0, e2);
		
		// TODO: material
		API_OverriddenAttribute material;
		material.attributeIndex = 4;
		material.overridden = true;

		UInt32 triangleIndex = 0;
		ACAPI_Body_AddPolygon (bodyData, { e0, e1, e2 }, 0, material, triangleIndex);
	}

	API_ElementMemo bodyMemo = {};
	ACAPI_Body_Finish (bodyData, &bodyMemo.morphBody, &bodyMemo.morphMaterialMapTable);
	ACAPI_Body_Dispose (&bodyData);

	API_Element morphElement;
	BNZeroMemory (&morphElement, sizeof (API_Element));
	SetAPIElementType (morphElement, API_MorphID);
	err = ACAPI_Element_GetDefaults (&morphElement, nullptr);
	if (err != NoError) {
		return err;
	}

	// TODO: transformation
	err = ACAPI_Element_Create (&morphElement, &bodyMemo);
	if (err != NoError) {
		return err;
	}

	ACAPI_DisposeElemMemoHdls (&bodyMemo);
	return NoError;
}

static GSErrCode ImportDotbimContent (const char* fileContent)
{
	rapidjson::Document document;
	document.Parse (fileContent);
	
	if (!document.HasMember ("meshes") || !document.HasMember ("elements")) {
		return Error;
	}

	GS::HashTable<Int32, rapidjson::SizeType> meshIdToIndex;
	rapidjson::Value meshes = document["meshes"].GetArray ();
	for (rapidjson::SizeType meshIndex = 0; meshIndex < meshes.Size (); ++meshIndex) {
		const rapidjson::Value& mesh = meshes[meshIndex];
		meshIdToIndex.Add (mesh["mesh_id"].GetInt (), meshIndex);
	}

	rapidjson::Value elements = document["elements"].GetArray ();
	for (rapidjson::SizeType elementIndex = 0; elementIndex < elements.Size (); ++elementIndex) {
		const rapidjson::Value& element = elements[elementIndex];
		ImportDotbimElement (meshes, meshIdToIndex, element);
	}

	return NoError;
}

GSErrCode ImportDotbim (const IO::Location& fileLocation)
{
	IO::File file (fileLocation);
	if (file.GetStatus () != NoError) {
		return Error;
	}

	if (file.Open (IO::File::OpenMode::ReadMode) != NoError) {
		return Error;
	}

	UInt64 fileSize = 0;
	if (file.GetDataLength (&fileSize) != NoError) {
		return Error;
	}

	GSErrCode result = NoError;
	char* fileContent = new char[fileSize];
	if (file.ReadBin (fileContent, (USize) fileSize) == NoError) {
		try {
			result = ImportDotbimContent (fileContent);
		} catch (...) {
			result = Error;
		}
	}
	delete[] fileContent;
	file.Close ();

	return result;
}

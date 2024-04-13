#pragma once

#include <Vertex.hpp>
#include <Model.hpp>
#include <ModelElement.hpp>
#include <Transformation.hpp>

using GuidAndType = std::pair<GS::Guid, ModelerAPI::Element::Type>;

class Color
{
public:
    Color (unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    bool operator== (const Color& rhs) const;
    bool operator!= (const Color& rhs) const;

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

class TriangleEnumerator
{
public:
    TriangleEnumerator ();
    virtual ~TriangleEnumerator ();

    virtual void OnVertex (const ModelerAPI::Vertex& vertex) = 0;
    virtual void OnTriangle (const Color& color, Int32 v1, Int32 v2, Int32 v3) = 0;
};

class ModelEnumerator
{
public:
    ModelEnumerator (const ModelerAPI::Model& model);

    USize GetElementCount () const;
    const GS::Guid& GetElementGuid (UIndex index) const;
    bool GetElementBaseElementId (UIndex index, ModelerAPI::BaseElemId& baseElemId) const;
    bool GetElementTransformation (UIndex index, ModelerAPI::Transformation& transformation) const;
    void EnumerateElementGeometry (UIndex index, TriangleEnumerator& enumerator) const;

private:
    void BuildHierarchy ();
    bool IsHierarchicalMainElement (const GS::Guid& elementGuid) const;
    Int32 EnumerateElement (const ModelerAPI::Element& element, ModelerAPI::CoordinateSystem coordSystem, Int32 vertexOffset, TriangleEnumerator& enumerator) const;

    const ModelerAPI::Model& model;
    GS::Array<GuidAndType> topLevelElements;
    GS::HashTable<GS::Guid, ModelerAPI::Element> guidToElement;
    GS::HashTable<GS::Guid, GS::Array<GS::Guid>> elemHierarchy;
};

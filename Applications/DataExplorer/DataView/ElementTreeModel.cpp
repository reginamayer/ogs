/**
 * \file
 * \author Karsten Rink
 * \date   2011-05-10
 * \brief  Implementation of the ElementTreeModel class.
 *
 * \copyright
 * Copyright (c) 2012-2019, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#include "ElementTreeModel.h"

#include "MeshLib/Mesh.h"
#include "MeshLib/Node.h"
#include "MeshLib/MeshInformation.h"
#include "MeshLib/Elements/Element.h"

#include "GeoLib/AABB.h"

#include "MeshLib/Vtk/VtkMappedMeshSource.h"

#include "TreeItem.h"

/**
 * Constructor.
 */
ElementTreeModel::ElementTreeModel( QObject* parent )
    : TreeModel(parent), _mesh_source(nullptr)
{
    QList<QVariant> rootData;
    delete _rootItem;
    rootData << "Name" << "Type" << "" << "";
    _rootItem = new TreeItem(rootData, nullptr);
}

ElementTreeModel::~ElementTreeModel() = default;

void ElementTreeModel::setElement(vtkUnstructuredGridAlgorithm const*const grid, const unsigned elem_index)
{
    beginResetModel();

    _mesh_source = grid;
    this->clearView();

    auto const* const source =
        dynamic_cast<MeshLib::VtkMappedMeshSource const* const>(grid);

    if (!source)
        return;

    const MeshLib::Mesh* mesh = source->GetMesh();
    const MeshLib::Element* elem = mesh->getElement(elem_index);

    QList<QVariant> elemData;
    elemData << "Element " + QString::number(elem_index) << "" << "" << "";
    auto* elemItem = new TreeItem(elemData, _rootItem);
    _rootItem->appendChild(elemItem);

    QList<QVariant> typeData;
    typeData << "Element Type: " << QString::fromStdString(MeshElemType2String(elem->getGeomType()));
    auto* typeItem = new TreeItem(typeData, elemItem);
    elemItem->appendChild(typeItem);

    auto const mat_ids = materialIDs(*mesh);
    QString matIdString = !mat_ids ? QString("not defined") : QString::number((*mat_ids)[elem->getID()]);
    QList<QVariant> materialData;
    materialData << "MaterialID: " << matIdString;
    auto* matItem = new TreeItem(materialData, elemItem);
    elemItem->appendChild(matItem);

    QList<QVariant> volData;
    volData << "Area/Volume: " <<
    QString::number(mesh->getElement(elem_index)->getContent());
    auto* volItem = new TreeItem(volData, elemItem);
    elemItem->appendChild(volItem);

    QList<QVariant> nodeListData;
    nodeListData << "Nodes" << "" << "" << "";
    auto* nodeListItem = new TreeItem(nodeListData, elemItem);
    elemItem->appendChild(nodeListItem);

    //const std::vector<MeshLib::Node*> nodes_vec = grid->getNodes();
    std::size_t nElemNodes = elem->getNumberOfBaseNodes();
    for (std::size_t i = 0; i < nElemNodes; i++)
    {
        const MeshLib::Node* node = elem->getNode(i);
        QList<QVariant> nodeData;
        nodeData << "Node " + QString::number(node->getID())
                 << QString::number((*node)[0], 'f', 6)
                 << QString::number((*node)[1], 'f', 6)
                 << QString::number((*node)[2], 'f', 6);
        auto* nodeItem = new TreeItem(nodeData, nodeListItem);
        nodeListItem->appendChild(nodeItem);
    }
    endResetModel();
}

void ElementTreeModel::clearView()
{
    beginResetModel();
    _rootItem->removeChildren(0, _rootItem->childCount());
    endResetModel();
}

void ElementTreeModel::setMesh(MeshLib::Mesh const& mesh)
{
    beginResetModel();

    this->clearView();

    QList<QVariant> mesh_name;
    mesh_name << "Name:" << QString::fromStdString(mesh.getName()) << "" << "" << "";
    auto* name_item = new TreeItem(mesh_name, _rootItem);
    _rootItem->appendChild(name_item);

    QList<QVariant> nodes_number;
    nodes_number << "#Nodes: " << QString::number(mesh.getNumberOfNodes()) << "" << "";
    auto* nodes_item = new TreeItem(nodes_number, _rootItem);
    _rootItem->appendChild(nodes_item);

    QList<QVariant> elements_number;
    elements_number << "#Elements: " << QString::number(mesh.getNumberOfElements()) << "" << "";
    auto* elements_item = new TreeItem(elements_number, _rootItem);
    _rootItem->appendChild(elements_item);

    const std::array<QString, 7> n_element_names = {{ "Lines:", "Triangles:", "Quads:", "Tetrahedra:", "Hexahedra:", "Pyramids:", "Prisms:" }};
    const std::array<unsigned, 7>& n_element_types (MeshLib::MeshInformation::getNumberOfElementTypes(mesh));
    for (std::size_t i=0; i<n_element_types.size(); ++i)
    {
        if (n_element_types[i])
        {
            QList<QVariant> elements_number;
            elements_number << n_element_names[i] << QString::number(n_element_types[i]) << "" << "";
            auto* type_item = new TreeItem(elements_number, elements_item);
            elements_item->appendChild(type_item);
        }
    }

    QList<QVariant> bounding_box;
    bounding_box << "Bounding Box" << "" << "" << "";
    auto* aabb_item = new TreeItem(bounding_box, _rootItem);
    _rootItem->appendChild(aabb_item);

    const GeoLib::AABB aabb (MeshLib::MeshInformation::getBoundingBox(mesh));
    auto const& min = aabb.getMinPoint();
    auto const& max = aabb.getMaxPoint();

    QList<QVariant> min_aabb;
    min_aabb << "Min:" << QString::number(min[0], 'f') << QString::number(min[1], 'f') << QString::number(min[2], 'f');
    auto* min_item = new TreeItem(min_aabb, aabb_item);
    aabb_item->appendChild(min_item);

    QList<QVariant> max_aabb;
    max_aabb << "Max:" << QString::number(max[0], 'f') << QString::number(max[1], 'f') << QString::number(max[2], 'f');
    auto* max_item = new TreeItem(max_aabb, aabb_item);
    aabb_item->appendChild(max_item);

    QList<QVariant> edges;
    edges << "Edge Length: " << "[" + QString::number(mesh.getMinEdgeLength(), 'f') + "," << QString::number(mesh.getMaxEdgeLength(), 'f') + "]" << "";
    auto* edge_item = new TreeItem(edges, _rootItem);
    _rootItem->appendChild(edge_item);

    std::vector<std::string> const& vec_names (mesh.getProperties().getPropertyVectorNames());
    for (const auto& vec_name : vec_names)
    {
        QList<QVariant> array_info;
        array_info << QString::fromStdString(vec_name) + ": ";
        auto vec_bounds(
            MeshLib::MeshInformation::getValueBounds<int>(mesh, vec_name));
        if (vec_bounds.second != std::numeric_limits<int>::max())
            array_info << "[" + QString::number(vec_bounds.first) + "," << QString::number(vec_bounds.second) + "]" << "";
        else
        {
            auto vec_bounds(MeshLib::MeshInformation::getValueBounds<double>(
                mesh, vec_name));
            if (vec_bounds.second != std::numeric_limits<double>::max())
                array_info  << "[" + QString::number(vec_bounds.first) + "," << QString::number(vec_bounds.second) + "]" << "";
        }
        if (array_info.size() == 1)
            array_info << "[ ?" << "? ]" << "";
        auto* vec_item = new TreeItem(array_info, _rootItem);
        _rootItem->appendChild(vec_item);
    }

    endResetModel();
}

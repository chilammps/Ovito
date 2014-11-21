///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __OVITO_DELAUNAY_TESSELLATION_H
#define __OVITO_DELAUNAY_TESSELLATION_H

#include <plugins/crystalanalysis/CrystalAnalysis.h>
#include <plugins/particles/data/SimulationCellData.h>
#include <plugins/particles/data/ParticleProperty.h>

#ifdef __clang__
	#ifndef CGAL_CFG_ARRAY_MEMBER_INITIALIZATION_BUG
		#define CGAL_CFG_ARRAY_MEMBER_INITIALIZATION_BUG
	#endif
#endif
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_3.h>
#include <CGAL/Triangulation_cell_base_with_info_3.h>

namespace Ovito { namespace Plugins { namespace CrystalAnalysis {

/**
 * Generates a Delaunay tessellation of a particle system.
 */
class DelaunayTessellation
{
public:

	typedef CGAL::Exact_predicates_inexact_constructions_kernel BaseKernel;

	/**
	 * Define an extended Point_3 type for the CGAL library that stores the corresponding atom index in
	 * addition to the xyz coordinates. This extended Point_3 type is used with the CGAL triangulation class.
	 */
	class Point3WithIndex : public BaseKernel::Point_3 {
	private:
		typedef BaseKernel Kernel;
		typedef Kernel::Point_3 Base;
	public:
		// Constructors required by CGAL:
		Point3WithIndex() : Base(), _index(-1), _isGhost(false) {}
		Point3WithIndex(const CGAL::Origin& o) : Base(o), _index(-1), _isGhost(false) {}
		Point3WithIndex(int x, int y, int z) : Base(x, y, z), _index(-1), _isGhost(false) {}
		Point3WithIndex(double x, double y, double z) : Base(x, y, z), _index(-1), _isGhost(false) {}

		// Our added constructors, which take an additional index and a ghost flag.
		Point3WithIndex(const Point3& p, int index, bool isGhost) : Base(p.x(), p.y(), p.z()), _index(index), _isGhost(isGhost) {}
		Point3WithIndex(double x, double y, double z, int index, bool isGhost) : Base(x, y, z), _index(index), _isGhost(isGhost) {}

		bool operator==(const Point3WithIndex& other) const { return x() == other.x() && y() == other.y() && z() == other.z(); }
		bool operator!=(const Point3WithIndex& other) const { return x() != other.x() || y() != other.y() || z() != other.z(); }

		Kernel::Vector_3 operator-(const Point3WithIndex& b) const { return Kernel::Vector_3(x()-b.x(), y()-b.y(), z()-b.z()); }
		operator Ovito::Point3() const { return Ovito::Point3(x(),y(),z()); }
		operator Ovito::Point_3<double>() const { return Ovito::Point_3<double>(x(),y(),z()); }

		/// Returns the original index of this point.
		int index() const { return _index; }

		/// Returns whether this is a ghost point.
		bool isGhost() const { return _isGhost; }

	private:

		/// The original index of this point.
		int _index;

		/// Indicates whether this is a ghost point.
		bool _isGhost;
	};

	/// Data structure attached to each tessellation cell.
	struct CellInfo {
		bool isGhost;	// Indicates whether this is a ghost tetrahedron.
		bool flag;		// An additional flag that can be used by other code.
		int index;		// An index assigned to the cell.
	};

	// A custom CGAL kernel that uses our own Point_3 type.
	// K_ is the new kernel, and K_Base is the old kernel
	template < typename K_, typename K_Base >
	class MyCartesian_base : public K_Base::template Base<K_>::Type
	{
		typedef typename K_Base::template Base<K_>::Type   OldK;
	public:
		typedef K_                                Kernel;
		typedef Point3WithIndex                   Point_3;
		template < typename Kernel2 >
		struct Base { typedef MyCartesian_base<Kernel2, K_Base>  Type; };
	};
	struct MyKernel : public CGAL::Type_equality_wrapper<
	                MyCartesian_base<MyKernel, BaseKernel>,
	                MyKernel>
	{};
	typedef CGAL::Filtered_kernel<MyKernel>							K;
	typedef CGAL::Triangulation_cell_base_with_info_3<CellInfo,K> 	CbDS;
	typedef CGAL::Triangulation_ds_vertex_base_3<> 					VbDS;

	// Define data types for Delaunay triangulation class.
	typedef K 														GT;
	typedef CGAL::Triangulation_data_structure_3<CGAL::Triangulation_vertex_base_3<GT, VbDS>, CbDS> 		TDS;
	typedef CGAL::Delaunay_triangulation_3<GT, TDS>					DT;

	// Often-used iterator types.
	typedef DT::Triangulation_data_structure::Cell_iterator 		CellIterator;
	typedef DT::Triangulation_data_structure::Vertex_iterator 		VertexIterator;

	typedef DT::Triangulation_data_structure::Vertex_handle 		VertexHandle;
	typedef DT::Triangulation_data_structure::Cell_handle 			CellHandle;

	typedef DT::Triangulation_data_structure::Facet_circulator		FacetCirculator;

public:

	/// Generates the Delaunay tessellation.
	void generateTessellation(const SimulationCellData& simCell, const Point3* positions, size_t numPoints, FloatType ghostLayerSize);

	/// Returns the tetrahedron cell on the opposite side of the given cell facet.
	CellHandle mirrorCell(const CellHandle& cell, int facet) const {
		return _dt.tds().mirror_facet(DT::Triangulation_data_structure::Facet(cell, facet)).first;
	}

	/// Returns true if the given vertex is the infinite vertex of the tessellation.
	bool isInfiniteVertex(const VertexHandle& vertex) const {
		return _dt.is_infinite(vertex);
	}

	/// Returns whether the given tessellation cell connects four physical vertices.
	/// Returns false if one of the four vertices is the infinite vertex.
	bool isValidCell(const CellHandle& cell) const {
		for(int v = 0; v < 4; v++)
			if(cell->vertex(v)->point().index() == -1) return false;
		return true;
	}

	/// Returns the total number of tetrahedra in the tessellation.
	DT::size_type number_of_tetrahedra() const { return _dt.number_of_cells(); }

	CellIterator begin_cells() const { return _dt.cells_begin(); }
	CellIterator end_cells() const { return _dt.cells_end(); }
	VertexIterator begin_vertices() const { return _dt.vertices_begin(); }
	VertexIterator end_vertices() const { return _dt.vertices_end(); }

	/// Returns the cell vertex for the given triangle vertex of the given cell facet.
	static inline int cellFacetVertexIndex(int cellFacetIndex, int facetVertexIndex) {
		return CGAL::Triangulation_utils_3::vertex_triple_index(cellFacetIndex, facetVertexIndex);
	}

	FacetCirculator incident_facets(CellHandle cell, int i, int j, CellHandle start, int f) const {
		return _dt.tds().incident_facets(cell, i, j, start, f);
	}

	/// Returns the corresponding facet of a cell as seen from an adjacent cell.
	std::pair<CellHandle,int> mirrorFacet(const FacetCirculator& facet) const {
		return _dt.tds().mirror_facet(*facet);
	}

	/// Returns a reference to the internal CGAL Delaunay triangulation object.
	DT& dt() { return _dt; }

private:

	/// Determines whether the given tetrahedral cell is a ghost cell (or an invalid cell).
	bool isGhostCell(CellHandle cell) const;

	/// The internal CGAL triangulator object.
	DT _dt;
};

}}}	// End of namespace

#endif // __OVITO_DELAUNAY_TESSELLATION_H

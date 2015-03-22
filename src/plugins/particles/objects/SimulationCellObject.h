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

#ifndef __OVITO_SIMULATION_CELL_OBJECT_H
#define __OVITO_SIMULATION_CELL_OBJECT_H

#include <plugins/particles/Particles.h>
#include <core/scene/objects/DataObject.h>
#include <core/gui/properties/PropertiesEditor.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <plugins/particles/data/SimulationCell.h>

namespace Ovito { namespace Particles {

/**
 * \brief Stores the geometry and boundary conditions of a simulation box.
 *
 * The simulation box geometry is a parallelepiped defined by three edge vectors.
 * A fourth vector specifies the origin of the simulation box in space.
 */
class OVITO_PARTICLES_EXPORT SimulationCellObject : public DataObject
{
public:

	/// \brief Constructor. Creates an empty simulation cell.
	Q_INVOKABLE SimulationCellObject(DataSet* dataset) : DataObject(dataset),
		_cellVector1(Vector3::Zero()), _cellVector2(Vector3::Zero()), _cellVector3(Vector3::Zero()),
		_cellOrigin(Point3::Origin()), _pbcX(false), _pbcY(false), _pbcZ(false) {
		init();
	}

	/// \brief Constructs a cell from the given cell data structure.
	explicit SimulationCellObject(DataSet* dataset, const SimulationCell& data) : DataObject(dataset),
			_cellVector1(data.matrix().column(0)), _cellVector2(data.matrix().column(1)), _cellVector3(data.matrix().column(2)),
			_cellOrigin(Point3::Origin() + data.matrix().column(3)), _pbcX(data.pbcFlags()[0]), _pbcY(data.pbcFlags()[1]), _pbcZ(data.pbcFlags()[2]) {
		init();
	}

	/// \brief Constructs a cell from three vectors specifying the cell's edges.
	/// \param a1 The first edge vector.
	/// \param a2 The second edge vector.
	/// \param a3 The third edge vector.
	/// \param origin The origin position.
	SimulationCellObject(DataSet* dataset, const Vector3& a1, const Vector3& a2, const Vector3& a3,
			const Point3& origin = Point3::Origin(), bool pbcX = false, bool pbcY = false, bool pbcZ = false) :
		DataObject(dataset),
		_cellVector1(a1), _cellVector2(a2), _cellVector3(a3),
		_cellOrigin(origin), _pbcX(pbcX), _pbcY(pbcY), _pbcZ(pbcZ) {
		init();
	}

	/// \brief Constructs a cell from a matrix that specifies its shape and position in space.
	/// \param cellMatrix The matrix
	SimulationCellObject(DataSet* dataset, const AffineTransformation& cellMatrix, bool pbcX = false, bool pbcY = false, bool pbcZ = false) :
		DataObject(dataset),
		_cellVector1(cellMatrix.column(0)), _cellVector2(cellMatrix.column(1)), _cellVector3(cellMatrix.column(2)),
		_cellOrigin(Point3::Origin() + cellMatrix.column(3)), _pbcX(pbcX), _pbcY(pbcY), _pbcZ(pbcZ) {
		init();
	}

	/// \brief Constructs a cell with an axis-aligned box shape.
	/// \param box The axis-aligned box.
	/// \param pbcX Specifies whether periodic boundary conditions are enabled in the X direction.
	/// \param pbcY Specifies whether periodic boundary conditions are enabled in the Y direction.
	/// \param pbcZ Specifies whether periodic boundary conditions are enabled in the Z direction.
	SimulationCellObject(DataSet* dataset, const Box3& box, bool pbcX = false, bool pbcY = false, bool pbcZ = false) :
		DataObject(dataset),
		_cellVector1(box.sizeX(), 0, 0), _cellVector2(0, box.sizeY(), 0), _cellVector3(0, 0, box.sizeZ()),
		_cellOrigin(box.minc), _pbcX(pbcX), _pbcY(pbcY), _pbcZ(pbcZ) {
		init();
		OVITO_ASSERT_MSG(box.sizeX() >= 0 && box.sizeY() >= 0 && box.sizeZ() >= 0, "SimulationCellObject constructor", "The simulation box must have a non-negative volume.");
	}

	/// \brief Sets the cell geometry to match the given cell data structure.
	void setData(const SimulationCell& data, bool setPBCFlags = true) {
		_cellVector1 = data.matrix().column(0);
		_cellVector2 = data.matrix().column(1);
		_cellVector3 = data.matrix().column(2);
		_cellOrigin = Point3::Origin() + data.matrix().column(3);
		if(setPBCFlags) {
			_pbcX = data.pbcFlags()[0];
			_pbcY = data.pbcFlags()[1];
			_pbcZ = data.pbcFlags()[2];
		}
	}

	/// \brief Returns a simulation cell data structure that stores the cell's properties.
	SimulationCell data() const {
		SimulationCell data;
		data.setMatrix(cellMatrix());
		data.setPbcFlags(pbcX(), pbcY(), pbcZ());
		return data;
	}

	/// \brief Returns the geometry of the simulation cell as a 3x4 matrix.
	/// \return The shape matrix of the simulation cell.
	///         The first three matrix columns specify the three edge vectors;
	///         the fourth matrix column specifies the translation of the cell origin.
	AffineTransformation cellMatrix() const {
		return {
			_cellVector1.value(),
			_cellVector2.value(),
			_cellVector3.value(),
			_cellOrigin.value() - Point3::Origin()
		};
	}

	/// \brief Changes the cell's shape.
	/// \param shape Specifies the new shape matrix for the simulation cell.
	///              The first three matrix columns contain the three edge vectors.
	///              The fourth matrix column specifies the translation of the cell's origin.
	///
	/// \undoable
	void setCellMatrix(const AffineTransformation& shape) {
		_cellVector1 = shape.column(0);
		_cellVector2 = shape.column(1);
		_cellVector3 = shape.column(2);
		_cellOrigin = Point3::Origin() + shape.column(3);
	}

	/// Returns inverse of the simulation cell matrix.
	/// This matrix maps the simulation cell to the unit cube ([0,1]^3).
	AffineTransformation reciprocalCellMatrix() const {
		return cellMatrix().inverse();
	}

	/// Returns the first cell edge vector.
	const Vector3& edgeVector1() const { return _cellVector1; }

	/// Returns the second cell edge vector.
	const Vector3& edgeVector2() const { return _cellVector2; }

	/// Returns the third cell edge vector.
	const Vector3& edgeVector3() const { return _cellVector3; }

	/// Returns the cell origin.
	const Point3& origin() const { return _cellOrigin; }

	/// Computes the (positive) volume of the cell.
	FloatType volume() const {
		return std::abs(edgeVector1().dot(edgeVector2().cross(edgeVector3())));
	}

	/// \brief Enables or disables periodic boundary conditions in the three spatial directions.
	void setPBCFlags(const std::array<bool,3>& flags) {
		_pbcX = flags[0]; _pbcY = flags[1]; _pbcZ = flags[2];
	}

	/// \brief Returns the periodic boundary flags in all three spatial directions.
	std::array<bool,3> pbcFlags() const {
		return {{ pbcX(), pbcY(), pbcZ() }};
	}

	/// \brief Returns whether periodic boundary conditions are enabled in the X direction.
	bool pbcX() const { return _pbcX; }

	/// \brief Returns whether periodic boundary conditions are enabled in the X direction.
	bool pbcY() const { return _pbcY; }

	/// \brief Returns whether periodic boundary conditions are enabled in the X direction.
	bool pbcZ() const { return _pbcZ; }

	/// \brief Sets periodic boundary flag for the X direction.
	void setPbcX(bool enable) { _pbcX = enable; }

	/// \brief Sets periodic boundary flag for the Y direction.
	void setPbcY(bool enable) { _pbcY = enable; }

	/// \brief Sets periodic boundary flag for the Z direction.
	void setPbcZ(bool enable) { _pbcZ = enable; }

	/// \brief Computes the axis-aligned bounding box of the simulation cell.
	/// \return An axis-aligned box that completely covers the simulation cell.
	Box3 boundingBox() const {
		return Box3(Point3(0), Point3(1)).transformed(cellMatrix());
	}

	/// \brief Returns the title of this object.
	virtual QString objectTitle() override { return tr("Simulation cell"); }

	/// \brief Helper function that computes the modulo operation for two integer numbers k and n.
	///
	/// This function can handle negative numbers k. This allows mapping any number k that is
	/// outside the interval [0,n) back into the interval. Use this to implement periodic boundary conditions.
	static inline int modulo(int k, int n) {
		return SimulationCell::modulo(k,n);
	}

	/// \brief Helper function that computes the modulo operation for two floating-point numbers k and n.
	///
	/// This function can handle negative numbers k. This allows mapping any number k that is
	/// outside the interval [0,n) back into the interval. Use this to implement periodic boundary conditions.
	static inline FloatType modulo(FloatType k, FloatType n) {
		return SimulationCell::modulo(k,n);
	}

protected:

	/// Creates the storage for the internal parameters.
	void init();

	/// Stores the first cell edge.
	PropertyField<Vector3> _cellVector1;
	/// Stores the second cell edge.
	PropertyField<Vector3> _cellVector2;
	/// Stores the third cell edge.
	PropertyField<Vector3> _cellVector3;
	/// Stores the cell origin.
	PropertyField<Point3> _cellOrigin;

	/// Specifies periodic boundary condition in the X direction.
	PropertyField<bool> _pbcX;
	/// Specifies periodic boundary condition in the Y direction.
	PropertyField<bool> _pbcY;
	/// Specifies periodic boundary condition in the Z direction.
	PropertyField<bool> _pbcZ;

private:

	Q_OBJECT
	OVITO_OBJECT

	Q_CLASSINFO("ClassNameAlias", "SimulationCell");	// This for backward compatibility with files written by Ovito 2.4 and older.

	DECLARE_PROPERTY_FIELD(_cellVector1);
	DECLARE_PROPERTY_FIELD(_cellVector2);
	DECLARE_PROPERTY_FIELD(_cellVector3);
	DECLARE_PROPERTY_FIELD(_cellOrigin);
	DECLARE_PROPERTY_FIELD(_pbcX);
	DECLARE_PROPERTY_FIELD(_pbcY);
	DECLARE_PROPERTY_FIELD(_pbcZ);
};

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/**
 * \brief A properties editor for the SimulationCellObject class.
 */
class SimulationCellEditor : public PropertiesEditor
{
public:

	/// Default constructor.
	Q_INVOKABLE SimulationCellEditor() {}

protected:

	/// Creates the user interface controls for the editor.
	virtual void createUI(const RolloutInsertionParameters& rolloutParams) override;

protected Q_SLOTS:

	/// Is called when a spinner's value has changed.
	void onSizeSpinnerValueChanged(int dim);

	/// Is called when the user begins dragging a spinner interactively.
	void onSizeSpinnerDragStart(int dim);

	/// Is called when the user stops dragging a spinner interactively.
	void onSizeSpinnerDragStop(int dim);

	/// Is called when the user aborts dragging a spinner interactively.
	void onSizeSpinnerDragAbort(int dim);

	/// After the simulation cell size has changed, updates the UI controls.
	void updateSimulationBoxSize();

private:

	/// After the user has changed a spinner's value, this method changes the
	/// simulation cell geometry.
	void changeSimulationBoxSize(int dim);

	SpinnerWidget* simCellSizeSpinners[3];

	Q_OBJECT
	OVITO_OBJECT
};

OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace

#endif // __OVITO_SIMULATION_CELL_OBJECT_H

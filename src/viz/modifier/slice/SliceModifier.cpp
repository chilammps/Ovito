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

#include <core/Core.h>
#include <core/viewport/Viewport.h>
#include <core/viewport/ViewportManager.h>
#include <core/dataset/DataSetManager.h>
#include <core/scene/ObjectNode.h>
#include <core/animation/AnimManager.h>
#include <core/animation/controller/StandardControllers.h>
#include <core/scene/pipeline/PipelineObject.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/Vector3ParameterUI.h>
#include <core/gui/properties/BooleanParameterUI.h>
#include "SliceModifier.h"

namespace Viz {

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Viz, SliceModifier, ParticleModifier)
IMPLEMENT_OVITO_OBJECT(Viz, SliceModifierEditor, ParticleModifierEditor)
SET_OVITO_OBJECT_EDITOR(SliceModifier, SliceModifierEditor)
DEFINE_REFERENCE_FIELD(SliceModifier, _normalCtrl, "PlaneNormal", VectorController)
DEFINE_REFERENCE_FIELD(SliceModifier, _distanceCtrl, "PlaneDistance", FloatController)
DEFINE_REFERENCE_FIELD(SliceModifier, _widthCtrl, "SliceWidth", FloatController)
DEFINE_PROPERTY_FIELD(SliceModifier, _createSelection, "CreateSelection")
DEFINE_PROPERTY_FIELD(SliceModifier, _inverse, "Inverse")
DEFINE_PROPERTY_FIELD(SliceModifier, _applyToSelection, "ApplyToSelection")
SET_PROPERTY_FIELD_LABEL(SliceModifier, _normalCtrl, "Normal")
SET_PROPERTY_FIELD_LABEL(SliceModifier, _distanceCtrl, "Distance")
SET_PROPERTY_FIELD_LABEL(SliceModifier, _widthCtrl, "Slice width")
SET_PROPERTY_FIELD_LABEL(SliceModifier, _createSelection, "Select particles (do not delete)")
SET_PROPERTY_FIELD_LABEL(SliceModifier, _inverse, "Invert")
SET_PROPERTY_FIELD_LABEL(SliceModifier, _applyToSelection, "Apply to selected particles only")
SET_PROPERTY_FIELD_UNITS(SliceModifier, _normalCtrl, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SliceModifier, _distanceCtrl, WorldParameterUnit)
SET_PROPERTY_FIELD_UNITS(SliceModifier, _widthCtrl, WorldParameterUnit)

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
SliceModifier::SliceModifier() :
	_createSelection(false),
	_inverse(false),
	_applyToSelection(false)
{
	INIT_PROPERTY_FIELD(SliceModifier::_normalCtrl);
	INIT_PROPERTY_FIELD(SliceModiPutfier::_distanceCtrl);
	INIT_PROPERTY_FIELD(SliceModifier::_widthCtrl);
	INIT_PROPERTY_FIELD(SliceModifier::_createSelection);
	INIT_PROPERTY_FIELD(SliceModifier::_inverse);
	INIT_PROPERTY_FIELD(SliceModifier::_applyToSelection);

	_normalCtrl = ControllerManager::instance().createDefaultController<VectorController>();
	_distanceCtrl = ControllerManager::instance().createDefaultController<FloatController>();
	_widthCtrl = ControllerManager::instance().createDefaultController<FloatController>();
	setNormal(Vector3(1,0,0));
}

/******************************************************************************
* Asks the modifier for its validity interval at the given time.
******************************************************************************/
TimeInterval SliceModifier::modifierValidity(TimePoint time)
{
	// Return an empty validity interval if the modifier is currently being edited
	// to let the system create a pipeline cache point just before the modifier.
	// This will speed up re-evaluation of the pipeline if the user adjusts this modifier's parameters interactively.
	if(isBeingEdited())
		return TimeInterval::empty();

	TimeInterval interval = TimeInterval::forever();
	interval.intersect(_normalCtrl->validityInterval(time));
	interval.intersect(_distanceCtrl->validityInterval(time));
	interval.intersect(_widthCtrl->validityInterval(time));
	return interval;
}

/******************************************************************************
* Returns the slicing plane.
******************************************************************************/
Plane3 SliceModifier::slicingPlane(TimePoint time, TimeInterval& validityInterval)
{
	Plane3 plane;
	_normalCtrl->getValue(time, plane.normal, validityInterval);
	if(plane.normal == Vector3::Zero()) plane.normal = Vector3(0,0,1);
	else plane.normal.normalize();
	_distanceCtrl->getValue(time, plane.dist, validityInterval);
	if(inverse())
		return -plane;
	else
		return plane;
}

/******************************************************************************
* Modifies the particle object.
******************************************************************************/
ObjectStatus SliceModifier::modifyParticles(TimePoint time, TimeInterval& validityInterval)
{
#if 0
	QString statusMessage = tr("Slicing results:\n%n input atoms", 0, input()->atomsCount());

	// Compute filter mask.
	dynamic_bitset<> mask(input()->atomsCount());
	size_t numRejected = filterAtoms(mask, time, validityInterval);
	size_t numKept = input()->atomsCount() - numRejected;

	if(createSelection() == false) {

		statusMessage += tr("\n%n atoms deleted", 0, numRejected);
		statusMessage += tr("\n%n atoms remaining", 0, numKept);
		if(numRejected == 0)
			return EvaluationStatus(EvaluationStatus::EVALUATION_SUCCESS, QString(), statusMessage);

		// Replace referenced data channels with real copies and apply filter.
		output()->deleteAtoms(mask);
		OVITO_ASSERT(output()->atomsCount() == input()->atomsCount() - numRejected);
	}
	else {
		statusMessage += tr("\n%n atoms selected", 0, numRejected);
		statusMessage += tr("\n%n atoms unselected", 0, numKept);

		// Get selection data channel.
		DataChannel* selChannel = outputStandardChannel(DataChannel::SelectionChannel);
		selChannel->setVisible(true);

		int* s = selChannel->dataInt();
		for(size_t i = 0; i < selChannel->size(); i++) {
			*s++ = mask[i];
		}
	}
	return EvaluationStatus(EvaluationStatus::EVALUATION_SUCCESS, QString(), statusMessage);
#endif
	return ObjectStatus();
}

/******************************************************************************
* Performs the actual rejection of particles.
******************************************************************************/
size_t SliceModifier::filterParticles(std::vector<bool>& mask, TimePoint time, TimeInterval& validityInterval)
{
#if 0
	// Get the required data channels.
	DataChannel* posChannel = expectStandardChannel(DataChannel::PositionChannel);
	OVITO_ASSERT(posChannel->size() == mask.size());

	DataChannel* selectionChannel = inputStandardChannel(DataChannel::SelectionChannel);
	if(!applyToSelection()) selectionChannel = NULL;

	FloatType sliceWidth;
	_widthCtrl->getValue(time, sliceWidth, validityInterval);
	sliceWidth /= 2;

	Plane3 plane = slicingPlane(time, validityInterval);

	size_t na = 0;
	const Point3* p = posChannel->constDataPoint3();
	const int* s = NULL;
	if(selectionChannel) s = selectionChannel->constDataInt();

	if(sliceWidth <= 0) {
		for(size_t i = 0; i < posChannel->size(); i++, ++p, ++s) {
			if(plane.pointDistance(*p) > 0) {
				if(selectionChannel && !*s) continue;
				mask.set(i);
				na++;
			}
		}
	}
	else {
		for(size_t i = 0; i < posChannel->size(); i++, ++p, ++s) {
			if(inverse() == (plane.classifyPoint(*p, sliceWidth) == 0)) {
				if(selectionChannel && !*s) continue;
				mask.set(i);
				na++;
			}
		}
	}
	return na;
#endif
	return 0;
}
Put
#if 0
/******************************************************************************
* Makes the modifier render itself into the viewport.
******************************************************************************/
void SliceModifier::renderModifier(TimeTicks time, ObjectNode* contextNode, ModifierApplication* modApp, Viewport* vp)
{
	TimeInterval interval;

	Box3 bb = contextNode->localBoundingBox(time);
	if(bb.isEmpty()) return;

	Plane3 plane = slicingPlane(time, interval);

	FloatType sliceWidth;
	_widthCtrl->getValue(time, sliceWidth, interval);

	if(sliceWidth <= 0) {
		renderPlane(vp, plane, bb, Color(0.8f, 0.3f, 0.3f));
	}
	else {
		plane.dist += sliceWidth / 2;
		renderPlane(vp, plane, bb, Color(0.8f, 0.3f, 0.3f));
		plane.dist -= sliceWidth;
		renderPlane(vp, plane, bb, Color(0.8f, 0.3f, 0.3f));
	}
}

/******************************************************************************
* Renders a plane in the viewport with the given color.
******************************************************************************/
void SliceModifier::renderPlane(Viewport* vp, const Plane3& plane, const Box3& bb, const Color& color) const
{
	// Compute intersection lines of slicing plane and bounding box.
	QVector<Point3> lines;
	Ray3 edges[] = { Ray3(bb[0],bb[1]), Ray3(bb[1],bb[5]), Ray3(bb[5],bb[4]), Ray3(bb[4],bb[0]),
					 Ray3(bb[1],bb[3]), Ray3(bb[0],bb[2]), Ray3(bb[5],bb[7]), Ray3(bb[4],bb[6]),
					 Ray3(bb[2],bb[3]), Ray3(bb[3],bb[7]), Ray3(bb[7],bb[6]), Ray3(bb[6],bb[2]) };

	planeQuadIntersesction(edges[0], edges[1], edges[2], edges[3], plane, lines);
	planeQuadIntersesction(edges[1], edges[4], edges[9], edges[6], plane, lines);
	planeQuadIntersesction(edges[8], edges[9], edges[10], edges[11], plane, lines);
	planeQuadIntersesction(edges[5], edges[3], edges[7], edges[11], plane, lines);
	planeQuadIntersesction(edges[0], edges[4], edges[8], edges[5], plane, lines);
	planeQuadIntersesction(edges[2], edges[6], edges[10], edges[7], plane, lines);

	// If there is not intersection with the simulation box then
	// project the simulation box onto the plane to visualize the plane.
	if(lines.empty()) {
		for(int edge = 0; edge < 12; edge++) {
			lines.push_back(plane.projectPoint(edges[edge].base));
			lines.push_back(plane.projectPoint(edges[edge].base + edges[edge].dir));
		}
	}

	// Render plane-box intersection lines.
	vp->setRenderingColor(color);
	vp->renderLines(lines.size(), bb, &lines.front());
}

/******************************************************************************
* Computes the intersection lines of a plane and a quad.
******************************************************************************/
void SliceModifier::planeQuadIntersesction(const Ray3& r1, const Ray3& r2, const Ray3& r3, const Ray3& r4, const Plane3& plane, QVector<Point3>& lines) const
{
	const Ray3* rays[] = { &r1, &r2, &r3, &r4 };
	Point3 p1, p2;
	bool hasP1 = false;
	FloatType t;
	for(size_t i=0; i<4; i++) {
		t = plane.intersectionT(*rays[i], FLOATTYPE_EPSILON);
		if(t < 0.0 || t > 1.0) continue;
		if(!hasP1) {
			p1 = rays[i]->base + rays[i]->dir * t;
			hasP1 = true;
		}
		else {
			p2 = rays[i]->base + rays[i]->dir * t;
			if(!p2.equals(p1, FLOATTYPE_EPSILON)) {
				lines.push_back(p1);
				lines.push_back(p2);
				return;Put
			}
		}
	}
}
#endif

/******************************************************************************
* This method is called by the system when the modifier has been inserted
* into a PipelineObject.
******************************************************************************/
void SliceModifier::initializeModifier(PipelineObject* pipeline, ModifierApplication* modApp)
{
	ParticleModifier::initializeModifier(pipeline, modApp);

#if 0
	// Get the simulation cell from the input object to center the slicing plane in
	// the center of the simulation cell.
	PipelineFlowState input = modObject->evalObject(ANIM_MANAGER.time(), modApp, false);
	AtomsObject* inputObject = dynamic_object_cast<AtomsObject>(input.result());
	if(inputObject != NULL) {
		Point3 centerPoint = inputObject->simulationCell()->cellMatrix() * Point3(0.5, 0.5, 0.5);
		FloatType centerDistance = DotProduct(normal(), centerPoint - ORIGIN);
		if(fabs(centerDistance) > FLOATTYPE_EPSILON)
			setDistance(centerDistance);
	}
#endif
}


/******************************************************************************
* Sets up the UI widgets of the editor.
******************************************************************************/
void SliceModifierEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create a rollout.
	QWidget* rollout = createRollout(tr("Slicing plane"), rolloutParams);

    // Create the rollout contents.
	QGridLayout* layout = new QGridLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
#ifndef Q_WS_MAC
	layout->setHorizontalSpacing(0);
	layout->setVerticalSpacing(2);
#endif
	layout->setColumnStretch(1, 1);

	// Distance parameter.
	FloatParameterUI* distancePUI = new FloatParameterUI(this, PROPERTY_FIELD(SliceModifier::_distanceCtrl));
	layout->addWidget(distancePUI->label(), 0, 0);
	layout->addLayout(distancePUI->createFieldLayout(), 0, 1);

	// Normal parameter.
	for(int i = 0; i < 3; i++) {
		Vector3ParameterUI* normalPUI = new Vector3ParameterUI(this, PROPERTY_FIELD(SliceModifier::_normalCtrl), i);
		normalPUI->label()->setTextFormat(Qt::RichText);
		normalPUI->label()->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
		normalPUI->label()->setText(tr("<a href=\"%1\">%2</a>").arg(i).arg(normalPUI->label()->text()));
		connect(normalPUI->label(), SIGNAL(linkActivated(const QString&)), this, SLOT(onXYZNormal(const QString&)));
		layout->addWidget(normalPUI->label(), i+1, 0);
		layout->addLayout(normalPUI->createFieldLayout(), i+1, 1);
	}

	// Slice width parameter.
	FloatParameterUI* widthPUI = new FloatParameterUI(this, PROPERTY_FIELD(SliceModifier::_widthCtrl));
	layout->addWidget(widthPUI->label(), 4, 0);
	layout->addLayout(widthPUI->createFieldLayout(), 4, 1);
	widthPUI->setMinValue(0);

	// Invert parameter.
	BooleanParameterUI* invertPUI = new BooleanParameterUI(this, PROPERTY_FIELD(SliceModifier::_inverse));
	layout->addWidget(invertPUI->checkBox(), 5, 0, 1, 2);

	// Create selection parameter.
	BooleanParameterUI* createSelectionPUI = new BooleanParameterUI(this, PROPERTY_FIELD(SliceModifier::_createSelection));
	layout->addWidget(createSelectionPUI->checkBox(), 6, 0, 1, 2);

	// Apply to selection only parameter.
	BooleanParameterUI* applyToSelectionPUI = new BooleanParameterUI(this, PROPERTY_FIELD(SliceModifier::_applyToSelection));
	layout->addWidget(applyToSelectionPUI->checkBox(), 7, 0, 1, 2);

#if 0
	// Add buttons for view alignment functions.
	QPushButton* alignViewToPlaneBtn = new QPushButton(tr("Align view to plane"), rollout);
	connect(alignViewToPlaneBtn, SIGNAL(clicked(bool)), this, SLOT(onAlignViewToPlane()));
	layout->addWidget(alignViewToPlaneBtn, 8, 0, 1, 2);
	QPushButton* alignPlaneToViewBtn = new QPushButton(tr("Align plane to view"), rollout);
	connect(alignPlaneToViewBtn, SIGNAL(clicked(bool)), this, SLOT(onAlignPlaneToView()));
	layout->addWidget(alignPlaneToViewBtn, 9, 0, 1, 2);

	pickAtomPlaneInputMode = new PickAtomPlaneInputMode();
	pickAtomPlaneInputModeAction = new ViewportModeAction("SliceModifier.AlignPlaneToAtoms", pickAtomPlaneInputMode);
	pickAtomPlaneInputModeActionProxy = new ActionProxy(pickAtomPlaneInputModeAction);
	pickAtomPlaneInputModeActionProxy->setParent(this);
	pickAtomPlaneInputModeActionProxy->setText(tr("Align plane to atoms"));
	QWidget* alignPlaneToAtomsBtn = pickAtomPlaneInputModeActionProxy->requestWidget(rollout);
	layout->addWidget(alignPlaneToAtomsBtn, 10, 0, 1, 2);

	QPushButton* centerPlaneBtn = new QPushButton(tr("Center of simulation box"), rollout);
	connect(centerPlaneBtn, SIGNAL(clicked(bool)), this, SLOT(onCenterOfBox()));
	layout->addWidget(centerPlaneBtn, 11, 0, 1, 2);
#endif

	// Status label.
	layout->addWidget(statusLabel(), 12, 0, 1, 2);
}

/******************************************************************************
* Aligns the normal of the slicing plane with the X, Y, or Z axis.
******************************************************************************/
void SliceModifierEditor::onXYZNormal(const QString& link)
{
	SliceModifier* mod = static_object_cast<SliceModifier>(editObject());
	if(!mod) return;

	UndoManager::instance().beginCompoundOperation(tr("Set plane normal"));
	if(link == "0")
		mod->setNormal(Vector3(1,0,0));
	else if(link == "1")
		mod->setNormal(Vector3(0,1,0));
	else if(link == "2")
		mod->setNormal(Vector3(0,0,1));
	UndoManager::instance().endCompoundOperation();
}

/******************************************************************************
* Aligns the slicing plane to the viewing direction.
******************************************************************************/
void SliceModifierEditor::onAlignPlaneToView()
{
#if 0
	TimeInterval interval;

	Viewport* vp = VIEWPORT_MANAGER.activeViewport();
	if(!vp) return;

	// Get the object to world transformation for the currently selected object.
	ObjectNode* node = dynamic_object_cast<ObjectNode>(DATASET_MANAGER.currentSet()->selection()->firstNode());
	if(!node) return;
	const AffineTransformation& nodeTM = node->getWorldTransform(ANIM_MANAGER.time(), interval);
	AffineTransformation localToWorldTM = node->objectTransform() * nodeTM;

	// Get the base point of the current slicing plane in local coordinates.
	SliceModifier* mod = static_object_cast<SliceModifier>(editObject());
	if(!mod) return;
	Plane3 oldPlaneLocal = mod->slicingPlane(ANIM_MANAGER.time(), interval);
	Point3 basePoint = ORIGIN + oldPlaneLocal.normal * oldPlaneLocal.dist;

	// Get the orientation of the projection plane of the current viewport.
	Vector3 dirWorld = Normalize(vp->currentView().inverseViewMatrix * Vector3(0, 0, 1));
	Plane3 newPlaneLocal(basePoint, localToWorldTM.inverse() * dirWorld);
	if(abs(newPlaneLocal.normal.X) < FLOATTYPE_EPSILON) newPlaneLocal.normal.X = 0;
	if(abs(newPlaneLocal.normal.Y) < FLOATTYPE_EPSILON) newPlaneLocal.normal.Y = 0;
	if(abs(newPlaneLocal.normal.Z) < FLOATTYPE_EPSILON) newPlaneLocal.normal.Z = 0;

	UndoManager::instance().beginCompoundOperation(tr("Align plane to view"));
	mod->setNormal(Normalize(newPlaneLocal.normal));
	mod->setDistance(newPlaneLocal.dist);
	UndoManager::instance().endCompoundOperation();
#endif
}

/******************************************************************************
* Moves the plane to the center of the simulation box.
******************************************************************************/
void SliceModifierEditor::onCenterOfBox()
{
#if 0
	SliceModifier* mod = static_object_cast<SliceModifier>(editObject());
	if(!mod) return;

	// Get the simulation cell from the input object to center the slicing plane in
	// the center of the simulation cell.
	PipelineFlowState input = mod->getModifierInput();
	AtomsObject* inputObject = dynamic_object_cast<AtomsObject>(input.result());
	if(inputObject == NULL) return;

	Point3 centerPoint = inputObject->simulationCell()->cellMatrix() * Point3(0.5, 0.5, 0.5);
	FloatType centerDistance = DotProduct(mod->normal(), centerPoint - ORIGIN);

	UndoManager::instance().beginCompoundOperation(tr("Set plane position"));
	mod->setDistance(centerDistance);
	UndoManager::instance().endCompoundOperation();
#endif
}

/******************************************************************************
* Aligns the current viewing direction to the slicing plane.
******************************************************************************/
void SliceModifierEditor::onAlignViewToPlane()
{
#if 0
	TimeInterval interval;

	Viewport* vp = VIEWPORT_MANAGER.activeViewport();
	if(!vp) return;

	// Get the object to world transformation for the currently selected object.
	ObjectNode* node = dynamic_object_cast<ObjectNode>(DATASET_MANAGER.currentSet()->selection()->firstNode());
	if(!node) return;
	const AffineTransformation& nodeTM = node->getWorldTransform(ANIM_MANAGER.time(), interval);
	AffineTransformation localToWorldTM = node->objectTransform() * nodeTM;

	// Transform the current slicing plane to the world coordinate system.
	SliceModifier* mod = static_object_cast<SliceModifier>(editObject());
	if(!mod) return;
	Plane3 planeLocal = mod->slicingPlane(ANIM_MANAGER.time(), interval);
	Plane3 planeWorld = localToWorldTM * planeLocal;

	// Calculate the intersection point of the current viewing direction with the current slicing plane.
	Ray3 viewportRay = vp->viewportRay(Point2(0,0));
	FloatType t = planeWorld.intersectionT(viewportRay);
	Point3 intersectionPoint;
	if(t != FLOATTYPE_MAX)
		intersectionPoint = viewportRay.point(t);
	else
		intersectionPoint = ORIGIN + localToWorldTM.getTranslation();

	if(vp->currentView().isPerspective) {
		FloatType distance = Distance(ORIGIN + vp->currentView().inverseViewMatrix.getTranslation(), intersectionPoint);
		vp->settings()->setViewType(Viewport::VIEW_PERSPECTIVE);
		vp->settings()->setViewMatrix(AffineTransformation::lookAt(intersectionPoint + planeWorld.normal * distance,
			intersectionPoint, Vector3(0,0,1)));
	}
	else {
		vp->settings()->setViewType(Viewport::VIEW_ORTHO);
		vp->settings()->setViewMatrix(AffineTransformation::lookAt(ORIGIN, ORIGIN + (-planeWorld.normal), Vector3(0,0,1))
			* AffineTransformation::translation(Vector3(-intersectionPoint.X, -intersectionPoint.Y, -intersectionPoint.Z)));
	}
	vp->updateViewport(true);
#endif
}

#if 0
/******************************************************************************
* This is called by the system after the input handler has become the active handler.
******************************************************************************/
void PickAtomPlaneInputMode::onActivated()
{
	MAIN_FRAME->statusBar()->showMessage(tr("Select three atoms to define the slicing plane."));
}

/******************************************************************************
* This is called by the system after the input handler is no longer the active handler.
******************************************************************************/
void PickAtomPlaneInputMode::onDeactivated()
{
	pickedAtoms.clear();
	MAIN_FRAME->statusBar()->clearMessage();
}

/******************************************************************************
* Handles the mouse down events for a Viewport.
******************************************************************************/
void PickAtomPlaneInputMode::onMouseDown(Viewport& vp, QMouseEvent* event)
{
	ViewportInputHandler::onMouseDown(vp, event);

	if(event->button() == Qt::LeftButton) {

		if(pickedAtoms.size() >= 3) {
			pickedAtoms.clear();
			VIEWPORT_MANAGER.updateViewports();
		}

		PickAtomResult pickResult;
		if(pickAtom(vp, event->pos(), ANIM_MANAGER.time(), pickResult)) {

			// Do not select the same atom twice.
			if(pickedAtoms.size() >= 1 && pickedAtoms[0].worldPos.equals(pickResult.worldPos, FLOATTYPE_EPSILON)) return;
			if(pickedAtoms.size() >= 2 && pickedAtoms[1].worldPos.equals(pickResult.worldPos, FLOATTYPE_EPSILON)) return;

			pickedAtoms.push_back(pickResult);
			VIEWPORT_MANAGER.updateViewports();

			if(pickedAtoms.size() == 3) {

				// Get the slice modifier that is currently being edited.
				SliceModifier* mod = dynamic_object_cast<SliceModifier>(MAIN_FRAME->commandPanel()->editObject());
				if(mod)
					alignPlane(mod);
			}
		}
	}

}

/******************************************************************************
* Aligns the modifier's slicing plane to the three selected atoms.
******************************************************************************/
void PickAtomPlaneInputMode::alignPlane(SliceModifier* mod)
{
	OVITO_ASSERT(pickedAtoms.size() == 3);

	try {
		Plane3 worldPlane(pickedAtoms[0].worldPos, pickedAtoms[1].worldPos, pickedAtoms[2].worldPos, true);
		if(worldPlane.normal.equals(NULL_VECTOR, FLOATTYPE_EPSILON))
			throw Exception(tr("Cannot determine the new slicing plane. The three selected atoms are colinear."));

		// Get the object to world transformation for the currently selected object.
		ObjectNode* node = dynamic_object_cast<ObjectNode>(DATASET_MANAGER.currentSet()->selection()->firstNode());
		if(!node) return;
		TimeInterval interval;
		const AffineTransformation& nodeTM = node->getWorldTransform(ANIM_MANAGER.time(), interval);
		AffineTransformation localToWorldTM = node->objectTransform() * nodeTM;

		// Transform new plane from world to object space.
		Plane3 localPlane = localToWorldTM.inverse() * worldPlane;

		// Flip new plane orientation if necessary to align it with old orientation.
		if(DotProduct(localPlane.normal, mod->normal()) < 0)
			localPlane = -localPlane;

		localPlane.normalizePlane();
		UNDO_MANAGER.beginCompoundOperation(tr("Align plane to atoms"));
		mod->setNormal(localPlane.normal);
		mod->setDistance(localPlane.dist);
		UNDO_MANAGER.endCompoundOperation();
	}
	catch(const Exception& ex) {
		ex.showError();
	}
}

/******************************************************************************
* Lets the input mode render its overlay content in a viewport.
******************************************************************************/
void PickAtomPlaneInputMode::renderOverlay(Viewport* vp, bool isActive)
{
	ViewportInputHandler::renderOverlay(vp, isActive);

	Q_FOREACH(const PickAtomResult& pa, pickedAtoms) {
		renderSelectionMarker(vp, pa);
	}
}
#endif

};	// End of namespace

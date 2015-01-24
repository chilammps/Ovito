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
#include <core/scene/ObjectNode.h>
#include <core/gui/properties/FloatParameterUI.h>
#include <core/gui/properties/BooleanRadioButtonParameterUI.h>
#include <core/gui/properties/VariantComboBoxParameterUI.h>
#include <core/rendering/RenderSettings.h>
#include <core/rendering/SceneRenderer.h>
#include <core/scene/objects/helpers/TargetObject.h>
#include "CameraObject.h"
#include "moc_AbstractCameraObject.cpp"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(ObjectSystem) OVITO_BEGIN_INLINE_NAMESPACE(Scene) OVITO_BEGIN_INLINE_NAMESPACE(StdObj)

IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, AbstractCameraObject, DataObject);
IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CameraObject, AbstractCameraObject);
SET_OVITO_OBJECT_EDITOR(CameraObject, CameraObjectEditor);
DEFINE_PROPERTY_FIELD(CameraObject, _isPerspective, "IsPerspective");
DEFINE_REFERENCE_FIELD(CameraObject, _fov, "FOV", Controller);
DEFINE_REFERENCE_FIELD(CameraObject, _zoom, "Zoom", Controller);
SET_PROPERTY_FIELD_LABEL(CameraObject, _isPerspective, "Perspective projection");
SET_PROPERTY_FIELD_LABEL(CameraObject, _fov, "FOV angle");
SET_PROPERTY_FIELD_LABEL(CameraObject, _zoom, "FOV size");
SET_PROPERTY_FIELD_UNITS(CameraObject, _fov, AngleParameterUnit);
SET_PROPERTY_FIELD_UNITS(CameraObject, _zoom, WorldParameterUnit);

OVITO_BEGIN_INLINE_NAMESPACE(Internal)
	IMPLEMENT_SERIALIZABLE_OVITO_OBJECT(Core, CameraDisplayObject, DisplayObject);
	IMPLEMENT_OVITO_OBJECT(Core, CameraObjectEditor, PropertiesEditor);
OVITO_END_INLINE_NAMESPACE

/******************************************************************************
* Constructs a camera object.
******************************************************************************/
CameraObject::CameraObject(DataSet* dataset) : AbstractCameraObject(dataset), _isPerspective(true)
{
	INIT_PROPERTY_FIELD(CameraObject::_isPerspective);
	INIT_PROPERTY_FIELD(CameraObject::_fov);
	INIT_PROPERTY_FIELD(CameraObject::_zoom);

	_fov = ControllerManager::instance().createFloatController(dataset);
	_fov->setFloatValue(0, FLOATTYPE_PI/4.0);
	_zoom = ControllerManager::instance().createFloatController(dataset);
	_zoom->setFloatValue(0, 200);

	addDisplayObject(new CameraDisplayObject(dataset));
}

/******************************************************************************
* Asks the object for its validity interval at the given time.
******************************************************************************/
TimeInterval CameraObject::objectValidity(TimePoint time)
{
	TimeInterval interval = DataObject::objectValidity(time);
	if(isPerspective() && _fov) interval.intersect(_fov->validityInterval(time));
	if(!isPerspective() && _zoom) interval.intersect(_zoom->validityInterval(time));
	return interval;
}

/******************************************************************************
* Fills in the missing fields of the camera view descriptor structure.
******************************************************************************/
void CameraObject::projectionParameters(TimePoint time, ViewProjectionParameters& params)
{
	// Transform scene bounding box to camera space.
	Box3 bb = params.boundingBox.transformed(params.viewMatrix).centerScale(1.01f);

	// Compute projection matrix.
	params.isPerspective = isPerspective();
	if(params.isPerspective) {
		if(bb.minc.z() < -FLOATTYPE_EPSILON) {
			params.zfar = -bb.minc.z();
			params.znear = std::max(-bb.maxc.z(), params.zfar * 1e-4f);
		}
		else {
			params.zfar = std::max(params.boundingBox.size().length(), FloatType(1));
			params.znear = params.zfar * 1e-4f;
		}
		params.zfar = std::max(params.zfar, params.znear * 1.01f);

		// Get the camera angle.
		params.fieldOfView = _fov->getFloatValue(time, params.validityInterval);
		if(params.fieldOfView < FLOATTYPE_EPSILON) params.fieldOfView = FLOATTYPE_EPSILON;
		if(params.fieldOfView > FLOATTYPE_PI - FLOATTYPE_EPSILON) params.fieldOfView = FLOATTYPE_PI - FLOATTYPE_EPSILON;

		params.projectionMatrix = Matrix4::perspective(params.fieldOfView, 1.0f / params.aspectRatio, params.znear, params.zfar);
	}
	else {
		if(!bb.isEmpty()) {
			params.znear = -bb.maxc.z();
			params.zfar  = std::max(-bb.minc.z(), params.znear + 1.0f);
		}
		else {
			params.znear = 1;
			params.zfar = 100;
		}

		// Get the camera zoom.
		params.fieldOfView = _zoom->getFloatValue(time, params.validityInterval);
		if(params.fieldOfView < FLOATTYPE_EPSILON) params.fieldOfView = FLOATTYPE_EPSILON;

		params.projectionMatrix = Matrix4::ortho(-params.fieldOfView / params.aspectRatio, params.fieldOfView / params.aspectRatio,
							-params.fieldOfView, params.fieldOfView, params.znear, params.zfar);
	}
	params.inverseProjectionMatrix = params.projectionMatrix.inverse();
}

/******************************************************************************
* Returns the field of view of the camera.
******************************************************************************/
FloatType CameraObject::fieldOfView(TimePoint time, TimeInterval& validityInterval)
{
	if(isPerspective())
		return _fov->getFloatValue(time, validityInterval);
	else
		return _zoom->getFloatValue(time, validityInterval);
}

/******************************************************************************
* Changes the field of view of the camera.
******************************************************************************/
void CameraObject::setFieldOfView(TimePoint time, FloatType newFOV)
{
	if(isPerspective())
		_fov->setFloatValue(time, newFOV);
	else
		_zoom->setFloatValue(time, newFOV);
}

/******************************************************************************
* Returns whether this camera is a target camera directory at a target object.
******************************************************************************/
bool CameraObject::isTargetCamera() const
{
	for(ObjectNode* node : dependentNodes()) {
		if(node->lookatTargetNode() != nullptr)
			return true;
	}
	return false;
}

/******************************************************************************
* Changes the type of the camera to a target camera or a free camera.
******************************************************************************/
void CameraObject::setIsTargetCamera(bool enable)
{
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new TargetChangedUndoOperation(this));
	for(ObjectNode* node : dependentNodes()) {
		if(node->lookatTargetNode() == nullptr && enable) {
			if(SceneNode* parentNode = node->parentNode()) {
				AnimationSuspender noAnim(this);
				OORef<TargetObject> targetObj = new TargetObject(dataset());
				OORef<ObjectNode> targetNode = new ObjectNode(dataset());
				targetNode->setDataProvider(targetObj);
				targetNode->setName(tr("%1.target").arg(node->name()));
				parentNode->addChild(targetNode);
				// Position the new target to match the current orientation of the camera.
				TimeInterval iv;
				const AffineTransformation& cameraTM = node->getWorldTransform(dataset()->animationSettings()->time(), iv);
				Vector3 cameraPos = cameraTM.translation();
				Vector3 cameraDir = cameraTM.column(2).normalized();
				Vector3 targetPos = cameraPos - targetDistance() * cameraDir;
				targetNode->transformationController()->translate(0, targetPos, AffineTransformation::Identity());
				node->setLookatTargetNode(targetNode);
			}
		}
		else if(node->lookatTargetNode() != nullptr && !enable) {
			OORef<SceneNode> targetNode = node->lookatTargetNode();
			node->setLookatTargetNode(nullptr);
			targetNode->deleteNode();
		}
	}
	if(dataset()->undoStack().isRecording())
		dataset()->undoStack().push(new TargetChangedRedoOperation(this));
	notifyDependents(ReferenceEvent::TargetChanged);
}

/******************************************************************************
* With a target camera, indicates the distance between the camera and its target.
******************************************************************************/
FloatType CameraObject::targetDistance() const
{
	for(ObjectNode* node : dependentNodes()) {
		if(node->lookatTargetNode() != nullptr) {
			TimeInterval iv;
			Vector3 cameraPos = node->getWorldTransform(dataset()->animationSettings()->time(), iv).translation();
			Vector3 targetPos = node->lookatTargetNode()->getWorldTransform(dataset()->animationSettings()->time(), iv).translation();
			return (cameraPos - targetPos).length();
		}
	}

	// That's the fixed target distance of a free camera:
	return 50.0f;
}

OVITO_BEGIN_INLINE_NAMESPACE(Internal)

/******************************************************************************
* Constructor that creates the UI controls for the editor.
******************************************************************************/
void CameraObjectEditor::createUI(const RolloutInsertionParameters& rolloutParams)
{
	// Create the rollout.
	QWidget* rollout = createRollout(tr("Camera"), rolloutParams);

	QVBoxLayout* layout = new QVBoxLayout(rollout);
	layout->setContentsMargins(4,4,4,4);
	layout->setSpacing(2);

	QGridLayout* sublayout = new QGridLayout();
	sublayout->setContentsMargins(0,0,0,0);
	sublayout->setColumnStretch(2, 1);
	sublayout->setColumnMinimumWidth(0, 12);
	layout->addLayout(sublayout);

	// Camera projection parameter.
	BooleanRadioButtonParameterUI* isPerspectivePUI = new BooleanRadioButtonParameterUI(this, PROPERTY_FIELD(CameraObject::_isPerspective));
	isPerspectivePUI->buttonTrue()->setText(tr("Perspective camera:"));
	sublayout->addWidget(isPerspectivePUI->buttonTrue(), 0, 0, 1, 3);

	// FOV parameter.
	FloatParameterUI* fovPUI = new FloatParameterUI(this, PROPERTY_FIELD(CameraObject::_fov));
	sublayout->addWidget(fovPUI->label(), 1, 1);
	sublayout->addLayout(fovPUI->createFieldLayout(), 1, 2);
	fovPUI->setMinValue(1e-3f);
	fovPUI->setMaxValue(FLOATTYPE_PI - 1e-2f);

	isPerspectivePUI->buttonFalse()->setText(tr("Orthographic camera:"));
	sublayout->addWidget(isPerspectivePUI->buttonFalse(), 2, 0, 1, 3);

	// Zoom parameter.
	FloatParameterUI* zoomPUI = new FloatParameterUI(this, PROPERTY_FIELD(CameraObject::_zoom));
	sublayout->addWidget(zoomPUI->label(), 3, 1);
	sublayout->addLayout(zoomPUI->createFieldLayout(), 3, 2);
	zoomPUI->setMinValue(0);

	fovPUI->setEnabled(false);
	zoomPUI->setEnabled(false);
	connect(isPerspectivePUI->buttonTrue(), &QRadioButton::toggled, fovPUI, &FloatParameterUI::setEnabled);
	connect(isPerspectivePUI->buttonFalse(), &QRadioButton::toggled, zoomPUI, &FloatParameterUI::setEnabled);

	// Camera type.
	layout->addSpacing(10);
	VariantComboBoxParameterUI* typePUI = new VariantComboBoxParameterUI(this, "isTargetCamera");
	typePUI->comboBox()->addItem(tr("Free camera"), QVariant::fromValue(false));
	typePUI->comboBox()->addItem(tr("Target camera"), QVariant::fromValue(true));
	layout->addWidget(new QLabel(tr("Camera type:")));
	layout->addWidget(typePUI->comboBox());
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 CameraDisplayObject::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	// This is not a physical object. It doesn't have a size.
	return Box3(Point3::Origin(), Point3::Origin());
}

/******************************************************************************
* Computes the view-dependent bounding box of the object.
******************************************************************************/
Box3 CameraDisplayObject::viewDependentBoundingBox(TimePoint time, Viewport* viewport, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState)
{
	TimeInterval iv;
	Point3 cameraPos = Point3::Origin() + contextNode->getWorldTransform(time, iv).translation();
	FloatType size = 1.0f * viewport->nonScalingSize(cameraPos);
	Box3 bbox(Point3::Origin(), size);

	// Add the camera cone to the bounding box.
	if(contextNode->isSelected()) {
		if(CameraObject* camera = dynamic_object_cast<CameraObject>(dataObject)) {
			if(camera->isPerspective()) {
				// Determine the camera and target positions when rendering a target camera.
				FloatType targetDistance;
				if(contextNode->lookatTargetNode()) {
					Vector3 cameraPos = contextNode->getWorldTransform(time, iv).translation();
					Vector3 targetPos = contextNode->lookatTargetNode()->getWorldTransform(time, iv).translation();
					targetDistance = (cameraPos - targetPos).length();
				}
				else targetDistance = camera->targetDistance();

				// Determine the aspect ratio and angle of the camera cone.
				if(RenderSettings* renderSettings = dataset()->renderSettings()) {
					FloatType aspectRatio = renderSettings->outputImageAspectRatio();

					FloatType coneAngle = camera->fieldOfView(time, iv);
					FloatType sizeY = tan(0.5f * coneAngle) * targetDistance;
					FloatType sizeX = sizeY / aspectRatio;
					bbox.addPoint(Point3(sizeX, sizeY, -targetDistance));
					bbox.addPoint(Point3(-sizeX, sizeY, -targetDistance));
					bbox.addPoint(Point3(-sizeX, -sizeY, -targetDistance));
					bbox.addPoint(Point3(sizeX, -sizeY, -targetDistance));
				}
			}
		}
	}

	return bbox;
}

/******************************************************************************
* Lets the display object render a camera object.
******************************************************************************/
void CameraDisplayObject::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	// Camera objects are only visible in the viewports.
	if(renderer->isInteractive() == false || renderer->viewport() == nullptr)
		return;

	TimeInterval iv;

	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_cameraIcon || !_cameraIcon->isValid(renderer)
			|| !_pickingCameraIcon || !_pickingCameraIcon->isValid(renderer);

	// Determine icon color depending on selection state.
	Color color = ViewportSettings::getSettings().viewportColor(contextNode->isSelected() ? ViewportSettings::COLOR_SELECTION : ViewportSettings::COLOR_CAMERAS);

	// Do we have to update contents of the geometry buffers?
	bool updateContents = _geometryCacheHelper.updateState(dataObject, color) || recreateBuffer;

	// Re-create the geometry buffers if necessary.
	if(recreateBuffer) {
		_cameraIcon = renderer->createLinePrimitive();
		_pickingCameraIcon = renderer->createLinePrimitive();
	}

	// Fill geometry buffers.
	if(updateContents) {

		// Initialize lines.
		static std::vector<Point3> iconVertices;
		if(iconVertices.empty()) {
			// Load and parse PLY file that contains the camera icon.
			QFile meshFile(QStringLiteral(":/core/3dicons/camera.ply"));
			meshFile.open(QIODevice::ReadOnly | QIODevice::Text);
			QTextStream stream(&meshFile);
			for(int i = 0; i < 3; i++) stream.readLine();
			int numVertices = stream.readLine().section(' ', 2, 2).toInt();
			OVITO_ASSERT(numVertices > 0);
			for(int i = 0; i < 3; i++) stream.readLine();
			int numFaces = stream.readLine().section(' ', 2, 2).toInt();
			for(int i = 0; i < 2; i++) stream.readLine();
			std::vector<Point3> vertices(numVertices);
			for(int i = 0; i < numVertices; i++)
				stream >> vertices[i].x() >> vertices[i].y() >> vertices[i].z();
			for(int i = 0; i < numFaces; i++) {
				int numEdges, vindex, lastvindex, firstvindex;
				stream >> numEdges;
				for(int j = 0; j < numEdges; j++) {
					stream >> vindex;
					if(j != 0) {
						iconVertices.push_back(vertices[lastvindex]);
						iconVertices.push_back(vertices[vindex]);
					}
					else firstvindex = vindex;
					lastvindex = vindex;
				}
				iconVertices.push_back(vertices[lastvindex]);
				iconVertices.push_back(vertices[firstvindex]);
			}
		}

		_cameraIcon->setVertexCount(iconVertices.size());
		_cameraIcon->setVertexPositions(iconVertices.data());
		_cameraIcon->setLineColor(ColorA(color));

		_pickingCameraIcon->setVertexCount(iconVertices.size(), renderer->defaultLinePickingWidth());
		_pickingCameraIcon->setVertexPositions(iconVertices.data());
		_pickingCameraIcon->setLineColor(ColorA(color));
	}

	// Determine the camera and target positions when rendering a target camera.
	FloatType targetDistance = 0;
	bool showTargetLine = false;
	if(contextNode->lookatTargetNode()) {
		Vector3 cameraPos = contextNode->getWorldTransform(time, iv).translation();
		Vector3 targetPos = contextNode->lookatTargetNode()->getWorldTransform(time, iv).translation();
		targetDistance = (cameraPos - targetPos).length();
		showTargetLine = true;
	}

	// Determine the aspect ratio and angle of the camera cone.
	FloatType aspectRatio = 0;
	FloatType coneAngle = 0;
	if(contextNode->isSelected()) {
		if(RenderSettings* renderSettings = dataset()->renderSettings())
			aspectRatio = renderSettings->outputImageAspectRatio();
		if(CameraObject* camera = dynamic_object_cast<CameraObject>(dataObject)) {
			if(camera->isPerspective()) {
				coneAngle = camera->fieldOfView(time, iv);
				if(targetDistance == 0)
					targetDistance = camera->targetDistance();
			}
		}
	}

	// Do we have to re-create the geometry buffer from scratch?
	recreateBuffer = !_cameraCone || !_cameraCone->isValid(renderer);

	// Do we have to update contents of the geometry buffer?
	color = ViewportSettings::getSettings().viewportColor(ViewportSettings::COLOR_CAMERAS);
	updateContents = _coneCacheHelper.updateState(color, targetDistance, showTargetLine, aspectRatio, coneAngle) || recreateBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_cameraCone = renderer->createLinePrimitive();

	// Fill geometry buffer.
	if(updateContents) {
		std::vector<Point3> targetLineVertices;
		if(targetDistance != 0) {
			if(showTargetLine) {
				targetLineVertices.push_back(Point3::Origin());
				targetLineVertices.push_back(Point3(0,0,-targetDistance));
			}
			if(aspectRatio != 0 && coneAngle != 0) {
				FloatType sizeY = tan(0.5f * coneAngle) * targetDistance;
				FloatType sizeX = sizeY / aspectRatio;
				targetLineVertices.push_back(Point3::Origin());
				targetLineVertices.push_back(Point3(sizeX, sizeY, -targetDistance));
				targetLineVertices.push_back(Point3::Origin());
				targetLineVertices.push_back(Point3(-sizeX, sizeY, -targetDistance));
				targetLineVertices.push_back(Point3::Origin());
				targetLineVertices.push_back(Point3(-sizeX, -sizeY, -targetDistance));
				targetLineVertices.push_back(Point3::Origin());
				targetLineVertices.push_back(Point3(sizeX, -sizeY, -targetDistance));

				targetLineVertices.push_back(Point3(sizeX, sizeY, -targetDistance));
				targetLineVertices.push_back(Point3(-sizeX, sizeY, -targetDistance));
				targetLineVertices.push_back(Point3(-sizeX, sizeY, -targetDistance));
				targetLineVertices.push_back(Point3(-sizeX, -sizeY, -targetDistance));
				targetLineVertices.push_back(Point3(-sizeX, -sizeY, -targetDistance));
				targetLineVertices.push_back(Point3(sizeX, -sizeY, -targetDistance));
				targetLineVertices.push_back(Point3(sizeX, -sizeY, -targetDistance));
				targetLineVertices.push_back(Point3(sizeX, sizeY, -targetDistance));
			}
		}
		_cameraCone->setVertexCount(targetLineVertices.size());
		_cameraCone->setVertexPositions(targetLineVertices.data());
		_cameraCone->setLineColor(ColorA(color));
	}

	if(!renderer->isPicking())
		_cameraCone->render(renderer);

	// Setup transformation matrix to always show the camera icon at the same size.
	Point3 cameraPos = Point3::Origin() + renderer->worldTransform().translation();
	FloatType scaling = 0.3f * renderer->viewport()->nonScalingSize(cameraPos);
	renderer->setWorldTransform(renderer->worldTransform() * AffineTransformation::scaling(scaling));

	renderer->beginPickObject(contextNode);
	if(!renderer->isPicking())
		_cameraIcon->render(renderer);
	else
		_pickingCameraIcon->render(renderer);
	renderer->endPickObject();
}

OVITO_END_INLINE_NAMESPACE

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace

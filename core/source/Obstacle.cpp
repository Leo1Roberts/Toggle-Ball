#include "main.h"
#include "Colors.h"
#include "Model.h"
#include "Obstacle.h"

void AbstractShapeSpec::generateObstacleModel(Model& obstacleModel) const {
	obstacleModel.vertices.clear();
	obstacleModel.indices.clear();

	buildObstacleModel(obstacleModel);

	obstacleModel.sendToGpu();
	// TODO: setupVertexAttribs ?
}

void SegmentSpec::buildObstacleModel(Model& obstacleModel) const {
	// TODO
}

void ArcSpec::buildObstacleModel(Model& obstacleModel) const {
	// TODO
}


void AbstractShapeSpec::generateOutlineModel(Model& outlineModel) const {
	outlineModel.vertices.clear();
	outlineModel.indices.clear();

	buildOutlineModel(outlineModel);

	outlineModel.sendToGpu();
	// TODO: setupVertexAttribs ?
}

void SegmentSpec::buildOutlineModel(Model& outlineModel) const {
	// TODO
}

void ArcSpec::buildOutlineModel(Model& outlineModel) const {
	// TODO
}


void IMotionSpec::generateDomainModel(Model& domainModel, const AbstractShapeSpec& shapeSpec) const {
	domainModel.vertices.clear();
	domainModel.indices.clear();

	buildDomainModel(domainModel, shapeSpec);

	domainModel.sendToGpu();
	// TODO: setupVertexAttribs ?
}

void TogglingPositionSpec::buildDomainModel(Model& domainModel, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void TogglingAngleSpec::buildDomainModel(Model& domainModel, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void SpinningSpec::buildDomainModel(Model& domainModel, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void OscillatingPositionSpec::buildDomainModel(Model& domainModel, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void OscillatingAngleSpec::buildDomainModel(Model& domainModel, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}
#include "main.h"
#include "Colors.h"
#include "Mesh.h"
#include "Obstacle.h"

void AbstractShapeSpec::generateObstacleMesh(Mesh<ObjectVertex>& obstacleMesh) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;

	buildObstacleMesh(vs, is);

	obstacleMesh.setData(vs, is);
}

void SegmentSpec::buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	// TODO
}

void ArcSpec::buildObstacleMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	// TODO
}


void AbstractShapeSpec::generateOutlineMesh(Mesh<ObjectVertex>& outlineMesh) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;

	buildOutlineMesh(vs, is);

	outlineMesh.setData(vs, is);
}

void SegmentSpec::buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	// TODO
}

void ArcSpec::buildOutlineMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is) const {
	// TODO
}


void IMotionSpec::generateDomainMesh(Mesh<ObjectVertex>& domainMesh, const AbstractShapeSpec& shapeSpec) const {
	std::vector<ObjectVertex> vs;
	std::vector<Index> is;

	buildDomainMesh(vs, is, shapeSpec);

	domainMesh.setData(vs, is);
}

void TogglingPositionSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void TogglingAngleSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void SpinningSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void OscillatingPositionSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void OscillatingAngleSpec::buildDomainMesh(std::vector<ObjectVertex>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}
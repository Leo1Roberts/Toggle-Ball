#include "main.h"
#include "Colors.h"
#include "Mesh.h"
#include "Obstacle.h"

void AbstractShapeSpec::generateObstacleMesh(Mesh<Vertex3D>& obstacleMesh) const {
	std::vector<Vertex3D> vs;
	std::vector<Index> is;

	buildObstacleMesh(vs, is);

	obstacleMesh.setData(vs, is);
}

void SegmentSpec::buildObstacleMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is) const {
	// TODO
}

void ArcSpec::buildObstacleMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is) const {
	// TODO
}


void AbstractShapeSpec::generateOutlineMesh(Mesh<Vertex3D>& outlineMesh) const {
	std::vector<Vertex3D> vs;
	std::vector<Index> is;

	buildOutlineMesh(vs, is);

	outlineMesh.setData(vs, is);
}

void SegmentSpec::buildOutlineMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is) const {
	// TODO
}

void ArcSpec::buildOutlineMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is) const {
	// TODO
}


void IMotionSpec::generateDomainMesh(Mesh<Vertex3D>& domainMesh, const AbstractShapeSpec& shapeSpec) const {
	std::vector<Vertex3D> vs;
	std::vector<Index> is;

	buildDomainMesh(vs, is, shapeSpec);

	domainMesh.setData(vs, is);
}

void TogglingPositionSpec::buildDomainMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void TogglingAngleSpec::buildDomainMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void SpinningSpec::buildDomainMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void OscillatingPositionSpec::buildDomainMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}

void OscillatingAngleSpec::buildDomainMesh(std::vector<Vertex3D>& vs, std::vector<Index>& is, const AbstractShapeSpec& shapeSpec) const {
	// TODO
}
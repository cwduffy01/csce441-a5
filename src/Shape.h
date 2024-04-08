#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <string>
#include <vector>
#include <memory>

class Program;

/**
 * A shape defined by a list of triangles
 * - posBuf should be of length 3*ntris
 * - norBuf should be of length 3*ntris (if normals are available)
 * - texBuf should be of length 2*ntris (if texture coords are available)
 * posBufID, norBufID, and texBufID are OpenGL buffer identifiers.
 */
class Shape
{
public:
	Shape();
	virtual ~Shape();
	void loadMesh(const std::string& meshName);
	void loadPoints(std::vector<float> posBuf, std::vector<float> norBuf, std::vector<float> texBuf, std::vector<unsigned int> indBuf);
	void fitToUnitBox();
	void init();
	void draw(const std::shared_ptr<Program> prog) const;
	void drawPoints(const std::shared_ptr<Program> prog) const;

	void setPosBuf(std::vector<float>& pBuf);
	void setNorBuf(std::vector<float>& nBuf);
	void setTexBuf(std::vector<float>& tBuf);
	void setIndBuf(std::vector<unsigned int>& iBuf);

	float miny;
	
private:
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	std::vector<unsigned int> indBuf;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
	unsigned indBufID;
};

#endif

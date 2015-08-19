#pragma once
class GraphPoint {
public:
	GraphPoint(unsigned int triangles, bool indexed, double time);
//private:
	unsigned int m_triangles;
	bool m_indexed;
	double m_time;
};
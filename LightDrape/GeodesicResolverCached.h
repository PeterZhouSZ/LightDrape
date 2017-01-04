#pragma once
#include "geodesicresolver.h"
class GeodesicResolverCached :
	public GeodesicResolver
{
public:
	GeodesicResolverCached(void);
	~GeodesicResolverCached(void);

	DoubleProperty_ resolveGeo(WatertightMesh_ mesh){
		if(hasGeoCached(mesh)){
			PRINTLN("Use cached geo.");
			return readCached(mesh, getGeoFileName(mesh));
		}
		else{
			DoubleProperty_ ret = GeodesicResolver::resolveGeo(mesh);
			cachedGeo(ret, mesh->n_vertices(), getGeoFileName(mesh));
			PRINTLN("Cached geo success.");
			return ret;
		}
	}
private:
	std::string getGeoFileName(WatertightMesh_ mesh){
		std::string name = mesh->getName();
		std::string path = "..\\data\\geo_cache\\";
		return path + name + ".geo";
	}
	bool hasGeoCached(WatertightMesh_ mesh){
		std::string name = getGeoFileName(mesh);
		FILE* file = fopen(name.c_str(), "r");
		if(file != nullptr){
			fclose(file);
			return true;
		}
		else{
			return false;
		}
	}
	void cachedGeo(DoubleProperty_ pro, size_t vertexCount, std::string file){
		std::ofstream out(file);
		out << vertexCount << "\n";
		for(size_t i = 0; i < vertexCount; i++){
			out << pro->get(i) << "\n";
		}
		out.close();
	}

	DoubleProperty_ readCached(Mesh_ mesh, std::string file){
		DoubleProperty_ ret = smartNew(DoubleProperty);
		mesh->registerProperty(ret);
		std::ifstream in(file);
		int count;
		double dis;
		in >> count;
		for(int i = 0; i < count; i++){
			in >> dis;
			ret->set(i, dis);
		}
		return ret;
	}
};
S_PTR(GeodesicResolverCached);
#ifndef BVOX_LOADER_H
#define BVOX_LOADER_H


class MortonOctree;
typedef core::SharedPtr<MortonOctree> MortonOctreePtr;

class Logger;

class BVoxLoader
{
public:
	BVoxLoader(MortonOctreePtr octree, Logger * log);
	virtual ~BVoxLoader();

	void ReadFile(const std::string & fileName);
	void WriteFile(const std::string & fileName);

protected:
        MortonOctreePtr m_octree;
        Logger * m_log;
        uint32_t Depth; /// just until we get rid of templated octree.
};

typedef core::SharedPtr<BVoxLoader> BVoxLoaderPtr;

#endif
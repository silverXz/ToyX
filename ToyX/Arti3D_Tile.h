#ifndef _ARTI3D_TILE_H_
#define _ARTI3D_TILE_H_

#include <atomic>
#include <vector>

#include "Arti3D_Types.h"

class Arti3DTile
{
protected:
	friend class Arti3DDevice;
	friend class Arti3DThread;

	Arti3DTile();
	~Arti3DTile();


	Arti3DResult Create();
	
	// All Triangles Intersecting This Tile Has Been Rasterized!
	// This Tile Is Ready For Fragment Processing, Set The Flag.
	void SetReadyForFragmentProcessing();
	
	// Clear All Information Produced In The Last Frame.
	// Must Be Called After Or Before Rendering A Frame.
	void Clear();


private:
	int	m_iWidth;	// Width Of This Tile
	int 	m_iHeight;	// Height Of This Tile
	int	m_iX		// Top Left X Coordinate Of This Tile.
	int 	m_iY;		// Top Left Y Coordinate Of This Tile.

	// Face ID Buffer For Every Working Thread. The Buffer Size = threadNum * maxFaceNumPerTile.
	uint32_t		**m_ppFaceIndexBuffer;
	// Sizes of Face ID Buffer For Every Working Thread.
	uint32_t		*m_pIndexBufferSize;
	// Tile Coverage Type For Very Face In The Face ID Buffer. Its Size = threadNum * maxFaceNumPerTile.
	Arti3DTileCoverage	**m_ppTileCoverage;

	// Buffer Of Gegenerated Fragments From The Last Stage(Rasterization).
	std::vector<Arti3DFragment> m_vFragments;
	
	// Atomic Flag For Whether This Tile Has Been Added To The Job Queue.
	// May Be Read/Writen Simutaneously By More Than One Threads, So Its Operation Has To Be Atomic.
	std::atomic_flag	m_bAddedToJobQueue;
	
	// Whether All Rasterization Work Has Been Done.
	bool			m_bFinishedRasterization;
};


#endif

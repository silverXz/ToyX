#ifndef _ARTI3D_TILE_H_
#define _ARTI3D_TILE_H_

#include <atomic>
#include <vector>

#include "Arti3D_Types.h"

enum Arti3DFragmentCoverage
{
	ARTI3D_FC_TILE = 0,	// Fragment Size = Tile.
	ARTI3D_FC_BLOCK,	// Fragment Size = Block.
	ARTI3D_FC_MASKED,	// Fragment Size = 4 pixels.
};

// There are 3 kinds of size of fragment.
// #1: The size of a tile.
// #2: The size of a block.
// #3: The size of 4 pixels.
struct Arti3DFragment
{
	int x, y;		// Top Left Coordinates Of This Fragment.
	int mask;		// Coverage Mask, Only Valid When "coverType" = ARTI3D_MASKED.
	int faceID;
	int threadID;
	Arti3DFragmentCoverage	coverType;
};

class Arti3DTile
{
protected:
	friend class Arti3DDevice;
	friend class Arti3DThread;

	Arti3DTile();
	~Arti3DTile();

	// Could Only Be Created By Arti3DDevice;
	// @param i_iX : Top Left X Coordinate.
	// @param i_iY : Top Left Y Coordinate.
	// @param i_iWidth : Width Of This Tile.
	// @param i_iHeight : Height Of This Tile.
	Arti3DResult Create(uint32_t i_iX,uint32_t i_iY,uint32_t i_iWidth,uint32_t i_iHeight);
	
	// All Triangles Intersecting This Tile Has Been Rasterized!
	// This Tile Is Ready For Fragment Processing, Set The Flag.
	void SetReadyForFragmentProcessing();
	
	// Clear All Information Produced In The Last Frame.
	// Must Be Called After Or Before Rendering A Frame.
	void Clear();


private:
	int	m_iWidth;	// Width Of This Tile
	int m_iHeight;	// Height Of This Tile
	int	m_iX;		// Top Left X Coordinate Of This Tile.
	int m_iY;		// Top Left Y Coordinate Of This Tile.

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

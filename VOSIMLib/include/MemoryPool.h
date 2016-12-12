/*
Copyright 2016, Austen Satterlee

This file is part of VOSIMProject.

VOSIMProject is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

VOSIMProject is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with VOSIMProject. If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *  \file MemoryPool.h
 *  \brief
 *  \details
 *  \author Austen Satterlee
 *  \date 06/2016
 */

#ifndef __MEMORYPOOL__
#define __MEMORYPOOL__

#include "common.h"
#include <vector>

class VOSIMLIB_API MemoryPool
{
	struct Chunk
	{
		size_t size;
		uint8_t* addr;
		bool isFree;
		Chunk* next;
		Chunk* prev;

		~Chunk();
	};

	struct ChunkSortBySize
	{
		bool operator ()(const Chunk& a_lhs, const Chunk& a_rhs) const;
	};

	struct ChunkSortByAddr
	{
		bool operator ()(const Chunk& a_lhs, const Chunk& a_rhs) const;
	};

	std::vector<uint8_t> m_bytes;
	std::vector<Chunk*> m_freeChunks;
	std::vector<Chunk*> m_usedChunks;
	Chunk m_head;
	size_t m_bytesUsed;
public:
	MemoryPool(size_t a_numBytes);

	virtual ~MemoryPool();

	void* allocate(size_t a_numBytes);

	void free(void* a_ptr);
private:
	void _joinFreeChunks(Chunk* a_chunk);
};

#endif

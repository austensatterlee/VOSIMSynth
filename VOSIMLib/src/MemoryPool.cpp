#include "vosimlib/MemoryPool.h"

syn::MemoryPool::Chunk::~Chunk() {
    if (next)
        next->prev = prev;
    if (prev)
        prev->next = next;
}

bool syn::MemoryPool::ChunkSortBySize::operator()(const Chunk& a_lhs, const Chunk& a_rhs) const {
    return a_lhs.size < a_rhs.size;
}

bool syn::MemoryPool::ChunkSortByAddr::operator()(const Chunk& a_lhs, const Chunk& a_rhs) const {
    return a_lhs.addr < a_rhs.addr;
}

syn::MemoryPool::MemoryPool(size_t a_numBytes):
    m_head{0, nullptr, false, nullptr, nullptr},
    m_bytesUsed(0)
{
    m_bytes.resize(a_numBytes);
    Chunk* freechunk = new Chunk{a_numBytes, &m_bytes[0], true, nullptr, &m_head};
    m_head.next = freechunk;
    m_freeChunks.push_back(freechunk);
}

syn::MemoryPool::~MemoryPool() {
    Chunk* tmp = m_head.next;
    while (tmp) {
        delete tmp;
        tmp = m_head.next;
    }
}

void* syn::MemoryPool::allocate(size_t a_numBytes) {
    // Find best free chunk
    int minSizeDiff = -1;
    Chunk* bestChunk = nullptr;
    for (Chunk* chunk : m_freeChunks) {
        int size_dist = static_cast<int>(chunk->size) - static_cast<int>(a_numBytes);
        if (size_dist < 0)
            continue;
        if (minSizeDiff == -1 || size_dist < minSizeDiff) {
            minSizeDiff = size_dist;
            bestChunk = chunk;
        }
    }
    if (!bestChunk)
        throw std::runtime_error("Insufficient memory in pool");

    m_bytesUsed += a_numBytes;
    // Create new used chunk
    Chunk* usedChunk = new Chunk;
    usedChunk->size = a_numBytes;
    usedChunk->addr = bestChunk->addr;
    usedChunk->isFree = false;
    usedChunk->next = bestChunk;
    usedChunk->prev = bestChunk->prev;
    bestChunk->prev->next = usedChunk;
    bestChunk->prev = usedChunk;
    m_usedChunks.push_back(usedChunk);

    // Update free chunk
    if (minSizeDiff > 0) {
        bestChunk->addr = bestChunk->addr + a_numBytes;
        bestChunk->size = bestChunk->size - a_numBytes;
    } else {
        m_freeChunks.erase(find(m_freeChunks.begin(), m_freeChunks.end(), bestChunk));
        delete bestChunk;
    }

    return usedChunk->addr;
}

void syn::MemoryPool::free(void* a_ptr) {
    size_t numUsedChunks = m_usedChunks.size();
    for (int i = 0; i < numUsedChunks; i++) {
        Chunk* chunk = m_usedChunks[i];
        if (chunk->addr == a_ptr) {
            m_usedChunks.erase(m_usedChunks.begin() + i);
            chunk->isFree = true;
            m_freeChunks.push_back(chunk);
            m_bytesUsed -= chunk->size;
            _joinFreeChunks(chunk);
            break;
        }
    }
}

void syn::MemoryPool::_joinFreeChunks(Chunk* a_chunk) {
    if (!a_chunk->isFree)
        return;
    while (a_chunk->prev && a_chunk->prev->isFree) {
        a_chunk->prev->size += a_chunk->size;
        a_chunk = a_chunk->prev;
        m_freeChunks.erase(find(m_freeChunks.begin(), m_freeChunks.end(), a_chunk->next));
        delete a_chunk->next;
    }
    while (a_chunk->next && a_chunk->next->isFree) {
        a_chunk->size += a_chunk->next->size;
        m_freeChunks.erase(find(m_freeChunks.begin(), m_freeChunks.end(), a_chunk->next));
        delete a_chunk->next;
    }
}

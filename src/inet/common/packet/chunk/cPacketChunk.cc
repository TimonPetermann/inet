//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/chunk/cPacketChunk.h"
#include "inet/common/packet/chunk/SliceChunk.h"

namespace inet {

cPacketChunk::cPacketChunk(cPacket *packet) :
    Chunk(),
    packet(packet)
{
    take(packet);
}

cPacketChunk::cPacketChunk(const cPacketChunk& other) :
    Chunk(other),
    packet(other.packet->dup())
{
    take(packet);
}

cPacketChunk::~cPacketChunk()
{
    dropAndDelete(packet);
}

std::shared_ptr<Chunk> cPacketChunk::peekUnchecked(PeekPredicate predicate, PeekConverter converter, const Iterator& iterator, bit length, int flags) const
{
    bit chunkLength = getChunkLength();
    assert(bit(0) <= iterator.getPosition() && iterator.getPosition() <= chunkLength);
    // 1. peeking an empty part returns nullptr
    if (length == bit(0) || (iterator.getPosition() == chunkLength && length == bit(-1))) {
        if (predicate == nullptr || predicate(nullptr))
            return nullptr;
    }
    // 2. peeking the whole part returns this chunk
    if (iterator.getPosition() == bit(0) && (length == bit(-1) || length == chunkLength)) {
        auto result = const_cast<cPacketChunk *>(this)->shared_from_this();
        if (predicate == nullptr || predicate(result))
            return result;
    }
    // 3. peeking without conversion returns a SliceChunk
    if (converter == nullptr)
        return peekConverted<SliceChunk>(iterator, length, flags);
    // 4. peeking with conversion
    return converter(const_cast<cPacketChunk *>(this)->shared_from_this(), iterator, length, flags);
}

std::string cPacketChunk::str() const {
    std::ostringstream os;
    os << "cPacketChunk, packet = {" << ( packet != nullptr ? packet->str() : std::string("<null>")) << "}";
    return os.str();
}

} // namespace

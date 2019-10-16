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

#ifndef __INET_TIMETAG_H
#define __INET_TIMETAG_H

#include "inet/common/packet/Packet.h"
#include "inet/common/TimeTag_m.h"

namespace inet {

template <typename T>
void increaseTimeTag(const Ptr<Chunk>& data, simtime_t duration)
{
    data->mapAllTags<T>(b(0), b(-1), [&] (b offset, b length, T *timeTag) {
        for (int i = 0; i < (int)timeTag->getTimesArraySize(); i++)
            timeTag->setTimes(i, timeTag->getTimes(i) + duration);
    });
}

} // namespace inet

#endif // ifndef __INET_TIMETAG_H


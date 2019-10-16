//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_TIMINGMEASUREMENTSTARTER_H
#define __INET_TIMINGMEASUREMENTSTARTER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API TimingMeasurementStarter : public PacketFlowBase
{
  protected:
    PacketFilter packetFilter;
    b offset = b(0);
    b length = b(-1);
    const char *label = nullptr;
    bool measureElapsedTime = false;
    bool measureDelayingTime = false;
    bool measureQueueingTime = false;
    bool measureProcessingTime = false;
    bool measureTransmissionTime = false;
    bool measurePropagationTime = false;

  protected:
    virtual void initialize(int stage) override;

    template <typename T>
    void startMeasurement(Packet *packet, const Ptr<Chunk>& data, b offset, b length, const char *label, simtime_t value) {
        if (length == b(-1))
            length = data->getChunkLength();
        EV_INFO << "Starting measurement on packet " << packet->getName() << ": "
                << "range (" << offset << ", " << offset + length << "), ";
        if (label != nullptr && *label != '\0')
            EV_INFO << "label = " << label << ", ";
        EV_INFO << "class = " << T::getClassName() << std::endl;
        // KLUDGE: horrible
        auto packetProtocolTag = packet->findTag<PacketProtocolTag>();
        auto timeTag = packetProtocolTag != nullptr ? data->addTagIfAbsent<T>(offset + B(42), length - B(46)) : data->addTagIfAbsent<T>(offset, length);
        timeTag->insertLabels(label);
        timeTag->insertTimes(value);
    }

  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_TIMINGMEASUREMENTSTARTER_H


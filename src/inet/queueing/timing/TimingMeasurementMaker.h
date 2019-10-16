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

#ifndef __INET_TIMINGMEASUREMENTMAKER_H
#define __INET_TIMINGMEASUREMENTMAKER_H

#include "inet/common/packet/PacketFilter.h"
#include "inet/common/TimeTag_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API TimingMeasurementMaker : public PacketFlowBase
{
  public:
    static simsignal_t lifeTimeSignal;
    static simsignal_t elapsedTimeSignal;
    static simsignal_t delayingTimeSignal;
    static simsignal_t queueingTimeSignal;
    static simsignal_t processingTimeSignal;
    static simsignal_t transmissionTimeSignal;
    static simsignal_t propagationTimeSignal;

  protected:
    PacketFilter packetFilter;
    b offset = b(0);
    b length = b(-1);
    bool finish = false;
    const char* label = nullptr;
    cMatchExpression labelMatcher;
    bool measureLifeTime = false;
    bool measureElapsedTime = false;
    bool measureDelayingTime = false;
    bool measureQueueingTime = false;
    bool measureProcessingTime = false;
    bool measureTransmissionTime = false;
    bool measurePropagationTime = false;

  protected:
    virtual void initialize(int stage) override;
    virtual void makeMeasurement(Packet *packet, b offset, b length, const char *label, simsignal_t signal, simtime_t value);

    template <typename T>
    void makeMeasurement(Packet *packet, const Ptr<const Chunk>& data, b offset, b length, simsignal_t signal) {
        data->mapAllTags<T>(offset, length, [&] (b offset, b length, const T *timeTag) {
            for (int i = 0; i < (int)timeTag->getTimesArraySize(); i++) {
                auto label = timeTag->getLabels(i);
                cMatchableString matchableLabel(label);
                if (labelMatcher.matches(&matchableLabel)) {
                    cNamedObject signalDetails(label);
                    makeMeasurement(packet, offset, length, label, signal, timeTag->getTimes(i));
                }
            }
        });
    }

    template <typename T>
    void stopMeasurement(Packet *packet, const Ptr<Chunk>& data, b offset, b length) {
        data->mapAllTags<T>(offset, length, [&] (b offset, b length, T *timeTag) {
            for (int i = 0; i < (int)timeTag->getTimesArraySize(); i++) {
                auto label = timeTag->getLabels(i);
                cMatchableString matchableLabel(label);
                if (labelMatcher.matches(&matchableLabel)) {
                    EV_INFO << "Stopping measurement on packet " << packet->getName() << ": "
                            << "range (" << offset << ", " << offset + length << "), ";
                    if (label != nullptr && *label != '\0')
                        EV_INFO << "label = " << label;
                    EV_INFO << std::endl;
                    timeTag->eraseLabels(i);
                    timeTag->eraseTimes(i);
                    break;
                }
            }
        });
    }

  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif // ifndef __INET_TIMINGMEASUREMENTMAKER_H


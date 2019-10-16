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

#include "inet/common/FlowTag.h"
#include "inet/queueing/timing/TimingMeasurementMaker.h"

namespace inet {
namespace queueing {

simsignal_t TimingMeasurementMaker::lifeTimeSignal = cComponent::registerSignal("lifeTime");
simsignal_t TimingMeasurementMaker::elapsedTimeSignal = cComponent::registerSignal("elapsedTime");
simsignal_t TimingMeasurementMaker::delayingTimeSignal = cComponent::registerSignal("delayingTime");
simsignal_t TimingMeasurementMaker::queueingTimeSignal = cComponent::registerSignal("queueingTime");
simsignal_t TimingMeasurementMaker::processingTimeSignal = cComponent::registerSignal("processingTime");
simsignal_t TimingMeasurementMaker::transmissionTimeSignal = cComponent::registerSignal("transmissionTime");
simsignal_t TimingMeasurementMaker::propagationTimeSignal = cComponent::registerSignal("propagationTime");

Define_Module(TimingMeasurementMaker);

static bool matchesString(cMatchExpression& matchExpression, const char *string)
{
    cMatchableString matchableString(string);
    return matchExpression.matches(&matchableString);
}

void TimingMeasurementMaker::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        packetFilter.setPattern(par("packetFilter"), par("packetDataFilter"));
        offset = b(par("offset"));
        length = b(par("length"));
        finish = par("finish");
        label = par("label");
        labelMatcher.setPattern(label, false, true, true);
        cMatchExpression measureMatcher;
        measureMatcher.setPattern(par("measure"), false, true, true);
        measureLifeTime = matchesString(measureMatcher, "lifeTime");
        measureElapsedTime = matchesString(measureMatcher, "elapsedTime");
        measureDelayingTime = matchesString(measureMatcher, "delayingTime");
        measureQueueingTime = matchesString(measureMatcher, "queueingTime");
        measureProcessingTime = matchesString(measureMatcher, "processingTime");
        measureTransmissionTime = matchesString(measureMatcher, "transmissionTime");
        measurePropagationTime = matchesString(measureMatcher, "propagationTime");
    }
}

void TimingMeasurementMaker::processPacket(Packet *packet)
{
    if (packetFilter.matches(packet)) {
        const auto& data = packet->peekData();
        if (measureLifeTime)
            data->mapAllTags<CreationTimeTag>(offset, length, [&] (b offset, b length, const CreationTimeTag *timeTag) {
                makeMeasurement(packet, offset, length, nullptr, lifeTimeSignal, simTime() - timeTag->getCreationTime());
            });
        if (measureElapsedTime) {
            data->mapAllTags<ElapsedTimeTag>(offset, length, [&] (b offset, b length, const ElapsedTimeTag *timeTag) {
                for (int i = 0; i < (int)timeTag->getTimesArraySize(); i++) {
                    auto label = timeTag->getLabels(i);
                    cMatchableString matchableLabel(label);
                    if (labelMatcher.matches(&matchableLabel)) {
                        cNamedObject signalDetails(label);
                        makeMeasurement(packet, offset, length, label, elapsedTimeSignal, simTime() - timeTag->getTimes(i));
                    }
                }
            });
        }
        if (measureDelayingTime)
            makeMeasurement<DelayingTimeTag>(packet, data, offset, length, delayingTimeSignal);
        if (measureQueueingTime)
            makeMeasurement<QueueingTimeTag>(packet, data, offset, length, queueingTimeSignal);
        if (measureProcessingTime)
            makeMeasurement<ProcessingTimeTag>(packet, data, offset, length, processingTimeSignal);
        if (measureTransmissionTime)
            makeMeasurement<TransmissionTimeTag>(packet, data, offset, length, transmissionTimeSignal);
        if (measurePropagationTime)
            makeMeasurement<PropagationTimeTag>(packet, data, offset, length, propagationTimeSignal);
        if (finish) {
            packet->trim();
            auto& data = packet->removeAll();
            if (measureElapsedTime)
                stopMeasurement<ElapsedTimeTag>(packet, data, offset, length);
            if (measureDelayingTime)
                stopMeasurement<DelayingTimeTag>(packet, data, offset, length);
            if (measureQueueingTime)
                stopMeasurement<QueueingTimeTag>(packet, data, offset, length);
            if (measureProcessingTime)
                stopMeasurement<ProcessingTimeTag>(packet, data, offset, length);
            if (measureTransmissionTime)
                stopMeasurement<TransmissionTimeTag>(packet, data, offset, length);
            if (measurePropagationTime)
                stopMeasurement<PropagationTimeTag>(packet, data, offset, length);
            packet->insertAtBack(data);
            // KLUDGE:
            cStringTokenizer tokenizer(label);
            while (tokenizer.hasMoreTokens())
                endPacketFlow(this, packet, tokenizer.nextToken());
        }
    }
}

void TimingMeasurementMaker::makeMeasurement(Packet *packet, b offset, b length, const char *label, simsignal_t signal, simtime_t value)
{
    EV_INFO << "Making measurement on packet " << packet->getName() << ": "
            << "range (" << offset << ", " << offset + length << "), ";
    if (label != nullptr && *label != '\0')
        EV_INFO << "label = " << label << ", ";
    EV_INFO << "signal = " << cComponent::getSignalName(signal) << ", "
            << "value = " << value << std::endl;
    cNamedObject signalDetails(label);
    emit(signal, value, &signalDetails);
}

} // namespace queueing
} // namespace inet


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

#include "inet/common/ModuleAccess.h"
#include "inet/common/queueing/MarkovScheduler.h"

namespace inet {
namespace queueing {

Define_Module(MarkovScheduler);

MarkovScheduler::~MarkovScheduler()
{
    cancelAndDelete(transitionTimer);
    cancelAndDelete(waitTimer);
}

void MarkovScheduler::initialize(int stage)
{
    PacketSchedulerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        for (int i = 0; i < gateSize("in"); i++) {
            auto input = check_and_cast<IPacketProducer *>(getConnectedModule(inputGates[i]));
            producers.push_back(input);
        }
        consumer = dynamic_cast<IPacketConsumer *>(getConnectedModule(outputGate));
        state = par("initialState");
        int numStates = gateSize("in");
        cStringTokenizer transitionProbabilitiesTokenizer(par("transitionProbabilities"));
        for (int i = 0; i < numStates; i++) {
            transitionProbabilities.push_back({});
            for (int j = 0; j < numStates; j++)
                transitionProbabilities[i].push_back(atof(transitionProbabilitiesTokenizer.nextToken()));
        }
        cStringTokenizer waitIntervalsTokenizer(par("waitIntervals"));
        for (int i = 0; i < numStates; i++)
            waitIntervals.push_back(SimTime::parse(waitIntervalsTokenizer.nextToken()));
        waitTimer = new cMessage("WaitTimer");
        WATCH(state);
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (auto inputGate : inputGates)
            checkPushPacketSupport(inputGate);
        if (consumer != nullptr)
            checkPushPacketSupport(outputGate);
        producers[state]->handleCanPushPacket(inputGates[state]);
        scheduleWaitTimer();
    }
}

void MarkovScheduler::handleMessage(cMessage *message)
{
    if (message == waitTimer) {
        double v = uniform(0, 1);
        double sum = 0;
        int numStates = (int)transitionProbabilities.size();
        for (int i = 0; i < numStates; i++) {
            sum += transitionProbabilities[state][i];
            if (sum >= v || i == numStates - 1) {
                state = i;
                break;
            }
        }
        producers[state]->handleCanPushPacket(inputGates[state]);
        scheduleWaitTimer();
    }
    else
        throw cRuntimeError("Unknown message");
}

int MarkovScheduler::schedulePacket()
{
    return state;
}

void MarkovScheduler::scheduleWaitTimer()
{
    scheduleAt(simTime() + waitIntervals[state], waitTimer);
}

bool MarkovScheduler::canPushSomePacket(cGate *gate)
{
    return gate->getIndex() == state;
}

bool MarkovScheduler::canPushPacket(Packet *packet, cGate *gate)
{
    return canPushSomePacket(gate);
}

void MarkovScheduler::pushPacket(Packet *packet, cGate *gate)
{
    if (gate->getIndex() != state)
        throw cRuntimeError("Cannot push to gate");
    processedTotalLength += packet->getDataLength();
    pushOrSendPacket(packet, outputGate, consumer);
    numProcessedPackets++;
    updateDisplayString();
}

void MarkovScheduler::handleCanPushPacket(cGate *gate)
{
    producers[state]->handleCanPushPacket(inputGates[state]);
}

} // namespace queueing
} // namespace inet

/* -*- mode:c++ -*- ********************************************************
 * file:        ChannelAccess.h
 *
 * author:      Marc Loebbers
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 **************************************************************************/


#ifndef CHANNEL_ACCESS_H
#define CHANNEL_ACCESS_H

#include <list>
#include <limits>

#include "INETDefs.h"

#include "BasicModule.h"
//#include "ChannelControl.h"
#include "ChannelControlExtended.h"

// Forward declarations
class AirFrame;

/**
 * @brief Basic class for all physical layers, please don't touch!!
 *
 * This class is not supposed to work on its own, but it contains
 * functions and lists that cooperate with ChannelControl to handle
 * the dynamically created gates. This means EVERY SnrEval (the lowest
 * layer in a host) has to be derived from this class!!!! And please
 * follow the instructions on how to declare a physical layer in a
 * .ned file in "The Design of a Mobility Framework in OMNeT++"
 * paper.
 *
 * Please don't touch this class.
 *
 * @author Marc L�bbers
 * @ingroup channelControl
 * @ingroup phyLayer
 */
class INET_API ChannelAccess : public BasicModule
{
  public:
    ChannelAccess() : cc(NULL), myHostRef(NULL),
            hostPos(std::numeric_limits<double>::min(), std::numeric_limits<double>::min()),
            posFromDisplayString(true) {}

    /**
     * Called by the NotificationBoard whenever a change of a category
     * occurs to which this client has subscribed.
     */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);

  protected:
    /** @brief Pointer to the ChannelControl module*/
    ChannelControlExtended* cc;

//    /** @brief Pointer to the ChannelControl module*/
//    ChannelControlExtended* ccExt;

    /** @brief Identifies this host in the ChannelControl module*/
    ChannelControlExtended::HostRef myHostRef;

    Coord hostPos;
    bool posFromDisplayString;

  protected:
    /** @brief Sends a message to all hosts in range*/
    virtual void sendToChannel(AirFrame *msg);

    /** @brief Returns the host's position*/
    const Coord& getMyPosition() {return cc->getHostPosition(myHostRef);}

  protected:
    /** @brief Register with ChannelControl and subscribe to hostPos*/
    virtual void initialize(int stage);

    virtual int numInitStages() const {return 3;}
};

#endif


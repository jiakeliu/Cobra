#include "event.h"

cobraTransferEvent::cobraTransferEvent()
    :cobraNetEvent(cobraTransferEventType)
{
}

cobraTransferEvent::cobraTransferEvent(cobraTransferEvent& state)
    :cobraNetEvent(state)
{
}

cobraTransferEvent::~cobraTransferEvent()
{
}

int
cobraTransferEvent::serialize(QDataStream& stream)
{
    return cobraNetEvent::serialize(stream);
}

int
cobraTransferEvent::deserialize(QDataStream& stream)
{
    return cobraNetEvent::deserialize(stream);
}

cobraNetEvent*
cobraTransferEvent::duplicate()
{
    cobraTransferEvent* xfer = new cobraTransferEvent(*this);
    return xfer;
}

cobraTransferEventHandler::cobraTransferEventHandler()
    :cobraNetEventHandler("FileTransfer", cobraTransferEventType)
{
}

cobraTransferEventHandler::cobraTransferEventHandler(cobraTransferEventHandler& event)
    :cobraNetEventHandler(event)
{
}

cobraTransferEventHandler::~cobraTransferEventHandler()
{}

bool
cobraTransferEventHandler::handleEvent(cobraNetEvent* event)
{
    (void)event;
    return false;
}

bool
cobraTransferEventHandler::handleServerEvent(cobraNetEvent* event)
{
    (void)event;
    return false;
}

cobraNetEvent*
cobraTransferEventHandler::eventGenesis() const
{
    return new cobraTransferEvent();
}


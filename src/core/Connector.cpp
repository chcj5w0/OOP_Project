#include "Connector.h"

ConnectorSnap Connector::snapshot() const {
    ConnectorSnap snap{};
    snap.id       = id();
    snap.typeName = typeName();
    snap.size     = size();
    snap.capacity = capacity();
    snap.slots    = slotNames();
    return snap;
}

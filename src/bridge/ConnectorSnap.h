#pragma once

#include <string>
#include <vector>

// Snapshot of a Connector for the UI layer.
// `slots` holds the name of the product occupying each position
// ("" = empty). Ordering convention: index 0 is the exit end
// (where the downstream machine pops). For a Pipeline the vector only
// contains the queued products front-first; for a Conveyor it always
// has `capacity` entries so the UI can draw empty belt slots.
struct ConnectorSnap {
    int                      id;
    const char*              typeName;
    int                      size;
    int                      capacity;
    std::vector<std::string> slots;
};

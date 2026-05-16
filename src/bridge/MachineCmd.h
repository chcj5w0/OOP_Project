#pragma once

struct MachineCmd {
    bool startWork     = false;
    bool forceBreak    = false;
    bool instantRepair = false;
    int  targetId      = -1;
};

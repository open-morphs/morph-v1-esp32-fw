#pragma once

#define HANDSET_DISCONNECTION_ATTEMPT_COMPLETE          (1UL)
#define HANDSET_DISCONNECTED_REMOTELY                   (1UL << 1)
#define HANDSET_CONNECTION_ATTEMPT_COMPLETE             (1UL << 2)
#define HANDSET_CONNECTED_REMOTELY                      (1UL << 3)
#define HANDSET_INFORMATION_RECEIVED                    (1UL << 4)
#define HANDSET_STATE_RECEIVED                          (1UL << 5)

#define ALL_EVENTS_MASK                                 (0xFFFF)
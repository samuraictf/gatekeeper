#pragma once

// Set to True if we're running under QEMU.
// This could mean that we're being debugged, but it
// could also be part of the standard operation so we
// do not exit immediately.
extern int qemu_detected;

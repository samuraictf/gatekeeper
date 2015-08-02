#pragma once

/**
 * Prevent this process from killing any other processes, or
 * changing its program group ID or session ID.
 */
void install_child_control();

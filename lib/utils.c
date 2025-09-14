/*
 * PCIe Simulator Library - Utility Functions
 *
 * Copyright (c) 2025 Karan Mamaniya <kmamaniya@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Utility functions for error handling and helper operations.
 */

#include "api.h"

/*
 * Convert error code to human-readable string
 */
const char *pcie_sim_error_string(pcie_sim_error_t error)
{
    switch (error) {
    case PCIE_SIM_SUCCESS:
        return "Success";
    case PCIE_SIM_ERROR_DEVICE:
        return "Device error - check if device exists and is accessible";
    case PCIE_SIM_ERROR_PARAM:
        return "Invalid parameter - check function arguments";
    case PCIE_SIM_ERROR_MEMORY:
        return "Memory allocation error - insufficient memory";
    case PCIE_SIM_ERROR_TIMEOUT:
        return "Operation timeout - device may be busy";
    case PCIE_SIM_ERROR_SYSTEM:
        return "System error - check kernel logs and device status";
    default:
        return "Unknown error code";
    }
}
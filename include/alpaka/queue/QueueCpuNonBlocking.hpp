/* Copyright 2020 Jeffrey Kelling
 *
 * This file is part of Alpaka.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <alpaka/queue/QueueGenericThreadsNonBlocking.hpp>
#include <alpaka/event/EventCpu.hpp>

namespace alpaka
{
    namespace queue
    {
        using QueueCpuNonBlocking = QueueGenericThreadsNonBlocking<dev::DevCpu>;
    }
}

/**
* \file
* Copyright 2014-2015 Benjamin Worpitz, Rene Widera
*
* This file is part of alpaka.
*
* alpaka is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* alpaka is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with alpaka.
* If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <alpaka/core/Common.hpp>   // ALPAKA_FN_HOST_ACC

#include <type_traits>              // std::enable_if, std::is_base_of, std::is_same, std::decay

namespace alpaka
{
    //-----------------------------------------------------------------------------
    //! The grid block specifics
    //-----------------------------------------------------------------------------
    namespace block
    {
        //-----------------------------------------------------------------------------
        //! The block shared memory operation specifics.
        //-----------------------------------------------------------------------------
        namespace shared
        {
            //-----------------------------------------------------------------------------
            //! The block shared dynamic memory operation specifics.
            //-----------------------------------------------------------------------------
            namespace dyn
            {
                //-----------------------------------------------------------------------------
                //! The block shared dynamic memory operation traits.
                //-----------------------------------------------------------------------------
                namespace traits
                {
                    //#############################################################################
                    //! The block shared dynamic memory get trait.
                    //#############################################################################
                    template<
                        typename T,
                        typename TBlockSharedMemDyn,
                        typename TSfinae = void>
                    struct GetMem;
                }

                //-----------------------------------------------------------------------------
                //! Returns the pointr to the block shared dynamic memory.
                //!
                //! \tparam T The element type.
                //! \tparam TBlockSharedMemDyn The block shared dynamic memory implementation type.
                //! \param blockSharedMemDyn The block shared dynamic memory implementation.
                //-----------------------------------------------------------------------------
                ALPAKA_NO_HOST_ACC_WARNING
                template<
                    typename T,
                    typename TBlockSharedMemDyn>
                ALPAKA_FN_HOST_ACC auto getMem(
                    TBlockSharedMemDyn const & blockSharedMemDyn)
                -> T *
                {
                    return
                        traits::GetMem<
                            T,
                            TBlockSharedMemDyn>
                        ::getMem(
                            blockSharedMemDyn);
                }

                namespace traits
                {
                    //#############################################################################
                    //! The AllocVar trait specialization for classes with BlockSharedMemDynBase member type.
                    //#############################################################################
                    template<
                        typename T,
                        typename TBlockSharedMemDyn>
                    struct GetMem<
                        T,
                        TBlockSharedMemDyn,
                        typename std::enable_if<
                            std::is_base_of<typename TBlockSharedMemDyn::BlockSharedMemDynBase, typename std::decay<TBlockSharedMemDyn>::type>::value
                            && (!std::is_same<typename TBlockSharedMemDyn::BlockSharedMemDynBase, typename std::decay<TBlockSharedMemDyn>::type>::value)>::type>
                    {
                        //-----------------------------------------------------------------------------
                        //!
                        //-----------------------------------------------------------------------------
                        ALPAKA_NO_HOST_ACC_WARNING
                        ALPAKA_FN_HOST_ACC static auto getMem(
                            TBlockSharedMemDyn const & blockSharedMemDyn)
                        -> T *
                        {
                            // Delegate the call to the base class.
                            return
                                block::shared::dyn::getMem<
                                    T>(
                                        static_cast<typename TBlockSharedMemDyn::BlockSharedMemDynBase const &>(blockSharedMemDyn));
                        }
                    };
                }
            }
        }
    }
}
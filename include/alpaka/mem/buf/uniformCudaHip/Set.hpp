/* Copyright 2019 Benjamin Worpitz, Erik Zenker, Matthias Werner, René Widera
 *
 * This file is part of Alpaka.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#if defined(ALPAKA_ACC_GPU_CUDA_ENABLED) || defined(ALPAKA_ACC_GPU_HIP_ENABLED)

#include <alpaka/core/BoostPredef.hpp>

#if !BOOST_LANG_CUDA && !BOOST_LANG_HIP
    #error Compiler has to support CUDA/HIP!
#endif

#include <alpaka/queue/QueueUniformCudaHipRtBlocking.hpp>
#include <alpaka/queue/QueueUniformCudaHipRtNonBlocking.hpp>

#include <alpaka/dev/Traits.hpp>
#include <alpaka/dim/DimIntegralConst.hpp>
#include <alpaka/extent/Traits.hpp>
#include <alpaka/mem/view/Traits.hpp>
#include <alpaka/queue/Traits.hpp>
#include <alpaka/wait/Traits.hpp>

#include <alpaka/core/Assert.hpp>

// Backend specific includes.
#if defined(ALPAKA_ACC_GPU_CUDA_ENABLED)
#include <alpaka/core/Cuda.hpp>
#else
#include <alpaka/core/Hip.hpp>
#endif


namespace alpaka
{
    namespace dev
    {
        class DevUniformCudaHipRt;
    }
}

namespace alpaka
{
    namespace mem
    {
        namespace view
        {
            namespace uniform_cuda_hip
            {
                namespace detail
                {
                    //#############################################################################
                    //! The CUDA memory set trait.
                    template<
                        typename TDim,
                        typename TView,
                        typename TExtent>
                    struct TaskSetUniformCudaHip
                    {
                        //-----------------------------------------------------------------------------
                        TaskSetUniformCudaHip(
                            TView & view,
                            std::uint8_t const & byte,
                            TExtent const & extent) :
                                m_view(view),
                                m_byte(byte),
                                m_extent(extent),
                                m_iDevice(dev::getDev(view).m_iDevice)
                        {
                            static_assert(
                                !std::is_const<TView>::value,
                                "The destination view can not be const!");

                            static_assert(
                                dim::Dim<TView>::value == dim::Dim<TExtent>::value,
                                "The destination view and the extent are required to have the same dimensionality!");
                        }

                        TView & m_view;
                        std::uint8_t const m_byte;
                        TExtent const m_extent;
                        std::int32_t const m_iDevice;
                    };
                }
            }
            namespace traits
            {
                //#############################################################################
                //! The CUDA device memory set trait specialization.
                template<
                    typename TDim>
                struct CreateTaskSet<
                    TDim,
                    dev::DevUniformCudaHipRt>
                {
                    //-----------------------------------------------------------------------------
                    template<
                        typename TExtent,
                        typename TView>
                    ALPAKA_FN_HOST static auto createTaskSet(
                        TView & view,
                        std::uint8_t const & byte,
                        TExtent const & extent)
                    -> mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<
                        TDim,
                        TView,
                        TExtent>
                    {
                        return
                            mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<
                                TDim,
                                TView,
                                TExtent>(
                                    view,
                                    byte,
                                    extent);
                    }
                };
            }
        }
    }
    namespace queue
    {
        namespace traits
        {
            //#############################################################################
            //! The CUDA non-blocking device queue 1D set enqueue trait specialization.
            template<
                typename TView,
                typename TExtent>
            struct Enqueue<
                queue::QueueUniformCudaHipRtNonBlocking,
                mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<1u>, TView, TExtent>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto enqueue(
                    queue::QueueUniformCudaHipRtNonBlocking & queue,
                    mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<1u>, TView, TExtent> const & task)
                -> void
                {
                    ALPAKA_DEBUG_MINIMAL_LOG_SCOPE;

                    static_assert(
                        dim::Dim<TView>::value == 1u,
                        "The destination buffer is required to be 1-dimensional for this specialization!");
                    static_assert(
                        dim::Dim<TView>::value == dim::Dim<TExtent>::value,
                        "The destination buffer and the extent are required to have the same dimensionality!");

                    using Idx = idx::Idx<TExtent>;

                    auto & view(task.m_view);
                    auto const & byte(task.m_byte);
                    auto const & extent(task.m_extent);
                    auto const & iDevice(task.m_iDevice);

                    auto const extentWidth(extent::getWidth(extent));

                    if(extentWidth == 0)
                    {
                        return;
                    }

                    auto const extentWidthBytes(extentWidth * static_cast<Idx>(sizeof(elem::Elem<TView>)));
#if !defined(NDEBUG)
                    auto const dstWidth(extent::getWidth(view));
#endif
                    auto const dstNativePtr(reinterpret_cast<void *>(mem::view::getPtrNative(view)));
                    ALPAKA_ASSERT(extentWidth <= dstWidth);

                    // Set the current device.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(SetDevice)(
                            iDevice));
                    // Initiate the memory set.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(MemsetAsync)(
                            dstNativePtr,
                            static_cast<int>(byte),
                            static_cast<size_t>(extentWidthBytes),
                            queue.m_spQueueImpl->m_UniformCudaHipQueue));
                }
            };
            //#############################################################################
            //! The CUDA blocking device queue 1D set enqueue trait specialization.
            template<
                typename TView,
                typename TExtent>
            struct Enqueue<
                queue::QueueUniformCudaHipRtBlocking,
                mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<1u>, TView, TExtent>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto enqueue(
                    queue::QueueUniformCudaHipRtBlocking & queue,
                    mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<1u>, TView, TExtent> const & task)
                -> void
                {
                    ALPAKA_DEBUG_MINIMAL_LOG_SCOPE;

                    static_assert(
                        dim::Dim<TView>::value == 1u,
                        "The destination buffer is required to be 1-dimensional for this specialization!");
                    static_assert(
                        dim::Dim<TView>::value == dim::Dim<TExtent>::value,
                        "The destination buffer and the extent are required to have the same dimensionality!");

                    using Idx = idx::Idx<TExtent>;

                    auto & view(task.m_view);
                    auto const & byte(task.m_byte);
                    auto const & extent(task.m_extent);
                    auto const & iDevice(task.m_iDevice);

                    auto const extentWidth(extent::getWidth(extent));

                    if(extentWidth == 0)
                    {
                        return;
                    }

                    auto const extentWidthBytes(extentWidth * static_cast<Idx>(sizeof(elem::Elem<TView>)));
#if !defined(NDEBUG)
                    auto const dstWidth(extent::getWidth(view));
#endif
                    auto const dstNativePtr(reinterpret_cast<void *>(mem::view::getPtrNative(view)));
                    ALPAKA_ASSERT(extentWidth <= dstWidth);

                    // Set the current device.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(SetDevice)(
                            iDevice));
                    // Initiate the memory set.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(MemsetAsync)(
                            dstNativePtr,
                            static_cast<int>(byte),
                            static_cast<size_t>(extentWidthBytes),
                            queue.m_spQueueImpl->m_UniformCudaHipQueue));
                    wait::wait(queue);
                }
            };
            //#############################################################################
            //! The CUDA non-blocking device queue 2D set enqueue trait specialization.
            template<
                typename TView,
                typename TExtent>
            struct Enqueue<
                queue::QueueUniformCudaHipRtNonBlocking,
                mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<2u>, TView, TExtent>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto enqueue(
                    queue::QueueUniformCudaHipRtNonBlocking & queue,
                    mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<2u>, TView, TExtent> const & task)
                -> void
                {
                    ALPAKA_DEBUG_MINIMAL_LOG_SCOPE;

                    static_assert(
                        dim::Dim<TView>::value == 2u,
                        "The destination buffer is required to be 2-dimensional for this specialization!");
                    static_assert(
                        dim::Dim<TView>::value == dim::Dim<TExtent>::value,
                        "The destination buffer and the extent are required to have the same dimensionality!");

                    using Idx = idx::Idx<TExtent>;

                    auto & view(task.m_view);
                    auto const & byte(task.m_byte);
                    auto const & extent(task.m_extent);
                    auto const & iDevice(task.m_iDevice);

                    auto const extentWidth(extent::getWidth(extent));
                    auto const extentHeight(extent::getHeight(extent));

                    if(extentWidth == 0 || extentHeight == 0)
                    {
                        return;
                    }

                    auto const extentWidthBytes(extentWidth * static_cast<Idx>(sizeof(elem::Elem<TView>)));

#if !defined(NDEBUG)
                    auto const dstWidth(extent::getWidth(view));
                    auto const dstHeight(extent::getHeight(view));
#endif
                    auto const dstPitchBytesX(mem::view::getPitchBytes<dim::Dim<TView>::value - 1u>(view));
                    auto const dstNativePtr(reinterpret_cast<void *>(mem::view::getPtrNative(view)));
                    ALPAKA_ASSERT(extentWidth <= dstWidth);
                    ALPAKA_ASSERT(extentHeight <= dstHeight);

                    // Set the current device.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(SetDevice)(
                            iDevice));
                    // Initiate the memory set.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(Memset2DAsync)(
                            dstNativePtr,
                            static_cast<size_t>(dstPitchBytesX),
                            static_cast<int>(byte),
                            static_cast<size_t>(extentWidthBytes),
                            static_cast<size_t>(extentHeight),
                            queue.m_spQueueImpl->m_UniformCudaHipQueue));
                }
            };
            //#############################################################################
            //! The CUDA blocking device queue 2D set enqueue trait specialization.
            template<
                typename TView,
                typename TExtent>
            struct Enqueue<
                queue::QueueUniformCudaHipRtBlocking,
                mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<2u>, TView, TExtent>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto enqueue(
                    queue::QueueUniformCudaHipRtBlocking & queue,
                    mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<2u>, TView, TExtent> const & task)
                -> void
                {
                    ALPAKA_DEBUG_MINIMAL_LOG_SCOPE;

                    static_assert(
                        dim::Dim<TView>::value == 2u,
                        "The destination buffer is required to be 2-dimensional for this specialization!");
                    static_assert(
                        dim::Dim<TView>::value == dim::Dim<TExtent>::value,
                        "The destination buffer and the extent are required to have the same dimensionality!");

                    using Idx = idx::Idx<TExtent>;

                    auto & view(task.m_view);
                    auto const & byte(task.m_byte);
                    auto const & extent(task.m_extent);
                    auto const & iDevice(task.m_iDevice);

                    auto const extentWidth(extent::getWidth(extent));
                    auto const extentHeight(extent::getHeight(extent));

                    if(extentWidth == 0 || extentHeight == 0)
                    {
                        return;
                    }

                    auto const extentWidthBytes(extentWidth * static_cast<Idx>(sizeof(elem::Elem<TView>)));

#if !defined(NDEBUG)
                    auto const dstWidth(extent::getWidth(view));
                    auto const dstHeight(extent::getHeight(view));
#endif
                    auto const dstPitchBytesX(mem::view::getPitchBytes<dim::Dim<TView>::value - 1u>(view));
                    auto const dstNativePtr(reinterpret_cast<void *>(mem::view::getPtrNative(view)));
                    ALPAKA_ASSERT(extentWidth <= dstWidth);
                    ALPAKA_ASSERT(extentHeight <= dstHeight);

                    // Set the current device.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(SetDevice)(
                            iDevice));

                    // Initiate the memory set.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(Memset2DAsync)(
                            dstNativePtr,
                            static_cast<size_t>(dstPitchBytesX),
                            static_cast<int>(byte),
                            static_cast<size_t>(extentWidthBytes),
                            static_cast<size_t>(extentHeight),
                            queue.m_spQueueImpl->m_UniformCudaHipQueue));

                    wait::wait(queue);
                }
            };
            //#############################################################################
            //! The CUDA non-blocking device queue 3D set enqueue trait specialization.
            template<
                typename TView,
                typename TExtent>
            struct Enqueue<
                queue::QueueUniformCudaHipRtNonBlocking,
                mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<3u>, TView, TExtent>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto enqueue(
                    queue::QueueUniformCudaHipRtNonBlocking & queue,
                    mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<3u>, TView, TExtent> const & task)
                -> void
                {
                    ALPAKA_DEBUG_MINIMAL_LOG_SCOPE;

                    static_assert(
                        dim::Dim<TView>::value == 3u,
                        "The destination buffer is required to be 3-dimensional for this specialization!");
                    static_assert(
                        dim::Dim<TView>::value == dim::Dim<TExtent>::value,
                        "The destination buffer and the extent are required to have the same dimensionality!");

                    using Elem = alpaka::elem::Elem<TView>;
                    using Idx = idx::Idx<TExtent>;

                    auto & view(task.m_view);
                    auto const & byte(task.m_byte);
                    auto const & extent(task.m_extent);
                    auto const & iDevice(task.m_iDevice);

                    auto const extentWidth(extent::getWidth(extent));
                    auto const extentHeight(extent::getHeight(extent));
                    auto const extentDepth(extent::getDepth(extent));

                    // This is not only an optimization but also prevents a division by zero.
                    if(extentWidth == 0 || extentHeight == 0 || extentDepth == 0)
                    {
                        return;
                    }

                    auto const dstWidth(extent::getWidth(view));
#if !defined(NDEBUG)
                    auto const dstHeight(extent::getHeight(view));
                    auto const dstDepth(extent::getDepth(view));
#endif
                    auto const dstPitchBytesX(mem::view::getPitchBytes<dim::Dim<TView>::value - 1u>(view));
                    auto const dstPitchBytesY(mem::view::getPitchBytes<dim::Dim<TView>::value - (2u % dim::Dim<TView>::value)>(view));
                    auto const dstNativePtr(reinterpret_cast<void *>(mem::view::getPtrNative(view)));
                    ALPAKA_ASSERT(extentWidth <= dstWidth);
                    ALPAKA_ASSERT(extentHeight <= dstHeight);
                    ALPAKA_ASSERT(extentDepth <= dstDepth);

                    // Fill CUDA parameter structures.
                    ALPAKA_API_PREFIX(PitchedPtr) const pitchedPtrVal(
                        ALPAKA_PP_CONCAT(make_,ALPAKA_API_PREFIX(PitchedPtr))(
                            dstNativePtr,
                            static_cast<size_t>(dstPitchBytesX),
                            static_cast<size_t>(dstWidth * static_cast<Idx>(sizeof(Elem))),
                            static_cast<size_t>(dstPitchBytesY / dstPitchBytesX)));

                    ALPAKA_API_PREFIX(Extent) const extentVal(
                        ALPAKA_PP_CONCAT(make_,ALPAKA_API_PREFIX(Extent))(
                            static_cast<size_t>(extentWidth * static_cast<Idx>(sizeof(Elem))),
                            static_cast<size_t>(extentHeight),
                            static_cast<size_t>(extentDepth)));

                    // Set the current device.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(SetDevice)(
                            iDevice));
                    // Initiate the memory set.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(Memset3DAsync)(
                            pitchedPtrVal,
                            static_cast<int>(byte),
                            extentVal,
                            queue.m_spQueueImpl->m_UniformCudaHipQueue));
                }
            };
            //#############################################################################
            //! The CUDA blocking device queue 3D set enqueue trait specialization.
            template<
                typename TView,
                typename TExtent>
            struct Enqueue<
                queue::QueueUniformCudaHipRtBlocking,
                mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<3u>, TView, TExtent>>
            {
                //-----------------------------------------------------------------------------
                ALPAKA_FN_HOST static auto enqueue(
                    queue::QueueUniformCudaHipRtBlocking & queue,
                    mem::view::uniform_cuda_hip::detail::TaskSetUniformCudaHip<dim::DimInt<3u>, TView, TExtent> const & task)
                -> void
                {
                    ALPAKA_DEBUG_MINIMAL_LOG_SCOPE;

                    static_assert(
                        dim::Dim<TView>::value == 3u,
                        "The destination buffer is required to be 3-dimensional for this specialization!");
                    static_assert(
                        dim::Dim<TView>::value == dim::Dim<TExtent>::value,
                        "The destination buffer and the extent are required to have the same dimensionality!");

                    using Elem = alpaka::elem::Elem<TView>;
                    using Idx = idx::Idx<TExtent>;

                    auto & view(task.m_view);
                    auto const & byte(task.m_byte);
                    auto const & extent(task.m_extent);
                    auto const & iDevice(task.m_iDevice);

                    auto const extentWidth(extent::getWidth(extent));
                    auto const extentHeight(extent::getHeight(extent));
                    auto const extentDepth(extent::getDepth(extent));

                    // This is not only an optimization but also prevents a division by zero.
                    if(extentWidth == 0 || extentHeight == 0 || extentDepth == 0)
                    {
                        return;
                    }

                    auto const dstWidth(extent::getWidth(view));
#if !defined(NDEBUG)
                    auto const dstHeight(extent::getHeight(view));
                    auto const dstDepth(extent::getDepth(view));
#endif
                    auto const dstPitchBytesX(mem::view::getPitchBytes<dim::Dim<TView>::value - 1u>(view));
                    auto const dstPitchBytesY(mem::view::getPitchBytes<dim::Dim<TView>::value - (2u % dim::Dim<TView>::value)>(view));
                    auto const dstNativePtr(reinterpret_cast<void *>(mem::view::getPtrNative(view)));
                    ALPAKA_ASSERT(extentWidth <= dstWidth);
                    ALPAKA_ASSERT(extentHeight <= dstHeight);
                    ALPAKA_ASSERT(extentDepth <= dstDepth);

                    // Fill CUDA parameter structures.
                    ALPAKA_API_PREFIX(PitchedPtr) const pitchedPtrVal(
                        ALPAKA_PP_CONCAT(make_,ALPAKA_API_PREFIX(PitchedPtr))(
                            dstNativePtr,
                            static_cast<size_t>(dstPitchBytesX),
                            static_cast<size_t>(dstWidth * static_cast<Idx>(sizeof(Elem))),
                            static_cast<size_t>(dstPitchBytesY / dstPitchBytesX)));

                    ALPAKA_API_PREFIX(Extent) const extentVal(
                        ALPAKA_PP_CONCAT(make_,ALPAKA_API_PREFIX(Extent))(
                            static_cast<size_t>(extentWidth * static_cast<Idx>(sizeof(Elem))),
                            static_cast<size_t>(extentHeight),
                            static_cast<size_t>(extentDepth)));

                    // Set the current device.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(SetDevice)(
                            iDevice));
                    // Initiate the memory set.
                    ALPAKA_UNIFORM_CUDA_HIP_RT_CHECK(
                        ALPAKA_API_PREFIX(Memset3DAsync)(
                            pitchedPtrVal,
                            static_cast<int>(byte),
                            extentVal,
                            queue.m_spQueueImpl->m_UniformCudaHipQueue));

                    wait::wait(queue);
                }
            };
        }
    }
}

#endif

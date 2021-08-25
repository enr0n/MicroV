/// @copyright
/// Copyright (C) 2020 Assured Information Security, Inc.
///
/// @copyright
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// @copyright
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// @copyright
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.

#include <bf_control_ops.hpp>
#include <bf_syscall_t.hpp>
#include <dispatch_bootstrap.hpp>
#include <dispatch_fail.hpp>
#include <dispatch_vmexit.hpp>
#include <gs_initialize.hpp>
#include <gs_t.hpp>
#include <intrinsic_t.hpp>
#include <pp_pool_t.hpp>
#include <tls_t.hpp>
#include <vm_pool_t.hpp>
#include <vp_pool_t.hpp>
#include <vps_pool_t.hpp>

#include <bsl/convert.hpp>
#include <bsl/debug.hpp>
#include <bsl/unlikely.hpp>

namespace microv
{
    /// @brief stores the bf_syscall_t that MicroV will use
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    constinit syscall::bf_syscall_t g_mut_sys{};
    /// @brief stores the intrinsic_t that MicroV will use
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    constinit intrinsic_t g_mut_intrinsic{};

    /// @brief stores the pool of PPs that MicroV will use
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    constinit pp_pool_t g_mut_pp_pool{};
    /// @brief stores the pool of VMs that MicroV will use
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    constinit vm_pool_t g_mut_vm_pool{};
    /// @brief stores the pool of VPs that MicroV will use
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    constinit vp_pool_t g_mut_vp_pool{};
    /// @brief stores the pool of VPSs that MicroV will use
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    constinit vps_pool_t g_mut_vps_pool{};

    /// @brief stores the Global Storage for MicroV
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    constinit gs_t g_mut_gs{};
    /// @brief stores the Thread Local Storage for MicroV on each PP
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    thread_local tls_t g_mut_tls{};

    /// <!-- description -->
    ///   @brief Implements the bootstrap entry function. This function is
    ///     called on each PP while the hypervisor is being bootstrapped.
    ///
    /// <!-- inputs/outputs -->
    ///   @param ppid the physical process to bootstrap
    ///
    extern "C" void
    bootstrap_entry(bsl::safe_uint16::value_type const ppid) noexcept
    {
        auto const ret{dispatch_bootstrap(    // --
            g_mut_gs,                         // --
            g_mut_tls,                        // --
            g_mut_sys,                        // --
            g_mut_intrinsic,                  // --
            g_mut_vp_pool,                    // --
            g_mut_vps_pool,                   // --
            bsl::to_u16(ppid))};

        if (bsl::unlikely(!ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        return syscall::bf_control_op_exit();
    }

    /// <!-- description -->
    ///   @brief Implements the fast fail entry function. This is registered
    ///     by the main function to execute whenever a fast fail occurs.
    ///
    /// <!-- inputs/outputs -->
    ///   @param vpsid the ID of the VPS that generated the fail
    ///   @param fail_reason the exit reason associated with the fail
    ///
    extern "C" void
    fail_entry(
        bsl::safe_uint16::value_type const vpsid,
        syscall::bf_status_t::value_type const fail_reason) noexcept
    {
        auto const ret{dispatch_fail(    // --
            g_mut_gs,                    // --
            g_mut_tls,                   // --
            g_mut_sys,                   // --
            g_mut_intrinsic,             // --
            g_mut_pp_pool,               // --
            g_mut_vm_pool,               // --
            g_mut_vp_pool,               // --
            g_mut_vps_pool,              // --
            bsl::to_u16(vpsid),          // --
            bsl::to_u64(fail_reason))};

        if (bsl::unlikely(!ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        return syscall::bf_control_op_exit();
    }

    /// <!-- description -->
    ///   @brief Implements the VMExit entry function. This is registered
    ///     by the main function to execute whenever a VMExit occurs.
    ///
    /// <!-- inputs/outputs -->
    ///   @param vpsid the ID of the VPS that generated the VMExit
    ///   @param exit_reason the exit reason associated with the VMExit
    ///
    extern "C" void
    vmexit_entry(
        bsl::safe_uint16::value_type const vpsid,
        bsl::safe_uint64::value_type const exit_reason) noexcept
    {
        auto const ret{dispatch_vmexit(    // --
            g_mut_gs,                      // --
            g_mut_tls,                     // --
            g_mut_sys,                     // --
            g_mut_intrinsic,               // --
            g_mut_pp_pool,                 // --
            g_mut_vm_pool,                 // --
            g_mut_vp_pool,                 // --
            g_mut_vps_pool,                // --
            bsl::to_u16(vpsid),            // --
            bsl::to_u64(exit_reason))};

        if (bsl::unlikely(!ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        return syscall::bf_control_op_exit();
    }

    /// <!-- description -->
    ///   @brief Implements the main entry function for this microv
    ///
    /// <!-- inputs/outputs -->
    ///   @param version the version of the spec implemented by the
    ///     microkernel.
    ///
    extern "C" void
    ext_main_entry(bsl::uint32 const version) noexcept
    {
        bsl::errc_type mut_ret{};

        mut_ret = g_mut_sys.initialize(    // --
            bsl::to_u32(version),          // --
            &bootstrap_entry,              // --
            &vmexit_entry,                 // --
            &fail_entry);                  // --

        if (bsl::unlikely(!mut_ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        mut_ret = g_mut_intrinsic.initialize(g_mut_gs, g_mut_tls);
        if (bsl::unlikely(!mut_ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        mut_ret = gs_initialize(g_mut_gs, g_mut_sys, g_mut_intrinsic);
        if (bsl::unlikely(!mut_ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        mut_ret = g_mut_vm_pool.initialize(g_mut_gs, g_mut_tls, g_mut_sys, g_mut_intrinsic);
        if (bsl::unlikely(!mut_ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        mut_ret = g_mut_vp_pool.initialize(g_mut_gs, g_mut_tls, g_mut_sys, g_mut_intrinsic);
        if (bsl::unlikely(!mut_ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        mut_ret = g_mut_vps_pool.initialize(g_mut_gs, g_mut_tls, g_mut_sys, g_mut_intrinsic);
        if (bsl::unlikely(!mut_ret)) {
            bsl::print<bsl::V>() << bsl::here();
            return syscall::bf_control_op_exit();
        }

        return syscall::bf_control_op_wait();
    }
}
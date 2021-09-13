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

#ifndef MV_HYPERCALL_IMPL_HPP
#define MV_HYPERCALL_IMPL_HPP

#include <bsl/safe_integral.hpp>

namespace hypercall
{
    // -------------------------------------------------------------------------
    // mv_id_ops
    // -------------------------------------------------------------------------

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_id_op_version.
    ///
    /// <!-- inputs/outputs -->
    ///   @param pmut_reg0_out n/a
    ///   @return n/a
    ///
    extern "C" [[nodiscard]] auto mv_id_op_version_impl(bsl::uint32 *const pmut_reg0_out) noexcept
        -> bsl::uint64;

    // -------------------------------------------------------------------------
    // mv_handle_ops
    // -------------------------------------------------------------------------

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_handle_op_open_handle.
    ///
    /// <!-- inputs/outputs -->
    ///   @param reg0_in n/a
    ///   @param pmut_reg0_out n/a
    ///   @return n/a
    ///
    extern "C" [[nodiscard]] auto mv_handle_op_open_handle_impl(
        bsl::uint32 const reg0_in, bsl::uint64 *const pmut_reg0_out) noexcept -> bsl::uint64;

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_handle_op_close_handle.
    ///
    /// <!-- inputs/outputs -->
    ///   @param reg0_in n/a
    ///   @return n/a
    ///
    extern "C" [[nodiscard]] auto mv_handle_op_close_handle_impl(bsl::uint64 const reg0_in) noexcept
        -> bsl::uint64;

    // -------------------------------------------------------------------------
    // mv_debug_ops
    // -------------------------------------------------------------------------

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_debug_op_out.
    ///
    /// <!-- inputs/outputs -->
    ///   @param reg0_in n/a
    ///   @param reg1_in n/a
    ///
    extern "C" void
    mv_debug_op_out_impl(bsl::uint64 const reg0_in, bsl::uint64 const reg1_in) noexcept;

    // -------------------------------------------------------------------------
    // mv_pp_ops
    // -------------------------------------------------------------------------

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_pp_op_clr_shared_page_gpa.
    ///
    /// <!-- inputs/outputs -->
    ///   @param reg0_in n/a
    ///   @return n/a
    ///
    extern "C" [[nodiscard]] auto
    mv_pp_op_clr_shared_page_gpa_impl(bsl::uint64 const reg0_in) noexcept -> bsl::uint64;

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_pp_op_set_shared_page_gpa.
    ///
    /// <!-- inputs/outputs -->
    ///   @param reg0_in n/a
    ///   @param reg1_in n/a
    ///   @return n/a
    ///
    extern "C" [[nodiscard]] auto
    mv_pp_op_set_shared_page_gpa_impl(bsl::uint64 const reg0_in, bsl::uint64 const reg1_in) noexcept
        -> bsl::uint64;

    // -------------------------------------------------------------------------
    // mv_vs_ops
    // -------------------------------------------------------------------------

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_vs_op_gva_to_gla.
    ///
    /// <!-- inputs/outputs -->
    ///   @param reg0_in n/a
    ///   @param reg1_in n/a
    ///   @param reg2_in n/a
    ///   @param pmut_reg0_out n/a
    ///   @return n/a
    ///
    extern "C" [[nodiscard]] auto mv_vs_op_gva_to_gla_impl(
        bsl::uint64 const reg0_in,
        bsl::uint32 const reg1_in,
        void const *const reg2_in,
        bsl::uint64 *const pmut_reg0_out) noexcept -> bsl::uint64;

    /// <!-- description -->
    ///   @brief Implements the ABI for mv_vs_op_gla_to_gpa.
    ///
    /// <!-- inputs/outputs -->
    ///   @param reg0_in n/a
    ///   @param reg1_in n/a
    ///   @param reg2_in n/a
    ///   @param pmut_reg0_out n/a
    ///   @return n/a
    ///
    extern "C" [[nodiscard]] auto mv_vs_op_gla_to_gpa_impl(
        bsl::uint64 const reg0_in,
        bsl::safe_u16::value_type const reg1_in,
        bsl::uint64 const reg2_in,
        bsl::uint64 *const pmut_reg0_out) noexcept -> bsl::uint64;
}

#endif
#include <migraph/gpu/concat.hpp>
#include <migraph/operators.hpp>
#include <migraph/manage_ptr.hpp>
#include <migraph/gpu/miopen.hpp>
#include <migraph/gpu/device/concat.hpp>
#include <utility>

namespace migraph {
namespace gpu {

shape hip_concat::compute_shape(std::vector<shape> inputs) const
{
    inputs.pop_back();
    return op.compute_shape(inputs);
}

argument
hip_concat::compute(context& ctx, const shape& output_shape, const std::vector<argument>& args) const
{
    std::vector<std::size_t> offsets = op.compute_offsets(output_shape, args);
    return device::concat(ctx.get_stream().get(), output_shape, args, offsets);
}

} // namespace gpu

} // namespace migraph

#include <migraphx/simplify_algebra.hpp>
#include <migraphx/dead_code_elimination.hpp>
#include <migraphx/pass_manager.hpp>
#include <migraphx/operators.hpp>
#include <migraphx/generate.hpp>
#include <migraphx/ranges.hpp>
#include <migraphx/instruction.hpp>
#include <basic_ops.hpp>
#include <test.hpp>

void run_pass(migraphx::program& p)
{
    migraphx::run_passes(p, {migraphx::simplify_algebra{}, migraphx::dead_code_elimination{}});
}

TEST_CASE(simplify_add1)
{
    migraphx::program p1;
    {
        auto x    = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y    = p1.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto one  = p1.add_literal(1);
        auto two  = p1.add_literal(2);
        auto sum1 = p1.add_instruction(migraphx::op::add{}, x, one);
        auto sum2 = p1.add_instruction(migraphx::op::add{}, y, two);
        auto sum3 = p1.add_instruction(migraphx::op::add{}, sum1, sum2);
        p1.add_instruction(pass_op{}, sum3);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x    = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y    = p2.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto one  = p2.add_literal(1);
        auto two  = p2.add_literal(2);
        auto sum1 = p2.add_instruction(migraphx::op::add{}, one, two);
        auto sum2 = p2.add_instruction(migraphx::op::add{}, x, y);
        auto sum3 = p2.add_instruction(migraphx::op::add{}, sum2, sum1);
        p2.add_instruction(pass_op{}, sum3);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_add2)
{
    migraphx::program p1;
    {
        auto x    = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y    = p1.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto one  = p1.add_literal(1);
        auto two  = p1.add_literal(2);
        auto sum1 = p1.add_instruction(migraphx::op::add{}, one, x);
        auto sum2 = p1.add_instruction(migraphx::op::add{}, two, y);
        auto sum3 = p1.add_instruction(migraphx::op::add{}, sum1, sum2);
        p1.add_instruction(pass_op{}, sum3);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x    = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y    = p2.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto one  = p2.add_literal(1);
        auto two  = p2.add_literal(2);
        auto sum1 = p2.add_instruction(migraphx::op::add{}, one, two);
        auto sum2 = p2.add_instruction(migraphx::op::add{}, x, y);
        auto sum3 = p2.add_instruction(migraphx::op::add{}, sum2, sum1);
        p2.add_instruction(pass_op{}, sum3);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_add3)
{
    migraphx::program p1;
    {
        auto x    = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto one  = p1.add_literal(1);
        auto two  = p1.add_literal(2);
        auto sum1 = p1.add_instruction(migraphx::op::add{}, one, x);
        auto sum2 = p1.add_instruction(migraphx::op::add{}, one, two);
        auto sum3 = p1.add_instruction(migraphx::op::add{}, sum1, sum2);
        p1.add_instruction(pass_op{}, sum3);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x    = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto one  = p2.add_literal(1);
        auto two  = p2.add_literal(2);
        auto sum1 = p2.add_instruction(migraphx::op::add{}, one, two);
        auto sum2 = p2.add_instruction(migraphx::op::add{}, one, sum1);
        auto sum3 = p2.add_instruction(migraphx::op::add{}, x, sum2);
        p2.add_instruction(pass_op{}, sum3);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_add_broadcast1)
{
    migraphx::shape inner{migraphx::shape::int32_type, {2}};
    migraphx::shape outer{migraphx::shape::int32_type, {1, 2, 3, 3}};
    migraphx::op::broadcast b{1, {1, 2, 3, 3}};
    migraphx::program p1;
    {
        auto x    = p1.add_parameter("x", outer);
        auto y    = p1.add_parameter("y", outer);
        auto one  = p1.add_literal({inner, {1, 1}});
        auto oneb = p1.add_instruction(b, one);
        auto two  = p1.add_literal({inner, {2, 2}});
        auto twob = p1.add_instruction(b, two);
        auto sum1 = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto sum2 = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto sum3 = p1.add_instruction(migraphx::op::add{}, sum1, sum2);
        p1.add_instruction(pass_op{}, sum3);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x     = p2.add_parameter("x", outer);
        auto y     = p2.add_parameter("y", outer);
        auto one   = p2.add_literal({inner, {1, 1}});
        auto two   = p2.add_literal({inner, {2, 2}});
        auto sum1  = p2.add_instruction(migraphx::op::add{}, one, two);
        auto sum1b = p2.add_instruction(b, sum1);
        auto sum2  = p2.add_instruction(migraphx::op::add{}, x, y);
        auto sum3  = p2.add_instruction(migraphx::op::add{}, sum2, sum1b);
        p2.add_instruction(pass_op{}, sum3);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_add_broadcast2)
{
    migraphx::shape inner{migraphx::shape::int32_type, {2}};
    migraphx::shape outer{migraphx::shape::int32_type, {1, 2, 3, 3}};
    migraphx::op::broadcast b{1, {1, 2, 3, 3}};
    auto create_program = [&] {
        migraphx::program p;
        auto x    = p.add_parameter("x", outer);
        auto y    = p.add_parameter("y", outer);
        auto one  = p.add_literal({inner, {1, 1}});
        auto oneb = p.add_instruction(b, one);
        auto two  = p.add_literal({outer, {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}});
        auto sum1 = p.add_instruction(migraphx::op::add{}, x, y);
        auto sum2 = p.add_instruction(migraphx::op::add{}, oneb, two);
        auto sum3 = p.add_instruction(migraphx::op::add{}, sum2, sum1);
        p.add_instruction(pass_op{}, sum3);
        return p;
    };
    migraphx::program p1 = create_program();
    run_pass(p1);

    migraphx::program p2 = create_program();
    EXPECT(p1 == p2);
}

// TODO: Add test case
// TEST_CASE(simplify_add4)
void simplify_add4()
{
    migraphx::program p1;
    {
        auto x    = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y    = p1.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto one  = p1.add_literal(1);
        auto two  = p1.add_literal(2);
        auto sum1 = p1.add_instruction(migraphx::op::add{}, one, x);
        auto sum2 = p1.add_instruction(migraphx::op::add{}, sum1, y);
        auto sum3 = p1.add_instruction(migraphx::op::add{}, sum2, two);
        p1.add_instruction(pass_op{}, sum3);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x    = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y    = p2.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto one  = p2.add_literal(1);
        auto two  = p2.add_literal(2);
        auto sum1 = p2.add_instruction(migraphx::op::add{}, one, two);
        auto sum2 = p2.add_instruction(migraphx::op::add{}, x, y);
        auto sum3 = p2.add_instruction(migraphx::op::add{}, sum2, sum1);
        p2.add_instruction(pass_op{}, sum3);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_mul_conv1)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::int32_type, {1, 128, 28, 28}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {256, 128, 3, 3}}));
    auto conv = p.add_instruction(migraphx::op::convolution{{1, 1}, {2, 2}, {1, 1}}, x, w);
    auto a    = p.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {256}}));
    auto b    = p.add_instruction(migraphx::op::broadcast{1, {1, 256, 14, 14}}, a);
    auto mul  = p.add_instruction(migraphx::op::mul{}, conv, b);
    p.add_instruction(pass_op{}, mul);
    EXPECT(conv->outputs().front()->name() == "mul");
    run_pass(p);
    auto new_conv =
        std::find_if(p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; });
    EXPECT(new_conv->outputs().front()->name() != "mul");
}

TEST_CASE(simplify_mul_slice_conv1)
{
    migraphx::program p1;
    {
        auto x = p1.add_parameter("x", {migraphx::shape::int32_type, {1, 1024, 17, 17}});
        auto w = p1.add_literal(
            migraphx::generate_literal({migraphx::shape::int32_type, {768, 1024, 1, 1}}));
        auto conv   = p1.add_instruction(migraphx::op::convolution{}, x, w);
        auto slice1 = p1.add_instruction(migraphx::op::slice{{1}, {0}, {384}}, conv);
        auto a   = p1.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}));
        auto b   = p1.add_instruction(migraphx::op::broadcast{1, {1, 384, 17, 17}}, a);
        auto mul = p1.add_instruction(migraphx::op::mul{}, slice1, b);
        auto slice2 = p1.add_instruction(migraphx::op::slice{{1}, {384}, {768}}, conv);
        auto add    = p1.add_instruction(migraphx::op::add{}, mul, slice2);
        p1.add_instruction(pass_op{}, add);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x = p2.add_parameter("x", {migraphx::shape::int32_type, {1, 1024, 17, 17}});
        auto w = p2.add_literal(
            migraphx::generate_literal({migraphx::shape::int32_type, {768, 1024, 1, 1}}));
        auto wslice1 = p2.add_instruction(migraphx::op::slice{{0}, {0}, {384}}, w);
        auto a   = p2.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}));
        auto b   = p2.add_instruction(migraphx::op::broadcast{0, {384, 1024, 1, 1}}, a);
        auto mul = p2.add_instruction(migraphx::op::mul{}, b, wslice1);
        auto wslice2 = p2.add_instruction(migraphx::op::slice{{0}, {384}, {768}}, w);
        auto concat  = p2.add_instruction(migraphx::op::concat{0}, mul, wslice2);
        auto conv    = p2.add_instruction(migraphx::op::convolution{}, x, concat);
        auto slice1  = p2.add_instruction(migraphx::op::slice{{1}, {0}, {384}}, conv);
        auto slice2  = p2.add_instruction(migraphx::op::slice{{1}, {384}, {768}}, conv);
        auto add     = p2.add_instruction(migraphx::op::add{}, slice1, slice2);
        p2.add_instruction(pass_op{}, add);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_mul_slice_conv_overlapping_slice)
{
    migraphx::program p1;
    {
        auto x = p1.add_parameter("x", {migraphx::shape::int32_type, {1, 1024, 17, 17}});
        auto w = p1.add_literal(
            migraphx::generate_literal({migraphx::shape::int32_type, {768, 1024, 1, 1}}));
        auto conv   = p1.add_instruction(migraphx::op::convolution{}, x, w);
        auto slice1 = p1.add_instruction(migraphx::op::slice{{1}, {0}, {384}}, conv);
        auto a   = p1.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}));
        auto b   = p1.add_instruction(migraphx::op::broadcast{1, {1, 384, 17, 17}}, a);
        auto mul = p1.add_instruction(migraphx::op::mul{}, slice1, b);
        auto slice2 = p1.add_instruction(migraphx::op::slice{{1}, {383}, {767}}, conv);
        auto add    = p1.add_instruction(migraphx::op::add{}, mul, slice2);
        p1.add_instruction(pass_op{}, add);
    }
    migraphx::program p2 = p1;
    run_pass(p1);
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_mul_slice_conv_not_all_slice)
{
    migraphx::program p1;
    {
        auto x = p1.add_parameter("x", {migraphx::shape::int32_type, {1, 1024, 17, 17}});
        auto w = p1.add_literal(
            migraphx::generate_literal({migraphx::shape::int32_type, {768, 1024, 1, 1}}));
        auto conv   = p1.add_instruction(migraphx::op::convolution{}, x, w);
        auto slice1 = p1.add_instruction(migraphx::op::slice{{1}, {0}, {384}}, conv);
        auto a   = p1.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}));
        auto b   = p1.add_instruction(migraphx::op::broadcast{1, {1, 384, 17, 17}}, a);
        auto mul = p1.add_instruction(migraphx::op::mul{}, slice1, b);
        auto c   = p1.add_literal(
            migraphx::generate_literal({migraphx::shape::int32_type, {1, 768, 17, 17}}));
        auto add    = p1.add_instruction(migraphx::op::add{}, conv, c);
        auto concat = p1.add_instruction(migraphx::op::concat{1}, mul, add);
        p1.add_instruction(pass_op{}, concat);
    }
    migraphx::program p2 = p1;
    run_pass(p1);
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_mul_add)
{
    migraphx::program p1;
    {
        auto x   = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto one = p1.add_literal(1);
        auto two = p1.add_literal(2);
        auto sum = p1.add_instruction(migraphx::op::add{}, one, x);
        auto mul = p1.add_instruction(migraphx::op::mul{}, sum, two);
        p1.add_instruction(pass_op{}, mul);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x    = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto one  = p2.add_literal(1);
        auto two  = p2.add_literal(2);
        auto mul1 = p2.add_instruction(migraphx::op::mul{}, two, x);
        auto mul2 = p2.add_instruction(migraphx::op::mul{}, two, one);
        auto sum  = p2.add_instruction(migraphx::op::add{}, mul1, mul2);
        p2.add_instruction(pass_op{}, sum);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_inner_broadcast)
{
    auto b = migraphx::op::broadcast{1, {2, 1, 4, 5}};
    migraphx::program p1;
    {
        auto x   = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y   = p1.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto xb  = p1.add_instruction(b, x);
        auto yb  = p1.add_instruction(b, y);
        auto sum = p1.add_instruction(migraphx::op::add{}, xb, yb);
        p1.add_instruction(pass_op{}, sum);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x    = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto y    = p2.add_parameter("y", {migraphx::shape::int32_type, {1}});
        auto sum  = p2.add_instruction(migraphx::op::add{}, x, y);
        auto sumb = p2.add_instruction(b, sum);
        p2.add_instruction(pass_op{}, sumb);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_add_conv1)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::float_type, {1, 128, 28, 28}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 3, 3}}));
    auto y = p.add_parameter("y", {migraphx::shape::float_type, {1, 128, 28, 28}});
    auto v =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 3, 3}}));
    auto conv1 = p.add_instruction(migraphx::op::convolution{}, x, w);
    auto conv2 = p.add_instruction(migraphx::op::convolution{}, y, v);
    auto sum   = p.add_instruction(migraphx::op::add{}, conv1, conv2);
    p.add_instruction(pass_op{}, sum);
    auto s = p.get_output_shapes().back();
    run_pass(p);
    EXPECT(s == p.get_output_shapes().back());
    EXPECT(std::count_if(
               p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; }) == 1);
}

TEST_CASE(simplify_add_conv_no_fusion_7x7_diff_strides)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::float_type, {1, 128, 14, 14}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 7, 7}}));
    auto y = p.add_parameter("y", {migraphx::shape::float_type, {1, 128, 28, 28}});
    auto v =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 7, 7}}));
    auto conv1 = p.add_instruction(migraphx::op::convolution{}, x, w);
    auto conv2 = p.add_instruction(migraphx::op::convolution{{0, 0}, {3, 3}}, y, v);
    auto sum   = p.add_instruction(migraphx::op::add{}, conv1, conv2);
    p.add_instruction(pass_op{}, sum);
    auto s = p.get_output_shapes().back();
    run_pass(p);
    EXPECT(s == p.get_output_shapes().back());
    // No fusion
    EXPECT(std::count_if(
               p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; }) == 2);
}

TEST_CASE(simplify_add_conv_1x1_diff_strides1)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::float_type, {1, 128, 14, 14}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto y = p.add_parameter("y", {migraphx::shape::float_type, {1, 128, 28, 28}});
    auto v =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto conv1 = p.add_instruction(migraphx::op::convolution{}, x, w);
    auto conv2 = p.add_instruction(migraphx::op::convolution{{0, 0}, {2, 2}}, y, v);
    auto sum   = p.add_instruction(migraphx::op::add{}, conv1, conv2);
    p.add_instruction(pass_op{}, sum);
    auto s = p.get_output_shapes().back();
    run_pass(p);
    EXPECT(s == p.get_output_shapes().back());
    EXPECT(std::count_if(
               p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; }) == 1);
}

TEST_CASE(simplify_add_conv_1x1_diff_strides2)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::float_type, {1, 128, 28, 28}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto y = p.add_parameter("y", {migraphx::shape::float_type, {1, 128, 14, 14}});
    auto v =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto conv1 = p.add_instruction(migraphx::op::convolution{{0, 0}, {2, 2}}, x, w);
    auto conv2 = p.add_instruction(migraphx::op::convolution{}, y, v);
    auto sum   = p.add_instruction(migraphx::op::add{}, conv1, conv2);
    p.add_instruction(pass_op{}, sum);
    auto s = p.get_output_shapes().back();
    run_pass(p);
    EXPECT(s == p.get_output_shapes().back());
    EXPECT(std::count_if(
               p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; }) == 1);
}

TEST_CASE(simplify_add_conv_1x1_diff_strides_odd)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::float_type, {1, 54, 83, 83}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {54, 54, 1, 1}}));
    auto y = p.add_parameter("y", {migraphx::shape::float_type, {1, 54, 165, 165}});
    auto v =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {54, 54, 1, 1}}));
    auto conv1 = p.add_instruction(migraphx::op::convolution{}, x, w);
    auto conv2 = p.add_instruction(migraphx::op::convolution{{0, 0}, {2, 2}}, y, v);
    auto sum   = p.add_instruction(migraphx::op::add{}, conv1, conv2);
    p.add_instruction(pass_op{}, sum);
    auto s = p.get_output_shapes().back();
    run_pass(p);
    EXPECT(s == p.get_output_shapes().back());
    EXPECT(std::count_if(
               p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; }) == 1);
}

TEST_CASE(simplify_add_conv_no_fusion_asymetrical_strides1)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::float_type, {1, 128, 28, 14}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto y = p.add_parameter("y", {migraphx::shape::float_type, {1, 128, 14, 14}});
    auto v =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto conv1 = p.add_instruction(migraphx::op::convolution{{0, 0}, {2, 1}}, x, w);
    auto conv2 = p.add_instruction(migraphx::op::convolution{}, y, v);
    auto sum   = p.add_instruction(migraphx::op::add{}, conv1, conv2);
    p.add_instruction(pass_op{}, sum);
    auto s = p.get_output_shapes().back();
    run_pass(p);
    EXPECT(s == p.get_output_shapes().back());
    // No fusion
    EXPECT(std::count_if(
               p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; }) == 2);
}

TEST_CASE(simplify_add_conv_no_fusion_asymetrical_strides2)
{
    migraphx::program p;
    auto x = p.add_parameter("x", {migraphx::shape::float_type, {1, 128, 14, 14}});
    auto w =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto y = p.add_parameter("y", {migraphx::shape::float_type, {1, 128, 28, 14}});
    auto v =
        p.add_literal(migraphx::generate_literal({migraphx::shape::float_type, {256, 128, 1, 1}}));
    auto conv1 = p.add_instruction(migraphx::op::convolution{}, x, w);
    auto conv2 = p.add_instruction(migraphx::op::convolution{{0, 0}, {2, 1}}, y, v);
    auto sum   = p.add_instruction(migraphx::op::add{}, conv1, conv2);
    p.add_instruction(pass_op{}, sum);
    auto s = p.get_output_shapes().back();
    run_pass(p);
    EXPECT(s == p.get_output_shapes().back());
    // No fusion
    EXPECT(std::count_if(
               p.begin(), p.end(), [](auto&& ins) { return ins.name() == "convolution"; }) == 2);
}

TEST_CASE(simplify_concat_add_relu)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {1}};
    migraphx::program p1;
    {
        auto x      = p1.add_parameter("x", s);
        auto y      = p1.add_parameter("y", s);
        auto one    = p1.add_literal({s, {1}});
        auto two    = p1.add_literal({s, {2}});
        auto sum1   = p1.add_instruction(migraphx::op::add{}, x, one);
        auto relu1  = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2   = p1.add_instruction(migraphx::op::add{}, y, two);
        auto relu2  = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto concat = p1.add_instruction(migraphx::op::concat{0}, relu1, relu2);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x       = p2.add_parameter("x", s);
        auto y       = p2.add_parameter("y", s);
        auto one     = p2.add_literal({s, {1}});
        auto two     = p2.add_literal({s, {2}});
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, x, y);
        auto concat2 = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto sum     = p2.add_instruction(migraphx::op::add{}, concat1, concat2);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum);
        p2.add_instruction(pass_op{}, relu);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_concat_add_relu_partial)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {1}};
    migraphx::program p1;
    {
        auto x      = p1.add_parameter("x", s);
        auto y      = p1.add_parameter("y", s);
        auto one    = p1.add_literal({s, {1}});
        auto two    = p1.add_literal({s, {2}});
        auto sum1   = p1.add_instruction(migraphx::op::add{}, x, one);
        auto relu1  = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2   = p1.add_instruction(migraphx::op::add{}, y, two);
        auto relu2  = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto sum3   = p1.add_instruction(migraphx::op::add{}, x, y);
        auto concat = p1.add_instruction(migraphx::op::concat{0}, sum3, relu1, relu2);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x       = p2.add_parameter("x", s);
        auto y       = p2.add_parameter("y", s);
        auto one     = p2.add_literal({s, {1}});
        auto two     = p2.add_literal({s, {2}});
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, x, y);
        auto concat2 = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto sum1    = p2.add_instruction(migraphx::op::add{}, concat1, concat2);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2    = p2.add_instruction(migraphx::op::add{}, x, y);
        auto concat  = p2.add_instruction(migraphx::op::concat{0}, sum2, relu);
        p2.add_instruction(pass_op{}, concat);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_concat_add_relu_partial_broadcast)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {2, 1, 4, 5}};
    migraphx::program p1;
    {
        auto b      = migraphx::op::broadcast{1, {2, 1, 4, 5}};
        auto x      = p1.add_parameter("x", s);
        auto y      = p1.add_parameter("y", s);
        auto one    = p1.add_literal(1);
        auto oneb   = p1.add_instruction(b, one);
        auto two    = p1.add_literal(2);
        auto twob   = p1.add_instruction(b, two);
        auto sum    = p1.add_instruction(migraphx::op::add{}, x, y);
        auto concat = p1.add_instruction(migraphx::op::concat{1}, sum, oneb, twob);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b       = migraphx::op::broadcast{1, {2, 2, 4, 5}};
        auto x       = p2.add_parameter("x", s);
        auto y       = p2.add_parameter("y", s);
        auto one     = p2.add_literal(1);
        auto two     = p2.add_literal(2);
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto concatb = p2.add_instruction(b, concat1);
        auto sum     = p2.add_instruction(migraphx::op::add{}, x, y);
        auto concat2 = p2.add_instruction(migraphx::op::concat{1}, sum, concatb);
        p2.add_instruction(pass_op{}, concat2);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_concat_add_relu_broadcast_different_axis)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {2, 1, 4, 5}};
    migraphx::program p1;
    {
        auto b      = migraphx::op::broadcast{1, {2, 1, 4, 5}};
        auto x      = p1.add_parameter("x", s);
        auto y      = p1.add_parameter("y", s);
        auto one    = p1.add_literal(1);
        auto oneb   = p1.add_instruction(b, one);
        auto two    = p1.add_literal(2);
        auto twob   = p1.add_instruction(b, two);
        auto sum1   = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1  = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2   = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2  = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto concat = p1.add_instruction(migraphx::op::concat{1}, relu1, relu2);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b        = migraphx::op::broadcast{1, {2, 2, 4, 5}};
        auto x        = p2.add_parameter("x", s);
        auto y        = p2.add_parameter("y", s);
        auto one      = p2.add_literal(1);
        auto two      = p2.add_literal(2);
        auto concat1  = p2.add_instruction(migraphx::op::concat{1}, x, y);
        auto concat2  = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto concat2b = p2.add_instruction(b, concat2);
        auto sum      = p2.add_instruction(migraphx::op::add{}, concat1, concat2b);
        auto relu     = p2.add_instruction(migraphx::op::relu{}, sum);
        p2.add_instruction(pass_op{}, relu);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_concat_add_relu_broadcast_same_axis)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {2, 1, 4, 5}};
    migraphx::program p1;
    {
        auto b      = migraphx::op::broadcast{1, {2, 1, 4, 5}};
        auto x      = p1.add_parameter("x", s);
        auto y      = p1.add_parameter("y", s);
        auto one    = p1.add_literal(1);
        auto oneb   = p1.add_instruction(b, one);
        auto two    = p1.add_literal(2);
        auto twob   = p1.add_instruction(b, two);
        auto sum1   = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1  = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2   = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2  = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto concat = p1.add_instruction(migraphx::op::concat{0}, relu1, relu2);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b       = migraphx::op::broadcast{1, {2, 1, 4, 5}};
        auto x       = p2.add_parameter("x", s);
        auto y       = p2.add_parameter("y", s);
        auto one     = p2.add_literal(1);
        auto oneb    = p2.add_instruction(b, one);
        auto two     = p2.add_literal(2);
        auto twob    = p2.add_instruction(b, two);
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, x, y);
        auto concat2 = p2.add_instruction(migraphx::op::concat{0}, oneb, twob);
        auto sum     = p2.add_instruction(migraphx::op::add{}, concat1, concat2);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum);
        p2.add_instruction(pass_op{}, relu);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_div_const)
{
    migraphx::program p1;
    {
        auto x   = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto two = p1.add_literal(2);
        p1.add_instruction(migraphx::op::div{}, x, two);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x     = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto two   = p2.add_literal(2);
        auto recip = p2.insert_instruction(std::next(two), migraphx::op::recip{}, two);
        p2.add_instruction(migraphx::op::mul{}, x, recip);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_sub_const)
{
    migraphx::program p1;
    {
        auto x   = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto two = p1.add_literal(2);
        p1.add_instruction(migraphx::op::sub{}, x, two);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x   = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto two = p2.add_literal(2);
        auto neg = p2.insert_instruction(std::next(two), migraphx::op::neg{}, two);
        p2.add_instruction(migraphx::op::add{}, x, neg);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_rsqrt)
{
    migraphx::program p1;
    {
        auto x    = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto sqrt = p1.add_instruction(migraphx::op::sqrt{}, x);
        p1.add_instruction(migraphx::op::recip{}, sqrt);
    }
    run_pass(p1);

    migraphx::program p2;
    {

        auto x = p2.add_parameter("x", {migraphx::shape::int32_type, {1}});
        p2.add_instruction(migraphx::op::rsqrt{}, x);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_rsqrt_multi_use)
{
    migraphx::program p1;
    {
        auto x     = p1.add_parameter("x", {migraphx::shape::int32_type, {1}});
        auto sqrt  = p1.add_instruction(migraphx::op::sqrt{}, x);
        auto add   = p1.add_instruction(migraphx::op::add{}, sqrt, sqrt);
        auto rsqrt = p1.add_instruction(migraphx::op::recip{}, sqrt);
        p1.add_instruction(migraphx::op::add{}, rsqrt, add);
    }
    migraphx::program p2{p1};

    run_pass(p1);
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_slice_concat)
{
    auto s = migraphx::shape{migraphx::shape::float_type, {256}};

    migraphx::program p1;
    {
        auto x       = p1.add_parameter("x", s);
        auto y       = p1.add_parameter("y", s);
        auto xslice1 = p1.add_instruction(migraphx::op::slice{{0}, {0}, {128}}, x);
        auto xslice2 = p1.add_instruction(migraphx::op::slice{{0}, {128}, {256}}, x);
        auto yslice1 = p1.add_instruction(migraphx::op::slice{{0}, {0}, {128}}, y);
        auto yslice2 = p1.add_instruction(migraphx::op::slice{{0}, {128}, {256}}, y);
        auto concat =
            p1.add_instruction(migraphx::op::concat{0}, xslice1, xslice2, yslice1, yslice2);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x      = p2.add_parameter("x", s);
        auto y      = p2.add_parameter("y", s);
        auto concat = p2.add_instruction(migraphx::op::concat{0}, x, y);
        p2.add_instruction(pass_op{}, concat);
    }
    EXPECT(p1 == p2);
}

TEST_CASE(simplify_slice_concat_non_uniform)
{
    auto s = migraphx::shape{migraphx::shape::float_type, {256}};

    migraphx::program p1;
    {
        auto x       = p1.add_parameter("x", s);
        auto y       = p1.add_parameter("y", s);
        auto xslice1 = p1.add_instruction(migraphx::op::slice{{0}, {0}, {64}}, x);
        auto xslice2 = p1.add_instruction(migraphx::op::slice{{0}, {64}, {192}}, x);
        auto xslice3 = p1.add_instruction(migraphx::op::slice{{0}, {192}, {256}}, x);
        auto yslice1 = p1.add_instruction(migraphx::op::slice{{0}, {0}, {64}}, y);
        auto yslice2 = p1.add_instruction(migraphx::op::slice{{0}, {64}, {192}}, y);
        auto yslice3 = p1.add_instruction(migraphx::op::slice{{0}, {192}, {256}}, y);
        auto concat  = p1.add_instruction(
            migraphx::op::concat{0}, xslice1, xslice2, xslice3, yslice1, yslice2, yslice3);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x      = p2.add_parameter("x", s);
        auto y      = p2.add_parameter("y", s);
        auto concat = p2.add_instruction(migraphx::op::concat{0}, x, y);
        p2.add_instruction(pass_op{}, concat);
    }

    EXPECT(p1 == p2);
}

TEST_CASE(simplify_slice_concat_flipped)
{
    auto s = migraphx::shape{migraphx::shape::float_type, {256}};

    migraphx::program p1;
    {
        auto x       = p1.add_parameter("x", s);
        auto y       = p1.add_parameter("y", s);
        auto xslice1 = p1.add_instruction(migraphx::op::slice{{0}, {0}, {64}}, x);
        auto xslice2 = p1.add_instruction(migraphx::op::slice{{0}, {192}, {256}}, x);
        auto xslice3 = p1.add_instruction(migraphx::op::slice{{0}, {64}, {192}}, x);
        auto yslice1 = p1.add_instruction(migraphx::op::slice{{0}, {0}, {64}}, y);
        auto yslice2 = p1.add_instruction(migraphx::op::slice{{0}, {192}, {256}}, y);
        auto yslice3 = p1.add_instruction(migraphx::op::slice{{0}, {64}, {192}}, y);
        auto concat  = p1.add_instruction(
            migraphx::op::concat{0}, xslice1, xslice2, xslice3, yslice1, yslice2, yslice3);
        p1.add_instruction(pass_op{}, concat);
    }
    migraphx::program p2 = p1;
    run_pass(p1);

    EXPECT(p1 == p2);
}

TEST_CASE(simplify_split_add_relu)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4}};
    migraphx::program p1;
    {
        auto b     = migraphx::op::broadcast{1, {3, 1, 4}};
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto one   = p1.add_literal(1);
        auto oneb  = p1.add_instruction(b, one);
        auto two   = p1.add_literal(2);
        auto twob  = p1.add_instruction(b, two);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1 = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2 = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto add   = p1.add_instruction(migraphx::op::add{}, relu1, relu2);
        p1.add_instruction(pass_op{}, add);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b       = migraphx::op::broadcast{1, {3, 2, 4}};
        auto input   = p2.add_parameter("input", s);
        auto one     = p2.add_literal(1);
        auto two     = p2.add_literal(2);
        auto concat  = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto concatb = p2.add_instruction(b, concat);
        auto sum     = p2.add_instruction(migraphx::op::add{}, input, concatb);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum);
        auto x       = p2.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, relu);
        auto y       = p2.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, relu);
        auto add     = p2.add_instruction(migraphx::op::add{}, x, y);
        p2.add_instruction(pass_op{}, add);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_split_add_relu_reshape)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4}};
    migraphx::program p1;
    {
        auto b        = migraphx::op::broadcast{1, {3, 1, 4}};
        auto r        = migraphx::op::reshape{{3, 4}};
        auto input    = p1.add_parameter("input", s);
        auto x        = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y        = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto one      = p1.add_literal(1);
        auto oneb     = p1.add_instruction(b, one);
        auto two      = p1.add_literal(2);
        auto twob     = p1.add_instruction(b, two);
        auto sum1     = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1    = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto reshape1 = p1.add_instruction(r, relu1);
        auto sum2     = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2    = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto reshape2 = p1.add_instruction(r, relu2);
        auto add      = p1.add_instruction(migraphx::op::add{}, reshape1, reshape2);
        p1.add_instruction(pass_op{}, add);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b       = migraphx::op::broadcast{1, {3, 2, 4}};
        auto input   = p2.add_parameter("input", s);
        auto one     = p2.add_literal(1);
        auto two     = p2.add_literal(2);
        auto concat  = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto concatb = p2.add_instruction(b, concat);
        auto sum     = p2.add_instruction(migraphx::op::add{}, input, concatb);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum);
        auto rsp     = p2.add_instruction(migraphx::op::reshape{{3, 8}}, relu);
        auto slc1    = p2.add_instruction(migraphx::op::slice{{1}, {0}, {4}}, rsp);
        auto slc2    = p2.add_instruction(migraphx::op::slice{{1}, {4}, {8}}, rsp);
        auto add     = p2.add_instruction(migraphx::op::add{}, slc1, slc2);
        p2.add_instruction(pass_op{}, add);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_slice_different_axis)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4, 2}};
    migraphx::program p1;
    {
        auto r        = migraphx::op::reshape{{3, 2, 4}};
        auto input    = p1.add_parameter("input", s);
        auto x        = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y        = p1.add_instruction(migraphx::op::slice{{3}, {0}, {1}}, input);
        auto one      = p1.add_literal(1);
        auto oneb     = p1.add_instruction(migraphx::op::broadcast{1, {3, 1, 4, 2}}, one);
        auto two      = p1.add_literal(2);
        auto twob     = p1.add_instruction(migraphx::op::broadcast{3, {3, 2, 4, 1}}, two);
        auto sum1     = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1    = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto reshape1 = p1.add_instruction(r, relu1);
        auto sum2     = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2    = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto reshape2 = p1.add_instruction(r, relu2);
        auto add      = p1.add_instruction(migraphx::op::add{}, reshape1, reshape2);
        p1.add_instruction(pass_op{}, add);
    }
    migraphx::program p2 = p1;
    run_pass(p1);

    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_slice_missing_begining_slice)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 3, 4}};
    migraphx::program p1;
    {
        auto b     = migraphx::op::broadcast{1, {3, 1, 4}};
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1}, {2}, {3}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto one   = p1.add_literal(1);
        auto oneb  = p1.add_instruction(b, one);
        auto two   = p1.add_literal(2);
        auto twob  = p1.add_instruction(b, two);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1 = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2 = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto add   = p1.add_instruction(migraphx::op::add{}, relu1, relu2);
        p1.add_instruction(pass_op{}, add);
    }
    migraphx::program p2 = p1;
    run_pass(p1);

    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_slice_missing_middle_slice)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 3, 4}};
    migraphx::program p1;
    {
        auto b     = migraphx::op::broadcast{1, {3, 1, 4}};
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1}, {2}, {3}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto one   = p1.add_literal(1);
        auto oneb  = p1.add_instruction(b, one);
        auto two   = p1.add_literal(2);
        auto twob  = p1.add_instruction(b, two);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1 = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2 = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto add   = p1.add_instruction(migraphx::op::add{}, relu1, relu2);
        p1.add_instruction(pass_op{}, add);
    }
    migraphx::program p2 = p1;
    run_pass(p1);

    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_slice_missing_end_slice)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 3, 4}};
    migraphx::program p1;
    {
        auto b     = migraphx::op::broadcast{1, {3, 1, 4}};
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto one   = p1.add_literal(1);
        auto oneb  = p1.add_instruction(b, one);
        auto two   = p1.add_literal(2);
        auto twob  = p1.add_instruction(b, two);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1 = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2 = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto add   = p1.add_instruction(migraphx::op::add{}, relu1, relu2);
        p1.add_instruction(pass_op{}, add);
    }
    migraphx::program p2 = p1;
    run_pass(p1);

    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_split_add_relu_concat_same_axis)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4}};
    migraphx::program p1;
    {
        auto b      = migraphx::op::broadcast{1, {3, 1, 4}};
        auto input  = p1.add_parameter("input", s);
        auto x      = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y      = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto one    = p1.add_literal(1);
        auto oneb   = p1.add_instruction(b, one);
        auto two    = p1.add_literal(2);
        auto twob   = p1.add_instruction(b, two);
        auto sum1   = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1  = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2   = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2  = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto concat = p1.add_instruction(migraphx::op::concat{1}, relu1, relu2);
        p1.add_instruction(pass_op{}, concat);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b       = migraphx::op::broadcast{1, {3, 2, 4}};
        auto input   = p2.add_parameter("input", s);
        auto one     = p2.add_literal(1);
        auto two     = p2.add_literal(2);
        auto concat  = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto concatb = p2.add_instruction(b, concat);
        auto sum     = p2.add_instruction(migraphx::op::add{}, input, concatb);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum);
        p2.add_instruction(pass_op{}, relu);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_split_add_relu_multi_axes)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4, 6}};
    migraphx::program p1;
    {
        auto b     = migraphx::op::broadcast{1, {3, 1, 4, 3}};
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1, 3}, {0, 0}, {1, 3}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1, 3}, {1, 3}, {2, 6}}, input);
        auto one   = p1.add_literal(1);
        auto oneb  = p1.add_instruction(b, one);
        auto two   = p1.add_literal(2);
        auto twob  = p1.add_instruction(b, two);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1 = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2 = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto add   = p1.add_instruction(migraphx::op::add{}, relu1, relu2);
        p1.add_instruction(pass_op{}, add);
    }
    migraphx::program p2 = p1;
    run_pass(p1);
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_split_add_relu_used_multiple_split1)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4}};
    migraphx::program p1;
    {
        auto b     = migraphx::op::broadcast{1, {3, 1, 4}};
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto one   = p1.add_literal(1);
        auto oneb  = p1.add_instruction(b, one);
        auto two   = p1.add_literal(2);
        auto twob  = p1.add_instruction(b, two);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1 = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2 = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto add1  = p1.add_instruction(migraphx::op::add{}, relu1, relu2);
        auto add2  = p1.add_instruction(migraphx::op::add{}, x, add1);
        p1.add_instruction(pass_op{}, add2);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b       = migraphx::op::broadcast{1, {3, 2, 4}};
        auto input   = p2.add_parameter("input", s);
        auto slice   = p2.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto one     = p2.add_literal(1);
        auto two     = p2.add_literal(2);
        auto concat  = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto concatb = p2.add_instruction(b, concat);
        auto sum     = p2.add_instruction(migraphx::op::add{}, input, concatb);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum);
        auto x       = p2.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, relu);
        auto y       = p2.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, relu);
        auto add1    = p2.add_instruction(migraphx::op::add{}, x, y);
        auto add2    = p2.add_instruction(migraphx::op::add{}, slice, add1);
        p2.add_instruction(pass_op{}, add2);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_split_add_relu_used_multiple_split2)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4}};
    migraphx::program p1;
    {
        auto b     = migraphx::op::broadcast{1, {3, 1, 4}};
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto z     = p1.add_instruction(migraphx::op::relu{}, x);
        auto one   = p1.add_literal(1);
        auto oneb  = p1.add_instruction(b, one);
        auto two   = p1.add_literal(2);
        auto twob  = p1.add_instruction(b, two);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, x, oneb);
        auto relu1 = p1.add_instruction(migraphx::op::relu{}, sum1);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, y, twob);
        auto relu2 = p1.add_instruction(migraphx::op::relu{}, sum2);
        auto add1  = p1.add_instruction(migraphx::op::add{}, relu1, relu2);
        auto add2  = p1.add_instruction(migraphx::op::add{}, z, add1);
        p1.add_instruction(pass_op{}, add2);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto b       = migraphx::op::broadcast{1, {3, 2, 4}};
        auto input   = p2.add_parameter("input", s);
        auto slice   = p2.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto z       = p2.add_instruction(migraphx::op::relu{}, slice);
        auto one     = p2.add_literal(1);
        auto two     = p2.add_literal(2);
        auto concat  = p2.add_instruction(migraphx::op::concat{0}, one, two);
        auto concatb = p2.add_instruction(b, concat);
        auto sum     = p2.add_instruction(migraphx::op::add{}, input, concatb);
        auto relu    = p2.add_instruction(migraphx::op::relu{}, sum);
        auto x       = p2.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, relu);
        auto y       = p2.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, relu);
        auto add1    = p2.add_instruction(migraphx::op::add{}, x, y);
        auto add2    = p2.add_instruction(migraphx::op::add{}, z, add1);
        p2.add_instruction(pass_op{}, add2);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_split_between_add)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 4}};
    migraphx::program p1;
    {
        auto input = p1.add_parameter("input", s);
        auto x     = p1.add_instruction(migraphx::op::slice{{1}, {0}, {1}}, input);
        auto y     = p1.add_instruction(migraphx::op::slice{{1}, {1}, {2}}, input);
        auto sum   = p1.add_instruction(migraphx::op::add{}, x, y);
        p1.add_instruction(pass_op{}, sum);
    }
    migraphx::program p2 = p1;
    run_pass(p1);
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_dot_horiz)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 2}};
    migraphx::program p1;
    {
        auto input = p1.add_parameter("input", s);
        auto a     = p1.add_literal(migraphx::generate_literal(s, 0));
        auto b     = p1.add_literal(migraphx::generate_literal(s, 1));
        auto x     = p1.add_instruction(migraphx::op::dot{}, input, a);
        auto y     = p1.add_instruction(migraphx::op::dot{}, input, b);
        auto sum   = p1.add_instruction(migraphx::op::add{}, x, y);
        p1.add_instruction(pass_op{}, sum);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto input  = p2.add_parameter("input", s);
        auto a      = p2.add_literal(migraphx::generate_literal(s, 0));
        auto b      = p2.add_literal(migraphx::generate_literal(s, 1));
        auto concat = p2.add_instruction(migraphx::op::concat{2}, a, b);
        auto dot    = p2.add_instruction(migraphx::op::dot{}, input, concat);
        auto x      = p2.add_instruction(migraphx::op::slice{{2}, {0}, {2}}, dot);
        auto y      = p2.add_instruction(migraphx::op::slice{{2}, {2}, {4}}, dot);
        auto sum    = p2.add_instruction(migraphx::op::add{}, x, y);
        p2.add_instruction(pass_op{}, sum);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_dot_horiz_same_constant)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 2}};
    migraphx::program p1;
    {
        auto input = p1.add_parameter("input", s);
        auto a     = p1.add_literal(migraphx::generate_literal(s, 0));
        auto x     = p1.add_instruction(migraphx::op::dot{}, input, a);
        auto y     = p1.add_instruction(migraphx::op::dot{}, input, a);
        auto sum   = p1.add_instruction(migraphx::op::add{}, x, y);
        p1.add_instruction(pass_op{}, sum);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto input  = p2.add_parameter("input", s);
        auto a      = p2.add_literal(migraphx::generate_literal(s, 0));
        auto concat = p2.add_instruction(migraphx::op::concat{2}, a, a);
        auto dot    = p2.add_instruction(migraphx::op::dot{}, input, concat);
        auto x      = p2.add_instruction(migraphx::op::slice{{2}, {0}, {2}}, dot);
        auto y      = p2.add_instruction(migraphx::op::slice{{2}, {2}, {4}}, dot);
        auto sum    = p2.add_instruction(migraphx::op::add{}, x, y);
        p2.add_instruction(pass_op{}, sum);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_dot_horiz_flipped)
{
    auto s = migraphx::shape{migraphx::shape::int32_type, {3, 2, 2}};
    migraphx::program p1;
    {
        auto input = p1.add_parameter("input", s);
        auto a     = p1.add_literal(migraphx::generate_literal(s, 0));
        auto b     = p1.add_literal(migraphx::generate_literal(s, 1));
        auto x     = p1.add_instruction(migraphx::op::dot{}, input, a);
        auto y     = p1.add_instruction(migraphx::op::dot{}, b, input);
        auto sum   = p1.add_instruction(migraphx::op::add{}, x, y);
        p1.add_instruction(pass_op{}, sum);
    }

    migraphx::program p2 = p1;
    run_pass(p1);
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_conv_horiz)
{
    auto s  = migraphx::shape{migraphx::shape::int32_type, {8, 3, 64, 64}};
    auto ws = migraphx::shape{migraphx::shape::int32_type, {12, 3, 3, 3}};
    migraphx::program p1;
    {
        auto input = p1.add_parameter("input", s);
        auto a     = p1.add_literal(migraphx::generate_literal(ws, 0));
        auto b     = p1.add_literal(migraphx::generate_literal(ws, 1));
        auto x     = p1.add_instruction(migraphx::op::convolution{}, input, a);
        auto y     = p1.add_instruction(migraphx::op::convolution{}, input, b);
        auto sum   = p1.add_instruction(migraphx::op::add{}, x, y);
        p1.add_instruction(pass_op{}, sum);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto input  = p2.add_parameter("input", s);
        auto a      = p2.add_literal(migraphx::generate_literal(ws, 0));
        auto b      = p2.add_literal(migraphx::generate_literal(ws, 1));
        auto concat = p2.add_instruction(migraphx::op::concat{0}, a, b);
        auto conv   = p2.add_instruction(migraphx::op::convolution{}, input, concat);
        auto x      = p2.add_instruction(migraphx::op::slice{{1}, {0}, {12}}, conv);
        auto y      = p2.add_instruction(migraphx::op::slice{{1}, {12}, {24}}, conv);
        auto sum    = p2.add_instruction(migraphx::op::add{}, x, y);
        p2.add_instruction(pass_op{}, sum);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_group_conv_horiz)
{
    auto s  = migraphx::shape{migraphx::shape::int32_type, {1, 32, 111, 111}};
    auto ws = migraphx::shape{migraphx::shape::int32_type, {32, 1, 7, 7}};
    migraphx::program p1;
    {
        auto x  = p1.add_parameter("x", s);
        auto w1 = p1.add_literal(migraphx::generate_literal(ws, 1));
        auto w2 = p1.add_literal(migraphx::generate_literal(ws, 2));
        auto conv1 =
            p1.add_instruction(migraphx::op::convolution{{3, 3}, {2, 2}, {1, 1}, 32}, x, w1);
        auto conv2 =
            p1.add_instruction(migraphx::op::convolution{{3, 3}, {2, 2}, {1, 1}, 32}, x, w2);
        p1.add_instruction(pass_op{}, conv1, conv2);
    }
    migraphx::program p2 = p1;
    run_pass(p1);

    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_conv_horiz_grouped)
{
    auto s   = migraphx::shape{migraphx::shape::int32_type, {8, 6, 64, 64}};
    auto ws1 = migraphx::shape{migraphx::shape::int32_type, {6, 6, 3, 3}};
    auto ws2 = migraphx::shape{migraphx::shape::int32_type, {8, 6, 64, 64}};
    migraphx::program p1;
    {
        auto input = p1.add_parameter("input", s);
        auto a     = p1.add_literal(migraphx::generate_literal(ws1, 0));
        auto b     = p1.add_literal(migraphx::generate_literal(ws1, 1));
        auto c     = p1.add_literal(migraphx::generate_literal(ws2, 2));
        auto d     = p1.add_literal(migraphx::generate_literal(ws2, 3));
        auto convx = p1.add_instruction(migraphx::op::convolution{{1, 1}}, input, a);
        auto convy = p1.add_instruction(migraphx::op::convolution{{1, 1}}, input, b);
        auto dotx  = p1.add_instruction(migraphx::op::dot{}, input, c);
        auto doty  = p1.add_instruction(migraphx::op::dot{}, input, d);
        auto sum1  = p1.add_instruction(migraphx::op::add{}, convx, convy);
        auto sum2  = p1.add_instruction(migraphx::op::add{}, dotx, doty);
        auto sum3  = p1.add_instruction(migraphx::op::add{}, sum1, sum2);

        p1.add_instruction(pass_op{}, sum3);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto input   = p2.add_parameter("input", s);
        auto a       = p2.add_literal(migraphx::generate_literal(ws1, 0));
        auto b       = p2.add_literal(migraphx::generate_literal(ws1, 1));
        auto c       = p2.add_literal(migraphx::generate_literal(ws2, 2));
        auto d       = p2.add_literal(migraphx::generate_literal(ws2, 3));
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, a, b);
        auto concat2 = p2.add_instruction(migraphx::op::concat{3}, c, d);
        auto conv    = p2.add_instruction(migraphx::op::convolution{{1, 1}}, input, concat1);
        auto convx   = p2.add_instruction(migraphx::op::slice{{1}, {0}, {6}}, conv);
        auto convy   = p2.add_instruction(migraphx::op::slice{{1}, {6}, {12}}, conv);
        auto sum1    = p2.add_instruction(migraphx::op::add{}, convx, convy);
        auto dot     = p2.add_instruction(migraphx::op::dot{}, input, concat2);
        auto dotx    = p2.add_instruction(migraphx::op::slice{{3}, {0}, {64}}, dot);
        auto doty    = p2.add_instruction(migraphx::op::slice{{3}, {64}, {128}}, dot);
        auto sum2    = p2.add_instruction(migraphx::op::add{}, dotx, doty);
        auto sum3    = p2.add_instruction(migraphx::op::add{}, sum1, sum2);
        p2.add_instruction(pass_op{}, sum3);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_conv_horiz_grouped_extra1)
{
    auto s   = migraphx::shape{migraphx::shape::int32_type, {8, 6, 64, 64}};
    auto ws1 = migraphx::shape{migraphx::shape::int32_type, {6, 6, 3, 3}};
    auto ws2 = migraphx::shape{migraphx::shape::int32_type, {8, 6, 64, 64}};
    migraphx::program p1;
    {
        auto input   = p1.add_parameter("input", s);
        auto a       = p1.add_literal(migraphx::generate_literal(ws1, 0));
        auto b       = p1.add_literal(migraphx::generate_literal(ws1, 1));
        auto c       = p1.add_literal(migraphx::generate_literal(ws2, 2));
        auto d       = p1.add_literal(migraphx::generate_literal(ws2, 3));
        auto e       = p1.add_literal(migraphx::generate_literal(s, 4));
        auto convx   = p1.add_instruction(migraphx::op::convolution{{1, 1}}, input, a);
        auto convy   = p1.add_instruction(migraphx::op::convolution{{1, 1}}, input, b);
        auto dotx    = p1.add_instruction(migraphx::op::dot{}, input, c);
        auto doty    = p1.add_instruction(migraphx::op::dot{}, input, d);
        auto sqdiffx = p1.add_instruction(migraphx::op::sqdiff{}, input, e);
        auto sum1    = p1.add_instruction(migraphx::op::add{}, convx, convy);
        auto sum2    = p1.add_instruction(migraphx::op::add{}, dotx, doty);
        auto sum3    = sqdiffx;
        auto sum4    = p1.add_instruction(migraphx::op::add{}, sum1, sum2);
        auto sum5    = p1.add_instruction(migraphx::op::add{}, sum4, sum3);
        p1.add_instruction(pass_op{}, sum5);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto input   = p2.add_parameter("input", s);
        auto a       = p2.add_literal(migraphx::generate_literal(ws1, 0));
        auto b       = p2.add_literal(migraphx::generate_literal(ws1, 1));
        auto c       = p2.add_literal(migraphx::generate_literal(ws2, 2));
        auto d       = p2.add_literal(migraphx::generate_literal(ws2, 3));
        auto e       = p2.add_literal(migraphx::generate_literal(s, 4));
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, a, b);
        auto concat2 = p2.add_instruction(migraphx::op::concat{3}, c, d);
        auto conv    = p2.add_instruction(migraphx::op::convolution{{1, 1}}, input, concat1);
        auto convx   = p2.add_instruction(migraphx::op::slice{{1}, {0}, {6}}, conv);
        auto convy   = p2.add_instruction(migraphx::op::slice{{1}, {6}, {12}}, conv);
        auto sum1    = p2.add_instruction(migraphx::op::add{}, convx, convy);
        auto dot     = p2.add_instruction(migraphx::op::dot{}, input, concat2);
        auto dotx    = p2.add_instruction(migraphx::op::slice{{3}, {0}, {64}}, dot);
        auto doty    = p2.add_instruction(migraphx::op::slice{{3}, {64}, {128}}, dot);
        auto sum2    = p2.add_instruction(migraphx::op::add{}, dotx, doty);
        auto sqdiffx = p2.add_instruction(migraphx::op::sqdiff{}, input, e);
        auto sum3    = sqdiffx;
        auto sum4    = p2.add_instruction(migraphx::op::add{}, sum1, sum2);
        auto sum5    = p2.add_instruction(migraphx::op::add{}, sum4, sum3);
        p2.add_instruction(pass_op{}, sum5);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_conv_horiz_grouped_extra2)
{
    auto s   = migraphx::shape{migraphx::shape::int32_type, {8, 6, 64, 64}};
    auto ws1 = migraphx::shape{migraphx::shape::int32_type, {6, 6, 3, 3}};
    auto ws2 = migraphx::shape{migraphx::shape::int32_type, {8, 6, 64, 64}};
    migraphx::program p1;
    {
        auto input   = p1.add_parameter("input", s);
        auto a       = p1.add_literal(migraphx::generate_literal(ws1, 0));
        auto b       = p1.add_literal(migraphx::generate_literal(ws1, 1));
        auto c       = p1.add_literal(migraphx::generate_literal(ws2, 2));
        auto d       = p1.add_literal(migraphx::generate_literal(ws2, 3));
        auto e       = p1.add_literal(migraphx::generate_literal(s, 4));
        auto f       = p1.add_literal(migraphx::generate_literal(s, 5));
        auto convx   = p1.add_instruction(migraphx::op::convolution{{1, 1}}, input, a);
        auto convy   = p1.add_instruction(migraphx::op::convolution{{1, 1}}, input, b);
        auto dotx    = p1.add_instruction(migraphx::op::dot{}, input, c);
        auto doty    = p1.add_instruction(migraphx::op::dot{}, input, d);
        auto sqdiffx = p1.add_instruction(migraphx::op::sqdiff{}, input, e);
        auto sqdiffy = p1.add_instruction(migraphx::op::sqdiff{}, input, f);
        auto sum1    = p1.add_instruction(migraphx::op::add{}, convx, convy);
        auto sum2    = p1.add_instruction(migraphx::op::add{}, dotx, doty);
        auto sum3    = p1.add_instruction(migraphx::op::add{}, sqdiffx, sqdiffy);
        auto sum4    = p1.add_instruction(migraphx::op::add{}, sum1, sum2);
        auto sum5    = p1.add_instruction(migraphx::op::add{}, sum4, sum3);
        p1.add_instruction(pass_op{}, sum5);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto input   = p2.add_parameter("input", s);
        auto a       = p2.add_literal(migraphx::generate_literal(ws1, 0));
        auto b       = p2.add_literal(migraphx::generate_literal(ws1, 1));
        auto c       = p2.add_literal(migraphx::generate_literal(ws2, 2));
        auto d       = p2.add_literal(migraphx::generate_literal(ws2, 3));
        auto e       = p2.add_literal(migraphx::generate_literal(s, 4));
        auto f       = p2.add_literal(migraphx::generate_literal(s, 5));
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, a, b);
        auto concat2 = p2.add_instruction(migraphx::op::concat{3}, c, d);
        auto conv    = p2.add_instruction(migraphx::op::convolution{{1, 1}}, input, concat1);
        auto convx   = p2.add_instruction(migraphx::op::slice{{1}, {0}, {6}}, conv);
        auto convy   = p2.add_instruction(migraphx::op::slice{{1}, {6}, {12}}, conv);
        auto sum1    = p2.add_instruction(migraphx::op::add{}, convx, convy);
        auto dot     = p2.add_instruction(migraphx::op::dot{}, input, concat2);
        auto dotx    = p2.add_instruction(migraphx::op::slice{{3}, {0}, {64}}, dot);
        auto doty    = p2.add_instruction(migraphx::op::slice{{3}, {64}, {128}}, dot);
        auto sum2    = p2.add_instruction(migraphx::op::add{}, dotx, doty);
        auto sqdiffx = p2.add_instruction(migraphx::op::sqdiff{}, input, e);
        auto sqdiffy = p2.add_instruction(migraphx::op::sqdiff{}, input, f);
        auto sum3    = p2.add_instruction(migraphx::op::add{}, sqdiffx, sqdiffy);
        auto sum4    = p2.add_instruction(migraphx::op::add{}, sum1, sum2);
        auto sum5    = p2.add_instruction(migraphx::op::add{}, sum4, sum3);
        p2.add_instruction(pass_op{}, sum5);
    }
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(simplify_mul_slice_conv_horiz_fusion)
{
    migraphx::program p1;
    {
        auto x = p1.add_parameter("x", {migraphx::shape::int32_type, {1, 1024, 17, 17}});
        auto w = p1.add_literal(
            migraphx::generate_literal({migraphx::shape::int32_type, {768, 1024, 1, 1}}));
        auto conv   = p1.add_instruction(migraphx::op::convolution{}, x, w);
        auto slice1 = p1.add_instruction(migraphx::op::slice{{1}, {0}, {384}}, conv);
        auto a1 =
            p1.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}, 1));
        auto b1  = p1.add_instruction(migraphx::op::broadcast{1, {1, 384, 17, 17}}, a1);
        auto mul = p1.add_instruction(migraphx::op::mul{}, slice1, b1);
        auto a2 =
            p1.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}, 2));
        auto b2   = p1.add_instruction(migraphx::op::broadcast{1, {1, 384, 17, 17}}, a2);
        auto add1 = p1.add_instruction(migraphx::op::add{}, mul, b2);
        auto a3 =
            p1.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}, 3));
        auto b3     = p1.add_instruction(migraphx::op::broadcast{1, {1, 384, 17, 17}}, a3);
        auto slice2 = p1.add_instruction(migraphx::op::slice{{1}, {384}, {768}}, conv);
        auto add2   = p1.add_instruction(migraphx::op::add{}, slice2, b3);
        p1.add_instruction(pass_op{}, add1, add2);
    }
    run_pass(p1);

    migraphx::program p2;
    {
        auto x = p2.add_parameter("x", {migraphx::shape::int32_type, {1, 1024, 17, 17}});
        auto w = p2.add_literal(
            migraphx::generate_literal({migraphx::shape::int32_type, {768, 1024, 1, 1}}));
        auto wslice1 = p2.add_instruction(migraphx::op::slice{{0}, {0}, {384}}, w);
        auto a1 =
            p2.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}, 1));
        auto b1      = p2.add_instruction(migraphx::op::broadcast{0, {384, 1024, 1, 1}}, a1);
        auto mul     = p2.add_instruction(migraphx::op::mul{}, b1, wslice1);
        auto wslice2 = p2.add_instruction(migraphx::op::slice{{0}, {384}, {768}}, w);
        auto concat1 = p2.add_instruction(migraphx::op::concat{0}, mul, wslice2);
        auto conv    = p2.add_instruction(migraphx::op::convolution{}, x, concat1);
        auto a2 =
            p2.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}, 2));
        auto a3 =
            p2.add_literal(migraphx::generate_literal({migraphx::shape::int32_type, {384}}, 3));
        auto concat2 = p2.add_instruction(migraphx::op::concat{}, a2, a3);
        auto b4      = p2.add_instruction(migraphx::op::broadcast{1, {1, 768, 17, 17}}, concat2);
        auto add     = p2.add_instruction(migraphx::op::add{}, conv, b4);
        auto slice1  = p2.add_instruction(migraphx::op::slice{{1}, {0}, {384}}, add);
        auto slice2  = p2.add_instruction(migraphx::op::slice{{1}, {384}, {768}}, add);
        p2.add_instruction(pass_op{}, slice1, slice2);
    }
    EXPECT(p1.sort() == p2.sort());
}
TEST_CASE(reorder_reshape_slice)
{
    std::vector<int64_t> perm0 = {0, 2, 1, 3};
    std::vector<int64_t> perm1 = {0, 2, 3, 1};
    auto create_p1             = [&](std::size_t batch_size) {
        migraphx::program p1;
        auto s     = migraphx::shape{migraphx::shape::float_type, {batch_size, 128, 1920}};
        auto input = p1.add_parameter("input", s);
        auto slc0  = p1.add_instruction(migraphx::op::slice{{2}, {0}, {640}}, input);
        auto slc1  = p1.add_instruction(migraphx::op::slice{{2}, {640}, {1280}}, input);
        auto slc2  = p1.add_instruction(migraphx::op::slice{{2}, {1280}, {1920}}, input);

        auto c0 = p1.add_instruction(migraphx::op::contiguous{}, slc0);
        auto c1 = p1.add_instruction(migraphx::op::contiguous{}, slc1);
        auto c2 = p1.add_instruction(migraphx::op::contiguous{}, slc2);

        std::vector<int64_t> lens = {static_cast<int64_t>(batch_size), 128, 10, 64};
        auto r0                   = p1.add_instruction(migraphx::op::reshape{lens}, c0);
        auto r1                   = p1.add_instruction(migraphx::op::reshape{lens}, c1);
        auto r2                   = p1.add_instruction(migraphx::op::reshape{lens}, c2);

        auto t0 = p1.add_instruction(migraphx::op::transpose{perm0}, r0);
        auto t1 = p1.add_instruction(migraphx::op::transpose{perm0}, r1);
        auto t2 = p1.add_instruction(migraphx::op::transpose{perm1}, r2);

        auto sum = p1.add_instruction(migraphx::op::add{}, t0, t1);
        auto ret = p1.add_instruction(migraphx::op::dot{}, sum, t2);
        p1.add_return({ret});

        return p1;
    };

    auto create_p2 = [&](std::size_t batch_size) {
        migraphx::program p2;
        auto s     = migraphx::shape{migraphx::shape::float_type, {batch_size, 128, 1920}};
        auto input = p2.add_parameter("input", s);
        std::vector<int64_t> lens = {static_cast<int64_t>(batch_size), 128, 30, 64};
        auto r                    = p2.add_instruction(migraphx::op::reshape{lens}, input);

        auto slc0 = p2.add_instruction(migraphx::op::slice{{2}, {0}, {10}}, r);
        auto slc1 = p2.add_instruction(migraphx::op::slice{{2}, {10}, {20}}, r);
        auto slc2 = p2.add_instruction(migraphx::op::slice{{2}, {20}, {30}}, r);

        auto t0 = p2.add_instruction(migraphx::op::transpose{perm0}, slc0);
        auto t1 = p2.add_instruction(migraphx::op::transpose{perm0}, slc1);
        auto t2 = p2.add_instruction(migraphx::op::transpose{perm1}, slc2);

        auto sum = p2.add_instruction(migraphx::op::add{}, t0, t1);
        auto ret = p2.add_instruction(migraphx::op::dot{}, sum, t2);
        p2.add_return({ret});

        return p2;
    };

    auto test = [&](std::size_t batch_size) {
        auto p1 = create_p1(batch_size);
        run_pass(p1);
        auto p2 = create_p2(batch_size);
        EXPECT(p1.sort() == p2.sort());
    };

    test(1);
    test(4);
    test(8);
}

TEST_CASE(reorder_reshape_slice_move_axis1)
{
    auto create_p1 = [](std::size_t batch_size) {
        migraphx::program p1;
        auto s = migraphx::shape{migraphx::shape::float_type, {batch_size, 256, 96}};
        std::vector<int64_t> perm0 = {0, 2, 1, 3};
        std::vector<int64_t> perm1 = {0, 2, 3, 1};
        auto input                 = p1.add_parameter("input", s);
        auto slc0                  = p1.add_instruction(migraphx::op::slice{{2}, {0}, {32}}, input);
        auto slc1 = p1.add_instruction(migraphx::op::slice{{2}, {32}, {64}}, input);
        auto slc2 = p1.add_instruction(migraphx::op::slice{{2}, {64}, {96}}, input);

        auto c0 = p1.add_instruction(migraphx::op::contiguous{}, slc0);
        auto c1 = p1.add_instruction(migraphx::op::contiguous{}, slc1);
        auto c2 = p1.add_instruction(migraphx::op::contiguous{}, slc2);

        std::vector<int64_t> lens = {static_cast<int64_t>(batch_size), 64, 4, 32};
        auto r0                   = p1.add_instruction(migraphx::op::reshape{lens}, c0);
        auto r1                   = p1.add_instruction(migraphx::op::reshape{lens}, c1);
        auto r2                   = p1.add_instruction(migraphx::op::reshape{lens}, c2);

        auto t0 = p1.add_instruction(migraphx::op::transpose{perm0}, r0);
        auto t1 = p1.add_instruction(migraphx::op::transpose{perm0}, r1);
        auto t2 = p1.add_instruction(migraphx::op::transpose{perm1}, r2);

        auto sum = p1.add_instruction(migraphx::op::add{}, t0, t1);
        auto ret = p1.add_instruction(migraphx::op::dot{}, sum, t2);
        p1.add_return({ret});

        return p1;
    };

    auto create_p2 = [](std::size_t batch_size) {
        migraphx::program p;
        auto s = migraphx::shape{migraphx::shape::float_type, {batch_size, 256, 96}};
        std::vector<int64_t> perm0 = {0, 2, 1, 3};
        std::vector<int64_t> perm1 = {0, 2, 3, 1};
        auto input                 = p.add_parameter("input", s);
        std::vector<int64_t> lens  = {static_cast<int64_t>(batch_size), 64, 4, 96};
        auto rsp                   = p.add_instruction(migraphx::op::reshape{lens}, input);
        auto slc0                  = p.add_instruction(migraphx::op::slice{{3}, {0}, {32}}, rsp);
        auto t0                    = p.add_instruction(migraphx::op::transpose{perm0}, slc0);
        auto slc1                  = p.add_instruction(migraphx::op::slice{{3}, {32}, {64}}, rsp);
        auto t1                    = p.add_instruction(migraphx::op::transpose{perm0}, slc1);
        auto slc2                  = p.add_instruction(migraphx::op::slice{{3}, {64}, {96}}, rsp);
        auto t2                    = p.add_instruction(migraphx::op::transpose{perm1}, slc2);

        auto sum = p.add_instruction(migraphx::op::add{}, t0, t1);
        auto ret = p.add_instruction(migraphx::op::dot{}, sum, t2);
        p.add_return({ret});

        return p;
    };

    auto test = [&](std::size_t batch_size) {
        auto p1 = create_p1(batch_size);
        auto p2 = create_p2(batch_size);
        run_pass(p1);
        EXPECT(p1.sort() == p2.sort());
    };

    test(4);
    test(8);
}

TEST_CASE(reorder_reshape_slice_move_axis2)
{
    auto create_p1 = [] {
        migraphx::program p1;
        migraphx::shape s{migraphx::shape::float_type, {128, 96}};
        auto input = p1.add_parameter("input", s);
        auto slc0  = p1.add_instruction(migraphx::op::slice{{1}, {0}, {32}}, input);
        auto slc1  = p1.add_instruction(migraphx::op::slice{{1}, {32}, {64}}, input);
        auto slc2  = p1.add_instruction(migraphx::op::slice{{1}, {64}, {96}}, input);

        auto c0 = p1.add_instruction(migraphx::op::contiguous{}, slc0);
        auto c1 = p1.add_instruction(migraphx::op::contiguous{}, slc1);
        auto c2 = p1.add_instruction(migraphx::op::contiguous{}, slc2);

        std::vector<int64_t> lens = {1, 16, 8, 32};
        auto r0                   = p1.add_instruction(migraphx::op::reshape{lens}, c0);
        auto r1                   = p1.add_instruction(migraphx::op::reshape{lens}, c1);
        auto r2                   = p1.add_instruction(migraphx::op::reshape{lens}, c2);

        auto sum = p1.add_instruction(migraphx::op::add{}, r0, r1);
        auto ret = p1.add_instruction(migraphx::op::mul{}, sum, r2);
        p1.add_return({ret});

        return p1;
    };

    auto create_p2 = [] {
        migraphx::program p;
        auto s                    = migraphx::shape{migraphx::shape::float_type, {128, 96}};
        auto input                = p.add_parameter("input", s);
        std::vector<int64_t> lens = {1, 16, 8, 96};
        auto rsp                  = p.add_instruction(migraphx::op::reshape{lens}, input);
        auto slc0                 = p.add_instruction(migraphx::op::slice{{3}, {0}, {32}}, rsp);
        auto slc1                 = p.add_instruction(migraphx::op::slice{{3}, {32}, {64}}, rsp);
        auto slc2                 = p.add_instruction(migraphx::op::slice{{3}, {64}, {96}}, rsp);

        auto sum = p.add_instruction(migraphx::op::add{}, slc0, slc1);
        auto ret = p.add_instruction(migraphx::op::mul{}, sum, slc2);
        p.add_return({ret});

        return p;
    };

    auto p1 = create_p1();
    auto p2 = create_p2();
    run_pass(p1);
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(reorder_reshape_slice_not_apply)
{
    auto create_p = [] {
        migraphx::program p;
        migraphx::shape s{migraphx::shape::float_type, {128, 96}};
        auto input = p.add_parameter("input", s);
        auto slc0  = p.add_instruction(migraphx::op::slice{{1}, {0}, {32}}, input);
        auto slc1  = p.add_instruction(migraphx::op::slice{{1}, {32}, {64}}, input);
        auto slc2  = p.add_instruction(migraphx::op::slice{{1}, {64}, {96}}, input);

        auto c0 = p.add_instruction(migraphx::op::contiguous{}, slc0);
        auto c1 = p.add_instruction(migraphx::op::contiguous{}, slc1);
        auto c2 = p.add_instruction(migraphx::op::contiguous{}, slc2);

        std::vector<int64_t> lens = {1, 16, 16, 16};
        auto r0                   = p.add_instruction(migraphx::op::reshape{lens}, c0);
        auto r1                   = p.add_instruction(migraphx::op::reshape{lens}, c1);
        auto r2                   = p.add_instruction(migraphx::op::reshape{lens}, c2);

        auto sum = p.add_instruction(migraphx::op::add{}, r0, r1);
        auto ret = p.add_instruction(migraphx::op::mul{}, sum, r2);
        p.add_return({ret});

        return p;
    };

    auto p1 = create_p();
    auto p2 = p1;
    run_pass(p1);
    EXPECT(p1.sort() == p2.sort());
}

TEST_CASE(reorder_reshape_slice_diff_dims)
{
    auto create_p1 = [](std::size_t batch_size) {
        migraphx::program p1;
        auto s = migraphx::shape{migraphx::shape::float_type, {batch_size, 96, 96}};
        std::vector<int64_t> perm0 = {0, 2, 1, 3};
        std::vector<int64_t> perm1 = {0, 2, 3, 1};
        auto input                 = p1.add_parameter("input", s);
        auto slc0                  = p1.add_instruction(migraphx::op::slice{{2}, {0}, {32}}, input);
        auto slc1 = p1.add_instruction(migraphx::op::slice{{2}, {32}, {64}}, input);
        auto slc2 = p1.add_instruction(migraphx::op::slice{{2}, {64}, {96}}, input);

        auto c0 = p1.add_instruction(migraphx::op::contiguous{}, slc0);
        auto c1 = p1.add_instruction(migraphx::op::contiguous{}, slc1);
        auto c2 = p1.add_instruction(migraphx::op::contiguous{}, slc2);

        std::vector<int64_t> lens  = {static_cast<int64_t>(batch_size), 32, 3, 32};
        std::vector<int64_t> lens1 = {static_cast<int64_t>(batch_size), 48, 2, 32};
        auto r0                    = p1.add_instruction(migraphx::op::reshape{lens}, c0);
        auto r1                    = p1.add_instruction(migraphx::op::reshape{lens}, c1);
        auto r2                    = p1.add_instruction(migraphx::op::reshape{lens1}, c2);

        p1.add_return({r0, r1, r2});

        return p1;
    };

    auto test = [&](std::size_t batch_size) {
        auto p1 = create_p1(batch_size);
        auto p2 = p1;
        run_pass(p1);
        EXPECT(p1.sort() == p2.sort());
    };

    test(4);
    test(8);
}

TEST_CASE(reorder_slice_trans)
{
    std::vector<int64_t> perm = {0, 2, 1};
    auto create_p1            = [&](std::size_t batch_size) {
        migraphx::program p1;
        auto s     = migraphx::shape{migraphx::shape::float_type, {batch_size, 128, 1920}};
        auto input = p1.add_parameter("input", s);
        auto slc0  = p1.add_instruction(migraphx::op::slice{{2}, {0}, {640}}, input);
        auto slc1  = p1.add_instruction(migraphx::op::slice{{2}, {640}, {1280}}, input);
        auto slc2  = p1.add_instruction(migraphx::op::slice{{2}, {1280}, {1920}}, input);

        auto t0 = p1.add_instruction(migraphx::op::transpose{perm}, slc0);
        auto t1 = p1.add_instruction(migraphx::op::transpose{perm}, slc1);
        auto t2 = p1.add_instruction(migraphx::op::transpose{perm}, slc2);

        auto sum = p1.add_instruction(migraphx::op::add{}, t0, t1);
        auto ret = p1.add_instruction(migraphx::op::mul{}, sum, t2);
        p1.add_return({ret});

        return p1;
    };

    auto create_p2 = [&](std::size_t batch_size) {
        migraphx::program p2;
        auto s     = migraphx::shape{migraphx::shape::float_type, {batch_size, 128, 1920}};
        auto input = p2.add_parameter("input", s);
        auto r     = p2.add_instruction(migraphx::op::transpose{perm}, input);

        auto slc0 = p2.add_instruction(migraphx::op::slice{{1}, {0}, {640}}, r);
        auto slc1 = p2.add_instruction(migraphx::op::slice{{1}, {640}, {1280}}, r);
        auto slc2 = p2.add_instruction(migraphx::op::slice{{1}, {1280}, {1920}}, r);

        auto sum = p2.add_instruction(migraphx::op::add{}, slc0, slc1);
        auto ret = p2.add_instruction(migraphx::op::mul{}, sum, slc2);
        p2.add_return({ret});

        return p2;
    };

    auto test = [&](std::size_t batch_size) {
        auto p1 = create_p1(batch_size);
        run_pass(p1);
        auto p2 = create_p2(batch_size);
        EXPECT(p1.sort() == p2.sort());
    };

    test(1);
    test(8);
}

TEST_CASE(reorder_slice_trans_diff_perm)
{
    auto create_p1 = [](std::size_t batch_size) {
        migraphx::program p1;
        auto s = migraphx::shape{migraphx::shape::float_type, {batch_size, 128, 1920}};
        std::vector<int64_t> perm0 = {0, 2, 1};
        std::vector<int64_t> perm1 = {0, 1, 2};
        auto input                 = p1.add_parameter("input", s);
        auto slc0 = p1.add_instruction(migraphx::op::slice{{2}, {0}, {640}}, input);
        auto slc1 = p1.add_instruction(migraphx::op::slice{{2}, {640}, {1280}}, input);
        auto slc2 = p1.add_instruction(migraphx::op::slice{{2}, {1280}, {1920}}, input);

        auto t0 = p1.add_instruction(migraphx::op::transpose{perm0}, slc0);
        auto t1 = p1.add_instruction(migraphx::op::transpose{perm0}, slc1);
        auto t2 = p1.add_instruction(migraphx::op::transpose{perm1}, slc2);

        auto sum = p1.add_instruction(migraphx::op::add{}, t0, t1);
        auto ret = p1.add_instruction(migraphx::op::dot{}, sum, t2);
        p1.add_return({ret});

        return p1;
    };

    auto test = [&](std::size_t batch_size) {
        auto p1 = create_p1(batch_size);
        run_pass(p1);
        auto p2 = p1;
        EXPECT(p1.sort() == p2.sort());
    };

    test(1);
    test(4);
}

int main(int argc, const char* argv[]) { test::run(argc, argv); }

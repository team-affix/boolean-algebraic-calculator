#include "include/calculator.hpp"
#include <iostream>
#include <sstream>
#include <assert.h>

void test_invert(

)
{
    using namespace ba_calculator;

    std::stringstream l_ss;

    invert l_invert_0(operand::ptr(new unresolved("test")));

    l_ss << l_invert_0.m_descriptor;
    
    assert(l_ss.str() == "!(test)");

    // Clear the stringstream.
    l_ss.str("");


    operand::ptr l_sub_0 = l_invert_0.substitute("t", operand::ptr(new resolved(0)));

    l_ss << l_sub_0->m_descriptor;

    assert(l_ss.str() == "!(test)");

    // Clear the stringstream.
    l_ss.str("");


    operand::ptr l_sub_1 = l_invert_0.substitute("test", operand::ptr(new resolved(0)));

    l_ss << l_sub_1->m_descriptor;

    assert(l_ss.str() == "!(0)");

    // Clear the stringstream.
    l_ss.str("");


    operand::ptr l_simp_0 = l_invert_0.simplify();

    l_ss << l_simp_0->m_descriptor;
    
    assert(l_ss.str() == "!(test)");

    // Clear the stringstream.
    l_ss.str("");


    operand::ptr l_simp_1 = l_sub_1->simplify();

    l_ss << l_simp_1->m_descriptor;
    
    assert(l_ss.str() == "1");

    // Clear the stringstream.
    l_ss.str("");

}

void unit_test_main(

)
{
    test_invert();
}

int main(

)
{
    unit_test_main();
    return 0;
}

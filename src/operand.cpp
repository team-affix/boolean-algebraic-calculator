#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <assert.h>

#include "include/calculator.hpp"

using namespace ba_calculator;

operand::ptr::ptr(
    operand* a_operand
) :
    std::shared_ptr<operand>(a_operand)
{

}

bool operand::ptr::operator<(
    const ptr& a_operand
) const
{
    return get()->operator<(*a_operand);
}

operand::operand(
    const operand_types& a_operand_type
) :
    m_operand_type(a_operand_type),
    m_is_reduced(false)
{

}

operand::ptr operand::reduce_operands(

) const
{
    return ptr((operand*)this);
}

operand::ptr operand::simplify(
    
) const
{
    return ptr((operand*)this);
}

operand::ptr operand::expand(

) const
{
    return ptr((operand*)this);
}

operand::ptr operand::reduce(

) const
{
    if (m_is_reduced)
        // If the operand is already reduced, do nothing. Optimization.
        return ptr((operand*)this);

    ptr l_result = reduce_operands()->simplify()->expand();

    // Enable the flag so as to allow for optimization condition to be satisfied.
    l_result->m_is_reduced = true;

    return l_result;
    
}

bool operand::operator<(
    const operand& a_operand
) const
{
    // Return based on the precedence order of the operations.
    return m_operand_type < a_operand.m_operand_type;
}

bool operand::operator==(
    const operand& a_operand
) const
{
    return !operator<(a_operand) && !a_operand.operator<(*this);
}

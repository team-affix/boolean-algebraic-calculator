#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <assert.h>

#include "include/calculator.hpp"

using namespace ba_calculator;

resolved::~resolved(

)
{
    
}

resolved::resolved(
    const bool& a_value
) :
    operand(RESOLVED),
    m_value(a_value)
{

}

operand::ptr resolved::substitute(
    const std::string& a_identifier,
    const ptr& a_operand
) const
{
    return ptr((operand*)this);
}

std::string resolved::to_string(

) const
{
    return m_value ? "1" : "0";
}

bool resolved::operator<(
    const operand& a_operand
) const
{
    if (a_operand.m_operand_type != m_operand_type)
        // If the operand's type is not UNRESOLVED, just return
        // based on the precedence order of different operations.
        return operand::operator<(a_operand);

    const resolved& l_resolved = (const resolved&)a_operand;

    return !m_value && l_resolved.m_value;
    
}

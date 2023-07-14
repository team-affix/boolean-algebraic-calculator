#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <assert.h>

#include "include/calculator.hpp"

using namespace ba_calculator;

unresolved::~unresolved(

)
{

}

unresolved::unresolved(
    const std::string& a_identifier
) :
    operand(UNRESOLVED),
    m_identifier(a_identifier)
{

}

operand::ptr unresolved::substitute(
    const std::string& a_identifier,
    const ptr& a_operand
) const
{
    if (a_identifier == m_identifier)
        return a_operand;
    
    return ptr((operand*)this);

}

std::string unresolved::to_string(

) const
{
    return m_identifier;
}

bool unresolved::operator<(
    const operand& a_operand
) const
{
    if (a_operand.m_operand_type != m_operand_type)
        // If the operand's type is not UNRESOLVED, just return
        // based on the precedence order of different operations.
        return operand::operator<(a_operand);

    const unresolved& l_unresolved = (const unresolved&)a_operand;

    return m_identifier < l_unresolved.m_identifier;
    
}

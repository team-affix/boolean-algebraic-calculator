#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <assert.h>

#include "include/calculator.hpp"

using namespace ba_calculator;

invert::~invert(

)
{

}

invert::invert(
    const ptr& a_operand
) :
    operand(INVERT),
    m_operand(a_operand)
{

}

operand::ptr invert::reduce_operands(

) const
{
    return ptr(new invert(m_operand->reduce()));
}

operand::ptr invert::simplify(

) const
{
    switch(m_operand->m_operand_type)
    {
        case UNRESOLVED:
        {
            return ptr((operand*)this);
        }
        case RESOLVED:
        {
            resolved* l_resolved_operand = (resolved*)m_operand.get();
            return ptr(new resolved(!l_resolved_operand->m_value));
        }
        case INVERT:
        {
            // DOUBLE NEGATION, just return the grandchild.
            invert* l_invert = (invert*)m_operand.get();
            return l_invert->m_operand;
        }
        case PRODUCT:
        {
            return ptr((operand*)this);
        }
        case SUM:
        {
            return ptr((operand*)this);
        }
        default:
        {
            throw std::runtime_error("Error: unknown operand type in invert::simplify()");
        }
    }
}

operand::ptr invert::expand(

) const
{
    switch(m_operand->m_operand_type)
    {
        case UNRESOLVED:
        {
            return ptr((operand*)this);
        }
        case RESOLVED:
        {
            // resolved* l_resolved_operand = (resolved*)l_boiled_operand.get();
            // return ptr(new resolved(!l_resolved_operand->m_value));

            // ERROR CASE: We should NEVER see an inverted resolved in an expansion call.
            throw std::runtime_error("Error: inverted resolved in invert::expand()");
        }
        case INVERT:
        {
            // ERROR CASE: We should NEVER see a double-negation in an expansion call.
            throw std::runtime_error("Error: double-negation in invert::expand()");
        }
        case PRODUCT:
        {
            // We choose to do the demorgan's expansion

            product* l_product = (product*)m_operand.get();

            std::set<ptr> l_result_operands;

            std::transform(
                l_product->m_operands.begin(),
                l_product->m_operands.end(),
                std::inserter(l_result_operands, l_result_operands.begin()),
                [](
                    const ptr& a_child_operand
                )
                {
                    return ptr(new invert(a_child_operand));
                }
            );

            ptr l_result = ptr(new sum(l_result_operands));

            return l_result->reduce();
            
        }
        case SUM:
        {
            // We choose to do the demorgan's expansion
            
            sum* l_sum = (sum*)m_operand.get();

            std::set<ptr> l_result_operands;

            std::transform(
                l_sum->m_operands.begin(),
                l_sum->m_operands.end(),
                std::inserter(l_result_operands, l_result_operands.begin()),
                [](
                    const ptr& a_child_operand
                )
                {
                    return ptr(new invert(a_child_operand));
                }
            );

            ptr l_result = ptr(new product(l_result_operands));

            return l_result->reduce();
            
        }
        default:
        {
            throw std::runtime_error("Error: unknown operand type in invert::expand()");
        }
    }

}

operand::ptr invert::substitute(
    const std::string& a_identifier,
    const ptr& a_operand
) const
{
    return ptr(new invert(
        m_operand->substitute(a_identifier, a_operand)
    ));
}

std::string invert::to_string(

) const
{
    return std::string("!") + m_operand->to_string();
}

bool invert::operator<(
    const operand& a_operand
) const
{
    if (a_operand.m_operand_type != m_operand_type)
        return operand::operator<(a_operand);

    const invert& l_invert = (const invert&)a_operand;

    return m_operand.operator<(l_invert.m_operand);
    
}

#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <assert.h>

#include "include/calculator.hpp"

using namespace ba_calculator;

sum::~sum(

)
{

}

sum::sum(
    const std::set<ptr>& a_operands
) :
    operand(SUM),
    m_operands(a_operands)
{
    
}

operand::ptr sum::simplify(

) const
{

}

operand::ptr sum::expand(

) const
{
    std::set<ptr> l_products;

    for (const ptr& l_operand : m_operands)
    {
        ptr l_simplified_operand = l_operand->simplify();

        switch(l_simplified_operand->m_operand_type)
        {
            case UNRESOLVED:
            {
                l_products.insert(ptr(new product({l_simplified_operand})));
                break;
            }
            case RESOLVED:
            {
                const resolved* l_resolved = (const resolved*)l_simplified_operand.get();

                if (l_resolved->m_value == 0)
                    continue;

                // If we make it here, it means the resolved operand's value was 1.
                // This bypasses the rest of the sum. Early return.
                return ptr(new resolved(1));
                
            }
            case INVERT:
            {
                l_products.insert(ptr(new product({l_simplified_operand})));
                break;
            }
            case PRODUCT:
            {
                l_products.insert(l_simplified_operand);
                break;
            }
            case SUM:
            {
                // We will merge the sum in, by converting its operands 
                // to products where need be.

                const sum* l_sum = (const sum*)l_simplified_operand.get();
                
                std::transform(
                    l_sum->m_operands.begin(),
                    l_sum->m_operands.end(),
                    std::inserter(l_products, l_products.begin()),
                    [](
                        const ptr& a_child_operand
                    )
                    {
                        if (a_child_operand->m_operand_type != PRODUCT)
                            return ptr(new product({a_child_operand}));
                            
                        return a_child_operand;

                    }
                );
                
                break;
            }
            default:
            {
                throw std::runtime_error("Error: unknown operand type in product::simplify()");
            }
        }

        // Now that we've aggregated a bunch of products in the sum, we need to
        // find coverages.

        for (auto l_it_0 = l_products.begin(); l_it_0 != l_products.end(); std::advance(l_it_0, 1))
        {
            for (auto l_it_1 = l_products.begin(); l_it_1 != l_products.end();)
            {

                // Save the current iterator as a temporary var,
                // and advance the iterator l_it_1 to the next position,
                // so that if we do invalidate the current iterator,
                // we will be able to continue iteration. 
                auto l_current = l_it_1;
                std::advance(l_it_1, 1);
                
                if (l_it_0 == l_it_1)
                    continue;

                if (covers((const product&)**l_it_0, (const product&)**l_current))
                    l_products.erase(l_current);

            }
        }

        return ptr(new sum(l_products));
        
    }
    
}

operand::ptr sum::substitute(
    const std::string& a_identifier,
    const ptr& a_substitution_operand
) const
{
    std::set<ptr> l_result_operands;
    
    // Transform each operand according to the substitution rule.
    std::transform(
        m_operands.begin(),
        m_operands.end(),
        std::inserter(l_result_operands, l_result_operands.begin()),
        [&a_identifier, &a_substitution_operand](const ptr& a_operand)
        {
            return a_operand->substitute(a_identifier, a_substitution_operand);
        }
    );

    return ptr(new sum(l_result_operands));
    
}

std::string sum::to_string(

) const
{
    std::stringstream l_result_ss;
    
    l_result_ss << "(";

    // Write all of the descriptors to the resulting stringstream.
    std::transform(
        m_operands.begin(),
        m_operands.end(),
        std::ostream_iterator<std::string>(l_result_ss, " || "),
        [](const ptr& a_operand)
        {
            return a_operand->to_string();
        }
    );

    l_result_ss << ")";

    // Convert the stringstream to a string and return it.
    return l_result_ss.str();
    
}

bool sum::operator<(
    const operand& a_operand
) const
{
    if (a_operand.m_operand_type != m_operand_type)
        return operand::operator<(a_operand);

    const sum& l_sum = (const sum&)a_operand;

    if (m_operands.size() < l_sum.m_operands.size())
        // Take the size as a cue.
        return true;

    if (m_operands.size() > l_sum.m_operands.size())
        // If the local has more operands, consider it to be the larger of the two.
        return false;

    std::set<ptr>::iterator l_it_0 = m_operands.begin();
    std::set<ptr>::iterator l_it_1 = l_sum.m_operands.begin();

    for (int i = 0; i < m_operands.size(); i++)
    {
        // Compare element-wise.
        if (*l_it_0 < *l_it_1)
            return true;
        if (*l_it_0 > *l_it_1)
            return false;

        std::advance(l_it_0, 1);
        std::advance(l_it_1, 1);
        
    }

    return false;

}

bool sum::covers(
    const product& a_product_0,
    const product& a_product_1
)
{
    for (const ptr& l_product_0_operand : a_product_0.m_operands)
    {
        // Each operand in the product will be a literal.
        assert(l_product_0_operand->m_operand_type == INVERT || l_product_0_operand->m_operand_type == UNRESOLVED);
           
        if (
            !std::binary_search(
                a_product_1.m_operands.begin(),
                a_product_1.m_operands.end(),
                l_product_0_operand
            )
        )
            return false;

    }

    return true;
    
}

bool sum::are_opposites(
    const product& a_product_0,
    const product& a_product_1
)
{

}

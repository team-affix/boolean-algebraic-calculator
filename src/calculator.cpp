#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>

#include "calculator.hpp"

using namespace ba_calculator;

operand::operand(
    const operand_types& a_operand_type
) :
    m_operand_type(a_operand_type)
{

}

bool operand::operator<(
    const ptr& a_operand
) const
{
    // Return based on the precedence order of the operations.
    return m_operand_type < a_operand->m_operand_type;
}

bool operand::operator==(
    const ptr& a_operand
) const
{
    return !operator<(a_operand) && !a_operand->operator<(ptr((operand*)this));
}

unresolved::unresolved(
    const std::string& a_identifier
) :
    operand(UNRESOLVED),
    m_identifier(a_identifier)
{

}

operand::ptr unresolved::simplify(

) const
{
    return ptr((operand*)this);
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
    const ptr& a_operand
) const
{
    if (a_operand->m_operand_type != m_operand_type)
        // If the operand's type is not UNRESOLVED, just return
        // based on the precedence order of different operations.
        return operand::operator<(a_operand);

    const unresolved* l_unresolved = (const unresolved*)a_operand.get();

    return m_identifier < l_unresolved->m_identifier;
    
}

resolved::resolved(
    const bool& a_value
) :
    operand(RESOLVED),
    m_value(a_value)
{

}

operand::ptr resolved::simplify(

) const
{
    return ptr((operand*)this);
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

invert::invert(
    const ptr& a_operand
) :
    operand(INVERT),
    m_operand(a_operand)
{

}

operand::ptr invert::simplify(

) const
{
    // First, simplify the operand.
    ptr l_simplified_operand = m_operand->simplify();

    if (l_simplified_operand->m_operand_type == RESOLVED)
    {
        resolved* l_resolved_operand = (resolved*)l_simplified_operand.get();

        return ptr(new resolved(!l_resolved_operand->m_value));
        
    }

    return ptr((operand*)this);
    
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
    return std::string("!(") + m_operand->to_string() + std::string(")");
}

bool invert::operator<(
    const ptr& a_operand
) const
{
    if (a_operand->m_operand_type != m_operand_type)
        return operand::operator<(a_operand);

    const invert* l_invert = (const invert*)a_operand.get();

    return m_operand->operator<(l_invert->m_operand);
    
}

product::product(
    const std::vector<ptr>& a_operands
) :
    operand(PRODUCT),
    m_operands(a_operands)
{

}

operand::ptr product::simplify(

) const
{
    // Initialize the foremost_product_operands vector to contain a 1,
    // as multiplication by 1 is an identity operation.
    std::vector<ptr> l_foremost_product_operands = { ptr(new resolved(1)) };

    // A list of all sums over which we will have to distribute.
    std::deque<ptr>                       l_sums;

    // Simplify the operands and then sort them according to the precedence where
    // sums appear as the right-most elements.
    for (const ptr& l_operand : m_operands)
    {
        ptr l_simplified_operand = l_operand->simplify();

        switch(l_simplified_operand->m_operand_type)
        {
            case UNRESOLVED:
            {
                l_foremost_product_operands.push_back(l_simplified_operand);
                break;
            }
            case RESOLVED:
            {
                const resolved* l_resolved = (const resolved*)l_simplified_operand.get();

                if (l_resolved->m_value == 1)
                    // Do nothing in this case, as the resolved
                    // value just suggests to continue.
                    continue;

                // If we make it here, the resolved value is 0.
                // 0 in a product reduces to zero, and as such we early return.
                return ptr(new resolved(0));

            }
            case INVERT:
            {
                l_foremost_product_operands.push_back(l_simplified_operand);
                break;
            }
            case PRODUCT:
            {
                // Join the operands of the child product with the
                // operands of the parent product. (flatten the products)

                const product* l_product = (const product*)l_simplified_operand.get();

                for (const ptr& a_child_operand : l_product->m_operands)
                    l_foremost_product_operands.push_back(a_child_operand);

                break;
                
            }
            case SUM:
            {
                l_sums.push_back(l_simplified_operand);
                break;
            }
            default:
            {
                throw std::runtime_error("Error: unknown operand type in product::simplify()");
            }
        }

    }

    // Construct a foremost product,
    ptr l_foremost_product = ptr(new product(l_foremost_product_operands));
    l_foremost_product = l_foremost_product->simplify();

    // wrap it in a sum, and then 
    // add it to the front of "sums."
    l_sums.push_front(ptr(new sum({l_foremost_product})));

    while (l_sums.size() > 1)
    {
        // Pop the first sum off of the vector.
        ptr l_first_ptr = l_sums.front();
        l_sums.pop_front();

        // Pop the second sum off of the vector.
        ptr l_second_ptr = l_sums.front();
        l_sums.pop_front();

        // Now, attempt to distribute this single
        // foremost sum (without cloning it) over multiplication
        // to the second sum.
        l_sums.push_front(distribute(l_first_ptr, l_second_ptr));

        // Simplify the newly distributed sum.
        l_sums.front() = l_sums.front()->simplify();

    }

    return l_sums.front();

}

operand::ptr product::substitute(
    const std::string& a_identifier,
    const ptr& a_substitution_operand
) const
{
    std::vector<ptr> l_result_operands;
    
    // Transform each operand according to the substitution rule.
    std::transform(
        m_operands.begin(),
        m_operands.end(),
        std::back_inserter(l_result_operands),
        [&a_identifier, &a_substitution_operand](const ptr& a_operand)
        {
            return a_operand->substitute(a_identifier, a_substitution_operand);
        }
    );

    return ptr(new product(l_result_operands));
    
}

std::string product::to_string(

) const
{
    std::stringstream l_result_ss;

    // Write all of the descriptors to the resulting stringstream.
    std::transform(
        m_operands.begin(),
        m_operands.end(),
        std::ostream_iterator<std::string>(l_result_ss, " && "),
        [](const ptr& a_operand)
        {
            return a_operand->to_string();
        }
    );

    // Convert the stringstream to a string and return it.
    return l_result_ss.str();
    
}

bool product::operator<(
    const ptr& a_operand
) const
{
    if (a_operand->m_operand_type != m_operand_type)
        return operand::operator<(a_operand);

    const product* l_product = (const product*)a_operand.get();

    if (m_operands.size() < l_product->m_operands.size())
        // Take the size as a cue.
        return true;

    for (int i = 0; i < m_operands.size(); i++)
    {
        // Compare element-wise.
        if (m_operands[i]->operator<(l_product->m_operands[i]))
            return true;
    }

    return false;

}

operand::ptr product::distribute(
    const ptr& a_sum_0,
    const ptr& a_sum_1
)
{
    std::vector<ptr> l_result_operands;

    // Cast both pointers
    const sum* l_sum_0 = (const sum*)a_sum_0.get();
    const sum* l_sum_1 = (const sum*)a_sum_1.get();

    for (const ptr& l_operand_0 : l_sum_0->m_operands)
    {   
        for (const ptr& l_operand_1 : l_sum_1->m_operands)
        {
            ptr l_product = ptr(new product({l_operand_0, l_operand_1}));

            l_result_operands.push_back(l_product);
            
        }
        
    }

    return ptr(new sum(l_result_operands));
    
}

sum::sum(
    const std::vector<ptr>& a_operands
) :
    operand(SUM),
    m_operands(a_operands)
{
    
}

operand::ptr sum::simplify(

) const
{
    std::vector<ptr> l_simplified_operands;

    for (const ptr& l_operand : m_operands)
    {
        ptr l_simplified_operand = l_operand->simplify();

        switch(l_simplified_operand->m_operand_type)
        {
            case UNRESOLVED:
            {
                l_simplified_operands.push_back(ptr(new product({l_simplified_operand})));
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
                l_simplified_operands.push_back(ptr(new product({l_simplified_operand})));
                break;
            }
            case PRODUCT:
            {
                l_simplified_operands.push_back(l_simplified_operand);
                break;
            }
            case SUM:
            {
                // We will merge the sum in.

                const sum* l_sum = (const sum*)l_simplified_operand.get();
                
                for (const ptr& l_)
                
                break;
            }
            default:
            {
                throw std::runtime_error("Error: unknown operand type in product::simplify()");
            }
        }
        
    }
    
}

operand::ptr sum::substitute(
    const std::string& a_identifier,
    const ptr& a_substitution_operand
) const
{

}

std::string sum::to_string(

) const
{
    std::stringstream l_result_ss;

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

    // Convert the stringstream to a string and return it.
    return l_result_ss.str();
    
}

bool sum::operator<(
    const ptr& a_operand
) const
{
    if (a_operand->m_operand_type != m_operand_type)
        return operand::operator<(a_operand);

    const product* l_product = (const product*)a_operand.get();

    if (m_operands.size() < l_product->m_operands.size())
        // Take the size as a cue.
        return true;

    for (int i = 0; i < m_operands.size(); i++)
    {
        // Compare element-wise.
        if (m_operands[i]->operator<(l_product->m_operands[i]))
            return true;
    }

    return false;

}

bool sum::covers(
    const ptr& a_product_0,
    const ptr& a_product_1
)
{
    const product* l_product_0 = (const product*)a_product_0.get();
    const product* l_product_1 = (const product*)a_product_1.get();

    for (const ptr& l_product_0_operand : l_product_0->m_operands)
    {
        if (
            !std::binary_search(
            l_product_1->m_operands.begin(),
            l_product_1->m_operands.end(),
            l_product_0_operand,
            [](
                const ptr& a_operand_0,
                const ptr& a_operand_1
            )
            {
                return a_operand_0->operator<(a_operand_1);
            })
        )
            return false;
    }

    return true;
    
}

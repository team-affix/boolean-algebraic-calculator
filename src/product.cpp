#include <algorithm>
#include <deque>
#include <iterator>
#include <sstream>
#include <assert.h>

#include "include/calculator.hpp"

using namespace ba_calculator;

product::~product(

)
{
    
}

product::product(
    const std::set<ptr>& a_operands
) :
    operand(PRODUCT),
    m_operands(a_operands)
{

}

operand::ptr product::reduce_operands(

) const
{
    std::set<ptr> l_result_operands;

    std::transform(
        m_operands.begin(),
        m_operands.end(),
        std::inserter(l_result_operands, l_result_operands.begin()),
        [](
            const ptr& a_operand
        )
        {
            return a_operand->reduce();
        }
    );

    return ptr(new product(l_result_operands));
    
}

operand::ptr product::simplify(

) const
{

}

operand::ptr product::expand(

) const
{
    // Initialize the foremost_product_operands vector to contain a 1,
    // as multiplication by 1 is an identity operation.
    std::set<ptr> l_foremost_product_operands = { ptr(new resolved(1)) };

    // A list of all sums over which we will have to distribute.
    std::set<ptr>                      l_sums;

    // Simplify the operands and then sort them according to the precedence where
    // sums appear as the right-most elements.
    for (const ptr& l_operand : m_operands)
    {
        switch(l_operand->m_operand_type)
        {
            case UNRESOLVED:
            {
                l_foremost_product_operands.insert(l_operand);
                break;
            }
            case RESOLVED:
            {
                const resolved* l_resolved = (const resolved*)l_operand.get();

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
                l_foremost_product_operands.insert(l_operand);
                break;
            }
            case PRODUCT:
            {
                // Merge the operands of the child product with the
                // operands of the parent product. (flatten the products)

                const product* l_product = (const product*)l_operand.get();

                std::copy(
                    l_product->m_operands.begin(),
                    l_product->m_operands.end(),
                    std::inserter(l_foremost_product_operands, l_foremost_product_operands.begin())
                );

                break;
                
            }
            case SUM:
            {
                l_sums.insert(l_operand);
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
    l_foremost_product = l_foremost_product->reduce();

    // wrap it in a sum, and then
    // add it to the front of "sums."
    l_sums.insert(ptr(new sum({l_foremost_product})));

    while (l_sums.size() > 1)
    {   
        // Pop the first sum off of the set.
        ptr l_first_ptr = *l_sums.begin();
        l_sums.erase(l_sums.begin());

        // Pop the second sum off of the set.
        ptr l_second_ptr = *l_sums.begin();
        l_sums.erase(l_sums.begin());
        
        // Now, attempt to distribute this single
        // foremost sum (without cloning it) over multiplication
        // to the second sum.
        l_sums.insert(distribute((const sum&)*l_first_ptr, (const sum&)*l_second_ptr)->reduce());

    }

    return *l_sums.begin();

}

operand::ptr product::substitute(
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

    return ptr(new product(l_result_operands));
    
}

std::string product::to_string(

) const
{
    std::stringstream l_result_ss;

    l_result_ss << "(";

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

    l_result_ss << ")";

    // Convert the stringstream to a string and return it.
    return l_result_ss.str();
    
}

bool product::operator<(
    const operand& a_operand
) const
{
    if (a_operand.m_operand_type != m_operand_type)
        return operand::operator<(a_operand);

    const product& l_product = (const product&)a_operand;

    if (m_operands.size() < l_product.m_operands.size())
        // Take the size as a cue.
        return true;

    if (m_operands.size() > l_product.m_operands.size())
        // If the local has more operands, consider it to be the larger of the two.
        return false;

    std::set<ptr>::iterator l_it_0 = m_operands.begin();
    std::set<ptr>::iterator l_it_1 = l_product.m_operands.begin();

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

operand::ptr product::distribute(
    const sum& a_sum_0,
    const sum& a_sum_1
)
{
    std::set<ptr> l_result_operands;

    for (const ptr& l_operand_0 : a_sum_0.m_operands)
    {
        // Sanity check on first operand's child
        assert(l_operand_0->m_operand_type == PRODUCT);
        
        for (const ptr& l_operand_1 : a_sum_1.m_operands)
        {
            // Sanity check on second operand's child
            assert(l_operand_1->m_operand_type == PRODUCT);
            
            ptr l_product = ptr(new product({l_operand_0, l_operand_1}));

            l_result_operands.insert(l_product);
            
        }
        
    }

    return ptr(new sum(l_result_operands));
    
}

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
    m_operand_type(a_operand_type)
{

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

unresolved::unresolved(
    const std::string& a_identifier
) :
    operand(UNRESOLVED),
    m_identifier(a_identifier)
{

}

operand::ptr unresolved::expand(

) const
{
    return ptr((operand*)this);
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

resolved::resolved(
    const bool& a_value
) :
    operand(RESOLVED),
    m_value(a_value)
{

}

operand::ptr resolved::expand(

) const
{
    return ptr((operand*)this);
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

operand::ptr invert::expand(

) const
{
    // First, expand the operand.
    ptr l_expanded_operand = m_operand->expand();

    switch(l_expanded_operand->m_operand_type)
    {
        case UNRESOLVED:
        {
            return ptr(new invert(l_expanded_operand));
        }
        case RESOLVED:
        {
            resolved* l_resolved_operand = (resolved*)l_expanded_operand.get();
            return ptr(new resolved(!l_resolved_operand->m_value));
        }
        case INVERT:
        {
            // DOUBLE NEGATION, just return the grandchild.
            invert* l_invert = (invert*)l_expanded_operand.get();
            return l_invert->m_operand;
        }
        case PRODUCT:
        {
            // We choose to do the demorgan's expansion

            product* l_product = (product*)l_expanded_operand.get();

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

            return l_result->expand();
            
        }
        case SUM:
        {
            // We choose to do the demorgan's expansion
            
            sum* l_sum = (sum*)l_expanded_operand.get();

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

            return l_result->expand();
            
        }
        default:
        {
            throw std::runtime_error("Error: unknown operand type in invert::simplify()");
        }
    }

}

operand::ptr invert::simplify(

) const
{
    // First, simplify the operand.
    ptr l_simplified_operand = m_operand->simplify();

    switch(l_simplified_operand->m_operand_type)
    {
        case UNRESOLVED:
        {
            return ptr(new invert(l_simplified_operand));
        }
        case RESOLVED:
        {
            resolved* l_resolved_operand = (resolved*)l_simplified_operand.get();
            return ptr(new resolved(!l_resolved_operand->m_value));
        }
        case INVERT:
        {
            // DOUBLE NEGATION, just return the grandchild.
            invert* l_invert = (invert*)l_simplified_operand.get();
            return l_invert->m_operand;
        }
        case PRODUCT:
        {
            return ptr(new invert(l_simplified_operand));
        }
        case SUM:
        {
            return ptr(new invert(l_simplified_operand));
        }
        default:
        {
            throw std::runtime_error("Error: unknown operand type in invert::simplify()");
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

product::product(
    const std::set<ptr>& a_operands
) :
    operand(PRODUCT),
    m_operands(a_operands)
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
        ptr l_simplified_operand = l_operand->simplify();

        switch(l_simplified_operand->m_operand_type)
        {
            case UNRESOLVED:
            {
                l_foremost_product_operands.insert(l_simplified_operand);
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
                l_foremost_product_operands.insert(l_simplified_operand);
                break;
            }
            case PRODUCT:
            {
                // Merge the operands of the child product with the
                // operands of the parent product. (flatten the products)

                const product* l_product = (const product*)l_simplified_operand.get();

                std::copy(
                    l_product->m_operands.begin(),
                    l_product->m_operands.end(),
                    std::inserter(l_foremost_product_operands, l_foremost_product_operands.begin())
                );

                break;
                
            }
            case SUM:
            {
                l_sums.insert(l_simplified_operand);
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
        l_sums.insert(distribute((const sum&)*l_first_ptr, (const sum&)*l_second_ptr)->simplify());

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

sum::sum(
    const std::set<ptr>& a_operands
) :
    operand(SUM),
    m_operands(a_operands)
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

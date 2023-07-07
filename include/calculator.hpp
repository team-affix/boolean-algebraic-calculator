#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <string>
#include <memory>

namespace ba_calculator
{
    struct operand
    {
        using ptr = std::shared_ptr<operand>;

        std::string m_operand_type;
        std::string m_descriptor;
        
        operand(
            const std::string& a_operand_type,
            const std::string& a_descriptor
        ) :
            m_operand_type(a_operand_type),
            m_descriptor(a_descriptor)
        {

        }
        
        virtual ptr clone(

        ) = 0;
        
        virtual ptr simplify(

        ) = 0;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) = 0;
        
    };
    
    struct unresolved : public operand
    {
        unresolved(
            const std::string& a_identifier
        ) :
            operand("unresolved", a_identifier)
        {

        }

        virtual ptr clone(

        )
        {
            return ptr(new unresolved(m_descriptor));
        }

        virtual ptr simplify(

        )
        {
            return clone();
        }

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        )
        {
            if (a_identifier == m_descriptor)
                return a_operand->clone();
            
            return clone();

        }
        
    };

    struct resolved : public operand
    {
        bool m_value;

        resolved(
            const bool& a_value
        ) :
            operand("resolved", a_value ? "1" : "0"),
            m_value(a_value)
        {

        }

        virtual ptr clone(

        )
        {
            return ptr(new resolved(m_value));
        }

        virtual ptr simplify(

        )
        {
            return ptr(new resolved(m_value));
        }

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        )
        {
            return clone();
        }
        
    };

    struct invert : public operand
    {
        ptr m_operand;
        
        invert(
            const ptr& a_operand
        ) :
            operand(
                "invert",
                std::string("!(") + a_operand->m_descriptor + std::string(")")
            ),
            m_operand(a_operand)
        {

        }

        virtual ptr clone(

        )
        {
            return ptr(new invert(m_operand->clone()));
        }

        virtual ptr simplify(

        )
        {
            // First, simplify the operand.
            ptr l_simplified_operand = m_operand->simplify();

            if (l_simplified_operand->m_operand_type == "resolved")
            {
                resolved* l_resolved_operand = (resolved*)l_simplified_operand.get();

                return ptr(new resolved(!l_resolved_operand->m_value));
                
            }

            return clone();
            
        }

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        )
        {
            return ptr(new invert(
                m_operand->substitute(a_identifier, a_operand)
            ));
        }
        
    };
    
}

#endif

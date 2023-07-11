#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <string>
#include <memory>
#include <set>

namespace ba_calculator
{
    enum operand_types
    {
        UNKNOWN = 0,
        UNRESOLVED = 1,
        RESOLVED = 2,
        INVERT = 3,
        PRODUCT = 4,
        SUM = 5
    };
    
    struct operand
    {
        struct ptr : public std::shared_ptr<operand>
        {
            ptr(
                operand* a_operand
            );
            
            bool operator<(
                const ptr& a_operand
            ) const;
        };

        operand_types m_operand_type;
        
        operand(
            const operand_types& a_operand_type
        );
        
        virtual ptr expand(

        ) const = 0;

        virtual ptr simplify(

        ) const = 0;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const = 0;

        virtual std::string to_string(

        ) const = 0;

        virtual bool operator<(
            const operand& a_operand
        ) const;

        bool operator==(
            const operand& a_operand
        ) const;
        
    };
    
    struct unresolved : public operand
    {
        std::string m_identifier;

        unresolved(
            const std::string& a_identifier
        );

        virtual ptr expand(

        ) const;

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const operand& a_operand
        ) const;

    };

    struct resolved : public operand
    {
        bool m_value;

        resolved(
            const bool& a_value
        );

        virtual ptr expand(

        ) const;

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const operand& a_operand
        ) const;
        
    };

    struct invert : public operand
    {
        ptr m_operand;
        
        invert(
            const ptr& a_operand
        );

        virtual ptr expand(

        ) const;

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const operand& a_operand
        ) const;
        
    };

    struct sum;

    struct product : public operand
    {
        std::set<ptr> m_operands;

        product(
            const std::set<ptr>& a_operands
        );
        
        virtual ptr expand(

        ) const;

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_substitution_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const operand& a_operand
        ) const;
    
    private:
        static ptr distribute(
            const sum& a_sum_0,
            const sum& a_sum_1
        );

    };

    struct sum : public operand
    {
        std::set<ptr> m_operands;

        sum(
            const std::set<ptr>& a_operands
        );
        
        virtual ptr expand(

        ) const;

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_substitution_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const operand& a_operand
        ) const;

    private:
        static bool covers(
            const product& a_product_0,
            const product& a_product_1
        );

        static bool are_opposites(
            const product& a_product_0,
            const product& a_product_1
        );
        
    };
    
}

#endif

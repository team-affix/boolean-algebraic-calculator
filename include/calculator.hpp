#ifndef CALCULATOR_HPP
#define CALCULATOR_HPP

#include <string>
#include <memory>
#include <vector>

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
        using ptr = std::shared_ptr<operand>;

        operand_types m_operand_type;
        
        operand(
            const operand_types& a_operand_type
        );
        
        virtual ptr simplify(

        ) const = 0;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const = 0;

        virtual std::string to_string(

        ) const = 0;

        virtual bool operator<(
            const ptr& a_operand
        ) const;

        bool operator==(
            const ptr& a_operand
        ) const;
        
    };
    
    struct unresolved : public operand
    {
        std::string m_identifier;

        unresolved(
            const std::string& a_identifier
        );

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const ptr& a_operand
        ) const;

    };

    struct resolved : public operand
    {
        bool m_value;

        resolved(
            const bool& a_value
        );

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const ptr& a_operand
        ) const;
        
    };

    struct invert : public operand
    {
        ptr m_operand;
        
        invert(
            const ptr& a_operand
        );

        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const ptr& a_operand
        ) const;
        
    };

    struct product : public operand
    {
        std::vector<ptr> m_operands;

        product(
            const std::vector<ptr>& a_operands
        );
        
        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_substitution_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const ptr& a_operand
        ) const;
    
    private:
        static ptr distribute(
            const ptr& a_sum_0,
            const ptr& a_sum_1
        );

    };

    struct sum : public operand
    {
        std::vector<ptr> m_operands;

        sum(
            const std::vector<ptr>& a_operands
        );
        
        virtual ptr simplify(

        ) const;

        virtual ptr substitute(
            const std::string& a_identifier,
            const ptr& a_substitution_operand
        ) const;

        virtual std::string to_string(

        ) const;

        virtual bool operator<(
            const ptr& a_operand
        ) const;

    private:
        static bool covers(
            const ptr& a_product_0,
            const ptr& a_product_1
        );
        
    };
    
}

#endif

#ifndef FUNCTION_H
#define FUNCTION_H

#include <QString>
#include <QVariant>

class Function
{
    public:
        Function();

        /**
         * @brief Checks if the string is a function (if it starts by =)
         * @param _str The string to test
         * @return True if _str is a function, false otherwise.
         */
        static bool isFunction(const QString& _str);

        /**
         * @brief Evaluates _str and returns a function.
         * @param _str The code to evaluate
         * @return  The evaluated function
         */
        static Function evaluate(const QString& _str);

        /**
         * @brief Returns a string version of the function (the "source code")
         * @return
         */
        QString toString() const;

        /**
         * @brief Returns the value of the function, after it's been evaluated, to be displayed.
         * @return
         */
        QVariant value() const;

        static const char FUNC_BEGIN_CHAR;

    private:
        bool m_isValid;

        QString m_lastCode;
};

#endif // FUNCTION_H

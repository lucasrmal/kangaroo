#ifndef BOOKSETTINGSTAB
#define BOOKSETTINGSTAB

#include <QWidget>

class BookSettingsTab : public QWidget
{
    Q_OBJECT

    public:
        explicit BookSettingsTab(QWidget* _parent) : QWidget(_parent) {}

        /**
         * @brief Tab text
         */
        virtual QString tabName() const = 0;

        /**
         * @brief Tab icon
         */
        virtual QString tabIcon() const = 0;

        /**
         * @brief Loads data from _account
         * @param _account
         */
        virtual void fillData() = 0;

        /**
         * @brief Saves the changes. No need to catch ModelException, it will be caught by FormBookSettings.
         * @param _account
         */
        virtual void save() const = 0;

        /**
         * @brief validate Field validation
         * @return Empty list if the validation is succesfull otherwise, list containing the error messages
         */
        virtual QStringList validate() const = 0;
};

/**
 * @brief The BookSettingsTabFactorySuper class
 *
 * Do not use this class, it's for internal use of
 * FormBookSettings. Use BookSettingsTabFactory instead.
 *
 * @see BookSettingsTabFactory
 */
class BookSettingsTabFactorySuper
{
    public:
        virtual BookSettingsTab* buildTab(QWidget* _parent) const = 0;
};

/**
 * Use this class to add register a BookSettingsTab to FormBookSettings.
 *
 * Simply pass <code>new BookSettingsTabFactory<MyOwnBookSettingsTab>()</code> to <code>FormBookSettings::registerTab()</code>.
 */
template<class T>
class BookSettingsTabFactory : public BookSettingsTabFactorySuper
{
    public:
        BookSettingsTab* buildTab(QWidget* _parent) const
        {
            return new T(_parent);
        }
};

#endif // BOOKSETTINGSTAB


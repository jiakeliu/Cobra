#ifndef FILEVALIDATOR_H
#define FILEVALIDATOR_H

#include <QValidator>

class cobraFileValidator : public QValidator
{
public:
    cobraFileValidator();

public:
    /**
     * @fn virtual State validate(QString& string, int& ) const
     * Verify a file exists.
     */
    virtual State validate(QString &, int &) const;
};

#endif // FILEVALIDATOR_H

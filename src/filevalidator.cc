#include "filevalidator.h"

#include <QFile>

cobraFileValidator::cobraFileValidator()
{
}

QValidator::State
cobraFileValidator::validate(QString& input, int& pos) const
{
    QFile file;
    (void)pos;

    if (file.exists(input))
        return QValidator::Acceptable;

    return QValidator::Intermediate;
}

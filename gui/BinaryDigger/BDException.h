#ifndef BDEXCEPTION_H
#define BDEXCEPTION_H

class BDException : public std::exception
{
    std::string message;
public:
    BDException(const QString &message)
    {
        this->message = message.toStdString();
    }
    ~BDException() throw() {}

    virtual const char* what() const throw() { return message.c_str(); }
};

#endif // BDEXCEPTION_H

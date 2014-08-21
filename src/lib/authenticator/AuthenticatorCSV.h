/*************************************************************************
 * Research_pacs - Copyright (C) Universite de Strasbourg
 * Distributed under the terms of the CeCILL-B license, as published by
 * the CEA-CNRS-INRIA. Refer to the LICENSE file or to
 * http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
 * for details.
 ************************************************************************/

#ifndef AUTHENTICATORCSV_H
#define AUTHENTICATORCSV_H

#include <string>
#include <map>

#include "AuthenticatorBase.h"

namespace authenticator
{

/** 
 * @brief Authenticator based on a CSV file with two columns (user and 
 *        clear-text password).
 */
class AuthenticatorCSV : public AuthenticatorBase
{
public:
    /**
     * Constructor
     * @param ifileName : CSV file path
     */
    AuthenticatorCSV(std::string const & ifileName);
    
    /**
     * Destructor
     */
    virtual ~AuthenticatorCSV();

    /**
     * Operator ()
     * @param identity: requested authentication
     * @return true if authentication success, false otherwise
     */
    virtual bool operator()(UserIdentityNegotiationSubItemRQ * identity) const;
    
    /**
     * Get number of dictionary entries
     * @return number of dictionary entries
     */
    unsigned int get_table_count() const;
    
private:
    /// User - Password dictionary
    std::map<std::string, std::string> _table;
    
};

}

#endif //AUTHENTICATORCSV_H

